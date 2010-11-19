#include <vector>
#include <cassert>
#include <algorithm>
#include "InputStorage.h"
#include "informer/data/CommonSettings.h"
#include "informer/data/DeliveryActivator.h"
#include "informer/data/FinalLog.h"
#include "informer/data/Region.h"
#include "informer/data/InputRegionRecord.h"
#include "informer/data/UserInfo.h"
#include "informer/io/FileGuard.h"
#include "informer/io/FileReader.h"
#include "informer/io/IOConverter.h"
#include "informer/io/HexDump.h"
#include "core/buffers/TmpBuf.hpp"
#include "core/buffers/IntHash64.hpp"
#include "core/synchronization/EventMonitor.hpp"

namespace {

const unsigned LENSIZE = 2;
const uint16_t defaultVersion = 1;

}

namespace eyeline {
namespace informer {

class InputStorage::BlackList
{
    static const size_t itemsize = 8;
    static const size_t packsize = 64;
    static const uint64_t packbits[packsize];

public:
    BlackList( InputStorage& is ) :
    is_(is), dropFileOffset_(0),
    minMsgId_(msgid_type(-1)), maxMsgId_(0), changing_(false) {}

    void init( msgid_type minRlast )
    {
        if (minRlast == msgid_type(-1)) return;

        smsc_log_debug(is_.log_,"D=%u minRlast=%llu, going to find this place in blacklist",
                       is_.getDlvId(),ulonglong(minRlast));

        try {
            FileGuard fg;
            bool isNew;
            const size_t maxSize = openFile(fg,minRlast,&isNew);
            dropFileOffset_ = fg.getPos();
            if (dropFileOffset_ < maxSize) {
                readChunk(fg);
            }
            fg.close();
            if (dropFileOffset_ >= maxSize) { 
                dropFileOffset_ = size_t(-1);
            }
            if (isNew) {
                writeActLog(0);
            }
        } catch ( ErrnoException& e ) {
            smsc_log_debug(is_.log_,"D=%u blacklist init exc: %s",is_.getDlvId(),e.what());
            dropFileOffset_ = size_t(-1);
        }
    }


    /// NOTE: the method should be invoked only once for given msgid
    bool isMessageDropped( msgid_type msgId ) {
        MutexGuard mg(dropMon_);
        bool res = true;
        const char* what = "";
        do {
            if (msgId < minMsgId_) {
                what = "less than min";
                res = false;
                break;
            } else if (msgId==minMsgId_) {
                ++minMsgId_;
            }

            if ((msgId > maxMsgId_ ||
                 unsigned(dropMsgHash_.Count()) < getCS()->getSkippedIdsMinCacheSize()) &&
                dropFileOffset_!=size_t(-1)) {

                if (dropMsgHash_.Count()==0) {
                    minMsgId_ = msgid_type(-1);
                    smsc_log_debug(is_.log_,"D=%u blklist has empty hash",is_.getDlvId());
                    maxMsgId_ = is_.getMinRlast();
                } else if (dropMsgHash_.Count() < 30) {

                    // we may scan the hash to find out real numbers
                    int64_t idx;
                    uint64_t* item;
                    minMsgId_ = msgid_type(-1);
                    maxMsgId_ = 0;
                    for ( DropMsgHash::Iterator it(dropMsgHash_); it.Next(idx,item); ) {
                        if (*item==0) {
                            dropMsgHash_.Delete(idx);
                            continue;
                        }
                        if (uint64_t(idx)*packsize < minMsgId_) {
                            for ( unsigned i = 0; i < packsize; ++i ) {
                                if ( *item & packbits[i] ) {
                                    minMsgId_ = uint64_t(idx)*packsize + i;
                                    break;
                                }
                            }
                        }
                        if (uint64_t(idx)*packsize > maxMsgId_) {
                            for ( unsigned i = packsize; i > 0; ) {
                                if ( *item & packbits[--i] ) {
                                    maxMsgId_ = uint64_t(idx)*packsize + i;
                                    break;
                                }
                            }
                        }
                    }
                    smsc_log_debug(is_.log_,"D=%u blklist after hash scan: min=%llu max=%llu",
                                   is_.getDlvId(), ulonglong(minMsgId_), ulonglong(maxMsgId_) );
                }

                FileGuard fg;
                const size_t maxSize = openFile(fg, dropFileOffset_ ? 0 : maxMsgId_);
                if (dropFileOffset_) {
                    smsc_log_debug(is_.log_,"D=%u seek to %llu",is_.getDlvId(),ulonglong(dropFileOffset_));
                    fg.seek(dropFileOffset_);
                }
                while ( !is_.core_.isStopping() ) {
                    readChunk(fg);
                    if (dropFileOffset_>=maxSize) {
                        dropFileOffset_ = size_t(-1);
                        break;
                    }
                    if (msgId <= maxMsgId_ || dropFileOffset_ == size_t(-1)) break;
                }
                if (msgId>maxMsgId_) {
                    what = "more than max";
                    res = false;
                    break;
                }
            }

            if (res) {
                const uint64_t idx = msgId / packsize;
                uint64_t* item = dropMsgHash_.GetPtr(idx);
                if (!item) {
                    res = false;
                    what = "not in cache";
                }
                const uint64_t bit = packbits[msgId % packsize];
                if ( *item & bit ) {
                    *item -= bit;
                    res = true;
                    what = "found in cache";
                    if (!*item) {
                        what = "found in cache, cleaned";
                        dropMsgHash_.Delete(idx);
                    }
                } else {
                    res = false;
                    what = "found, but bit not set";
                }
            }
        } while (false);
        smsc_log_debug(is_.log_,"D=%u/M=%llu is %sin blk list: %s",
                       is_.getDlvId(), msgId, res ? "" : "NOT ", what);
        return res;
    }


    /// add a list of dropped messages
    bool addMessages( std::vector<msgid_type>& dropped )
    {
        if (dropped.empty()) return false;
        struct ChangeGuard {
            ChangeGuard() : ref_(0) {}
            ~ChangeGuard() { if (ref_) {*ref_ = false;} }
            bool* ref_;
        };
        ChangeGuard cg;
        while (!is_.core_.isStopping()) {
            MutexGuard mg(dropMon_);
            if (!changing_) {
                cg.ref_ = &changing_;
                changing_ = true;
                break;
            }
            dropMon_.wait(300);
        }
        // locked
        smsc_log_debug(is_.log_,"D=%u adding %u messages to black list",
                       is_.getDlvId(),unsigned(dropped.size()));
        {
            msgid_type maxId = is_.lastMsgId_;
            std::vector<msgid_type>::iterator i =
                std::upper_bound(dropped.begin(),dropped.end(), maxId);
            if (i != dropped.end()) {
                dropped.erase(i,dropped.end());
                smsc_log_debug(is_.log_,"D=%u some messages have id>max, left=%u",
                               is_.getDlvId(), unsigned(dropped.size()));
            }
            if (dropped.empty()) return false;
        }
        msgid_type minRlast = is_.getMinRlast();
        {
            // skipping already dropped messages
            std::vector<msgid_type>::iterator i = 
                std::upper_bound(dropped.begin(), dropped.end(), minRlast);
            if (i != dropped.begin()) {
                dropped.erase(dropped.begin(),i);
                smsc_log_debug(is_.log_,"D=%u some messages already passed, left=%u",
                               is_.getDlvId(), unsigned(dropped.size()));
            }
            if (dropped.empty()) return false;
        }
        // smsc_log_debug(log_,"D=%u adding %u messages to black list",
        // is_.getDlvId(),unsigned(dropped.size()));

        FileGuard ofg;
        size_t maxSize;
        try {
            maxSize = openFile(ofg,minRlast);
            if (ofg.getPos() >= maxSize) {
                ofg.close();
            }
        } catch (ErrnoException& e) {
            smsc_log_debug(is_.log_,"D=%u blklist read exc: %s",is_.getDlvId(),e.what());
            maxSize = 0;
            ofg.close();
        }
        FileGuard nfg;
        char fname[100];
        sprintf(makeDeliveryPath(fname,is_.getDlvId()),"new/black.lst");
        nfg.create((getCS()->getStorePath() + fname + ".tmp").c_str(),0666,true,true);

        smsc::core::buffers::TmpBuf<char,2048> buf;
        smsc::core::buffers::TmpBuf<char,2048> tobuf;
        FromBuf fb(buf.get(),0);
        ToBuf tb(tobuf.get(),tobuf.getSize());
        std::vector<msgid_type>::iterator it = dropped.begin();
        while (true) {
            if (is_.core_.isStopping()) { return false; }
            if (fb.getPos() >= fb.getLen() && ofg.isOpened()) {
                const size_t wasread = readBuf(ofg,buf);
                if (ofg.getPos() >= maxSize) {
                    ofg.close();
                }
                fb.setBuf(buf.get(),wasread);
            }
            msgid_type nextItem;
            if (fb.getPos() < fb.getLen()) {
                nextItem = fb.get64();
            } else {
                nextItem = size_t(-1);
            }

            // dumping elements from vector until nextItem found
            std::vector<msgid_type>::iterator jt =
                std::lower_bound(it,dropped.end(),nextItem);
            for ( ; it != jt; ++it ) {
                tb.set64(*it);
                if (tb.getPos() == tb.getLen()) {
                    nfg.write(tobuf.get(),tb.getPos());
                    tb.setPos(0);
                }
            }
            it = jt;
            if (nextItem != size_t(-1)) {
                tb.set64(nextItem);
                if (tb.getPos() == tb.getLen()) {
                    nfg.write(tobuf.get(),tb.getPos());
                    tb.setPos(0);
                }
            }
            if (it == dropped.end()) break;
            else if (*it == nextItem) { ++it; }
        }
        // writing the rest of the buffer
        if (tb.getPos()) {
            nfg.write(tobuf.get(),tb.getPos());
        }
        // writing the tail of the file
        if (ofg.isOpened()) {
            while (true) {
                if (!is_.core_.isStopping()) return false;
                const size_t wasread = readBuf(ofg,buf);
                if (!wasread) break;
                nfg.write(buf.get(),wasread);
            }
        }
        ofg.close();
        nfg.close();

        {
            smsc_log_debug(is_.log_,"D=%u new black list file is written, renaming",is_.getDlvId());
            MutexGuard mg(dropMon_);
            if ( -1 == rename( (getCS()->getStorePath() + fname + ".tmp").c_str(),
                               (getCS()->getStorePath() + fname + ".new").c_str() ) ) {
                throw ErrnoException(errno,"rename(%s)",fname);
            }
            dropFileOffset_ = 0;
            dropMon_.notify();
        }
        writeActLog(100);
        return true;
    }

private:
    /// a hash for dropped message, which are stored combined by 64.
    typedef smsc::core::buffers::IntHash64<uint64_t> DropMsgHash;

    size_t openFile( FileGuard& fg, msgid_type minRlast, bool* isNew = 0 )
    {
        char fname[100];
        sprintf(makeDeliveryPath(fname,is_.getDlvId()),"new/black.lst");
        struct stat st;
        if ( ::stat((getCS()->getStorePath()+fname + ".new").c_str(),&st)==0 ) {
            fg.ropen((getCS()->getStorePath()+fname + ".new").c_str());
            smsc_log_debug(is_.log_,"D=%u new blklist file '%s.new' opened",
                           is_.getDlvId(),fname);
            if (isNew) *isNew = true;
        } else {
            fg.ropen((getCS()->getStorePath() + fname).c_str());
            smsc_log_debug(is_.log_,"D=%u blklist file '%s' opened",
                           is_.getDlvId(),fname);
            if (isNew) *isNew = false;
        }
        const off_t maxSize = fg.getStat(st).st_size;
        if (maxSize % itemsize != 0) {
            throw InfosmeException(EXC_BADFILE,"D=%u bad file '%s', size(%llu) %% %u != 0",
                                   is_.getDlvId(), fname, ulonglong(maxSize),
                                   unsigned(itemsize));
        }
        if (minRlast == 0) return size_t(maxSize);

        // seeking the position
        off_t left = 0;
        off_t right = maxSize;
        char buf[itemsize];
        FromBuf fb(buf,sizeof(buf));
        while ( left < right ) {
            off_t middle = (right + left) /itemsize /2 * itemsize;
            fg.seek(middle);
            fg.read(buf,sizeof(buf));
            fb.setPos(0);
            const msgid_type midVal = fb.get64();
            smsc_log_debug(is_.log_,"D=%u blklist pos=%llu val=%llu",
                           is_.getDlvId(),ulonglong(middle),ulonglong(midVal));
            if (midVal > minRlast) {
                right = middle;
            } else if (midVal == minRlast) {
                left = middle+itemsize;
                break;
            } else {
                left = middle;
            }
        }
        if (left>maxSize) {
            throw InfosmeException(EXC_LOGICERROR,"blacklist algo failed, left=%llu, maxSize=%llu",
                                   ulonglong(left),ulonglong(maxSize));
        }
        fg.seek(left);
        smsc_log_debug(is_.log_,"D=%u position for M=%llu found: %u",
                       is_.getDlvId(), ulonglong(minRlast),unsigned(left));
        return size_t(maxSize);
    }


    size_t readBuf( FileGuard& fg, smsc::core::buffers::TmpBuf<char,2048>& buf )
    {
        const size_t requested = itemsize * getCS()->getSkippedIdsChunkSize();
        buf.setSize( requested );
        const size_t wasread = fg.read( buf.get(), requested );
        if ( wasread % itemsize != 0 ) {
            throw InfosmeException(EXC_BADFILE, "blacklist wasread=%llu, not divided by %u",
                                   ulonglong(wasread), unsigned(itemsize));
        }
        return wasread;
    }


    /// read the next chunk from blacklist file
    void readChunk( FileGuard& fg )
    {
        smsc::core::buffers::TmpBuf<char,2048> buf;
        const size_t wasread = readBuf(fg,buf);
        smsc_log_debug(is_.log_,"D=%u reading chunk at pos %llu, size=%u",
                       is_.getDlvId(), ulonglong(fg.getPos()-wasread),unsigned(wasread));
        dropFileOffset_ += wasread;
        if (wasread==0) {
            dropFileOffset_ = size_t(-1);
            return;
        }
        FromBuf fb(buf.get(),wasread);
        uint64_t prevIdx = 0;
        uint64_t* item = 0;
        for ( size_t i = 0; i < wasread; i += itemsize ) {
            msgid_type msgId = fb.get64();
            uint64_t idx = msgId / packsize;
            if (!item || idx != prevIdx) {
                item = dropMsgHash_.GetPtr(idx);
                if (!item) item = &dropMsgHash_.Insert(idx,0ULL);
                prevIdx = idx;
            }
            *item |= packbits[msgId % packsize];
            if (msgId < minMsgId_ ) {
                minMsgId_ = msgId;
            }
            if (msgId <= maxMsgId_) {
                throw InfosmeException(EXC_LOGICERROR,"D=%u blacklist not sorted",is_.getDlvId());
            }
            maxMsgId_ = msgId;
        }
        smsc_log_debug(is_.log_,"D=%u after chunk read: min=%llu max=%llu offset=%u",
                       is_.getDlvId(), ulonglong(minMsgId_),
                       ulonglong(maxMsgId_), ulonglong(dropFileOffset_));
    }


    void writeActLog( unsigned sleepTime )
    {
        FileGuard fg;
        char fname[100];
        sprintf(makeDeliveryPath(fname,is_.getDlvId()),"new/black.lst");
        fg.ropen((getCS()->getStorePath()+fname + ".new").c_str());
        smsc_log_debug(is_.log_,"D=%u writing delete records to actlog by reading '%s'",
                       is_.getDlvId(), fname);
        smsc::core::buffers::TmpBuf<char,2048> buf;
        std::vector<msgid_type> msgIds;
        while (true) {
            if (is_.core_.isStopping()) break;
            const size_t wasread = fg.read(buf.get(),buf.getSize());
            if (wasread == 0) {
                // eof
                MutexGuard mg(dropMon_);
                if ( -1 == rename((getCS()->getStorePath() + fname + ".new").c_str(),
                                  (getCS()->getStorePath() + fname).c_str())) {
                    throw ErrnoException(errno,"rename(%s)",fname);
                }
                break;
            }
            FromBuf fb(buf.get(),wasread);
            for ( unsigned i = 0; i < wasread; i += itemsize ) {
                msgIds.push_back(fb.get64());
            }
            is_.activityLog_->addDeleteRecords(currentTimeSeconds(),msgIds);
            msgIds.clear();
            if (sleepTime>9) {
                MutexGuard mg(dropMon_);
                dropMon_.wait(sleepTime);
            }
        }
    }

private:
    InputStorage& is_;
    smsc::core::synchronization::EventMonitor dropMon_;
    DropMsgHash                               dropMsgHash_;
    size_t                                    dropFileOffset_;
    msgid_type                                minMsgId_;
    msgid_type                                maxMsgId_;
    bool                                      changing_;
};


const uint64_t InputStorage::BlackList::packbits[packsize] =
{
    0x0000000000000001ULL,
    0x0000000000000002ULL,
    0x0000000000000004ULL,
    0x0000000000000008ULL,
    0x0000000000000010ULL,
    0x0000000000000020ULL,
    0x0000000000000040ULL,
    0x0000000000000080ULL,
    0x0000000000000100ULL,
    0x0000000000000200ULL,
    0x0000000000000400ULL,
    0x0000000000000800ULL,
    0x0000000000001000ULL,
    0x0000000000002000ULL,
    0x0000000000004000ULL,
    0x0000000000008000ULL,
    0x0000000000010000ULL,
    0x0000000000020000ULL,
    0x0000000000040000ULL,
    0x0000000000080000ULL,
    0x0000000000100000ULL,
    0x0000000000200000ULL,
    0x0000000000400000ULL,
    0x0000000000800000ULL,
    0x0000000001000000ULL,
    0x0000000002000000ULL,
    0x0000000004000000ULL,
    0x0000000008000000ULL,
    0x0000000010000000ULL,
    0x0000000020000000ULL,
    0x0000000040000000ULL,
    0x0000000080000000ULL,
    0x0000000100000000ULL,
    0x0000000200000000ULL,
    0x0000000400000000ULL,
    0x0000000800000000ULL,
    0x0000001000000000ULL,
    0x0000002000000000ULL,
    0x0000004000000000ULL,
    0x0000008000000000ULL,
    0x0000010000000000ULL,
    0x0000020000000000ULL,
    0x0000040000000000ULL,
    0x0000080000000000ULL,
    0x0000100000000000ULL,
    0x0000200000000000ULL,
    0x0000400000000000ULL,
    0x0000800000000000ULL,
    0x0001000000000000ULL,
    0x0002000000000000ULL,
    0x0004000000000000ULL,
    0x0008000000000000ULL,
    0x0010000000000000ULL,
    0x0020000000000000ULL,
    0x0040000000000000ULL,
    0x0080000000000000ULL,
    0x0100000000000000ULL,
    0x0200000000000000ULL,
    0x0400000000000000ULL,
    0x0800000000000000ULL,
    0x1000000000000000ULL,
    0x2000000000000000ULL,
    0x4000000000000000ULL,
    0x8000000000000000ULL
};


class InputStorage::IReader : public FileReader::RecordReader
{
public:
    IReader( InputStorage& is,
             MessageList& ml,
             InputRegionRecord& iro,
             regionid_type regId ) :
    is_(is), msglist_(ml), ro_(iro),
    regId_(regId),
    currentTime_(currentTimeSeconds()) {}

    virtual bool isStopping() {
        return is_.core_.isStopping();
    }

    /// return the size of record length in octets.
    virtual size_t recordLengthSize() const { return LENSIZE; }

    /// read record length from fb and checks its validity.
    virtual size_t readRecordLength( size_t filePos, FromBuf& fb ) {
        const size_t rl(fb.get16());
        if (rl>10000) {
            throw InfosmeException(EXC_BADFILE,"reclen=%u is too big",unsigned(rl));
        }
        return rl;
    }

    /// read the record data (w/o length)
    virtual bool readRecordData( size_t filePos, FromBuf& fb ) {
        ro_.roff += fb.getLen();
        const uint16_t version = fb.get16();
        if (version!=1) {
            throw InfosmeException(EXC_BADFILE,"unsupported version %u at %llu",
                                   version,ulonglong(filePos));
        }
        const uint64_t msgId = fb.get64();
        const uint8_t  state = fb.get8();
        if (msgId>ro_.rlast) ro_.rlast = msgId;
        const bool isDropped = is_.blackList_->isMessageDropped(msgId);
        MessageLocker* mlk;
        MessageLocker mlk2;
        if ( isDropped ) {
            mlk = &mlk2;
            mlk->msg.state = MSGSTATE_KILLED;
        } else {
            msglist_.push_back(MessageLocker());
            mlk = &msglist_.back();
            mlk->msg.state = state & 0x7f;
        }
        mlk->serial = 0;
        Message& msg = mlk->msg;
        msg.msgId = msgid_type(msgId);
        msg.subscriber = fb.get64();
        // msg.lastTime = fb.get32();
        // msg.timeLeft = fb.get32();
        msg.userData = fb.getCString();
        if (state & 0x80) {
            MessageText(fb.getCString(),0).swap(msg.text);
        } else {
            MessageText(0,fb.get32()).swap(msg.text);
        }
        if (isDropped) {
            // we have to add kill record in activity log
            is_.activityLog_->addRecord(currentTime_,regId_,msg,0);
            is_.getDlvActivator().getFinalLog().addMsgRecord(currentTime_,
                                                             is_.getDlvId(),
                                                             is_.activityLog_->getUserInfo().getUserId(),
                                                             msg,
                                                             0 );
        }
        return !isDropped;
    }
private:
    InputStorage&      is_;
    MessageList&       msglist_;
    InputRegionRecord& ro_;
    regionid_type      regId_;
    msgtime_type       currentTime_;
};


// =============================================================================


InputStorage::InputStorage( DeliveryActivator& core,
                            InputJournal&      jnl ) :
log_(smsc::logger::Logger::getInstance("instore")),
core_(core),
rollingIter_(recordList_.end()),
jnl_(jnl),
lastfn_(0),
activityLog_(0),
lastMsgId_(0),
blackList_(0)
{
}


InputStorage::~InputStorage()
{
    if (blackList_) delete blackList_;
}


void InputStorage::init( ActivityLog& actLog )
{
    activityLog_ = &actLog;
    try {
        glossary_.init(getDlvId());
    } catch ( std::exception& e ) {
        smsc_log_error(log_,"D=%u glossary init exc: %s",getDlvId(),e.what());
        throw;
    }
    if (!blackList_) {
        blackList_ = new BlackList(*this);
    }
}


void InputStorage::addNewMessages( MsgIter begin, MsgIter end )
{
    std::vector< regionid_type > regs;
    regs.reserve(32);
    dispatchMessages(begin, end, regs);
    msgtime_type currentTime(currentTimeSeconds());
    // binding to glossary (necessary to write texts to activity log)
    for ( MsgIter i = begin; i != end; ++i ) {
        if (!i->msg.isTextUnique()) {
            // necessary to replace text ids with real texts
            glossary_.fetchText(i->msg.text,true);
        }
        activityLog_->addRecord(currentTime, i->serial, i->msg, 0);
    }
    core_.deliveryRegions( getDlvId(), regs, true );
}


void InputStorage::dropMessages( const std::vector<msgid_type>& msgids )
{
    // FIXME: optimize sort the vector outside please
    std::vector<msgid_type> msgIds(msgids);
    std::sort(msgIds.begin(),msgIds.end());
    blackList_->addMessages(msgIds);
    // activityLog_->addDeleteRecords( currentTimeSeconds(), msgIds );
}


void InputStorage::setRecordAtInit( const InputRegionRecord& ro,
                                    uint64_t maxMsgId )
{
    RecordList::iterator* ptr = recordHash_.GetPtr(ro.regionId);
    if (ptr) {
        doSetRecord(*ptr,ro);
    } else {
        ptr = &recordHash_.Insert(ro.regionId,
                                  recordList_.insert(recordList_.end(),ro));
        // NOTE: we don't care about rollingIter here
    }
    if (maxMsgId>lastMsgId_) lastMsgId_ = maxMsgId;
    InputRegionRecord& rec = **ptr;
    if (rec.wfn > lastfn_) lastfn_ = rec.wfn;
    smsc_log_debug(log_,"R=%u/D=%u set input record: RP=%u/%u, WP=%u/%u, count=%u, msgs=%llu, lastfn=%u",
                   unsigned(rec.regionId),
                   unsigned(getDlvId()),
                   unsigned(rec.rfn), unsigned(rec.roff),
                   unsigned(rec.wfn), unsigned(rec.woff),
                   unsigned(rec.count), ulonglong(lastMsgId_),
                   unsigned(lastfn_) );
}


void InputStorage::postInit( std::vector<regionid_type>& regs )
{
    int regId;
    RecordList::iterator iter;
    msgid_type minRlast = msgid_type(-1LL);
    for ( RecordHash::Iterator i(recordHash_); i.Next(regId,iter); ) {
        if ( iter->rlast < minRlast ) {
            minRlast = iter->rlast;
        }
        if ( iter->rfn == iter->wfn && iter->roff == iter->woff ) {
            // no more messages in region
            continue; 
        }
        regs.push_back(regionid_type(regId));
    }
    blackList_->init(minRlast);
}


size_t InputStorage::rollOver()
{
    smsc_log_debug(log_,"rollover input storage for D=%u",getDlvId());
    bool firstPass = true;
    size_t written = 0;
    do {
        if ( core_.isStopping() ) break;
        InputRegionRecord ro;
        msgid_type maxMsgId;
        {
            smsc::core::synchronization::MutexGuard mg(lock_);
            if (firstPass) {
                rollingIter_ = recordList_.begin();
                firstPass = false;
            }
            if ( rollingIter_ == recordList_.end() ) { break; }
            ro = *rollingIter_;
            ++rollingIter_;
            maxMsgId = lastMsgId_;
        }
        written += jnl_.journalRecord(getDlvId(),ro,maxMsgId);
    } while ( false );
    smsc_log_debug(log_,"roll over finished D=%u, written=%u",getDlvId(),unsigned(written));
    return written;
}


void InputStorage::dispatchMessages( MsgIter begin,
                                     MsgIter end,
                                     std::vector< regionid_type >& regs )
{
    static const unsigned fileSize = 100000;
    RegionFinder& rf = core_.getRegionFinder();
    MutexGuard mg(wlock_);
    // preprocess
    unsigned total = 0;
    msgid_type minRlast = lastMsgId_;
    for ( MsgIter i = begin; i != end; ++i ) {
        const regionid_type regId = rf.findRegion( i->msg.subscriber );
        Message& msg = i->msg;
        msg.msgId = ++lastMsgId_;
        msg.lastTime = 0;
        msg.timeLeft = 0;
        msg.retryCount = 0;
        msg.state = MSGSTATE_INPUT;
        i->serial = regId;
        if (log_->isDebugEnabled()) {
            uint8_t len, ton, npi;
            uint64_t addr = subscriberToAddress(i->msg.subscriber,len,ton,npi);
            smsc_log_debug(log_,"adding D=%u/M=%llu for .%u.%u.%0*.*llu -> R=%u",
                           unsigned(getDlvId()), ulonglong(i->msg.msgId),
                           ton,npi,len,len,ulonglong(addr),unsigned(regId));
        }
        if ( std::find(regs.begin(),regs.end(),regId) == regs.end() ) {
            regs.push_back(regId);
        }
        ++total;
    }
    smsc_log_debug(log_,"D=%u add %u new messages, regions: [%s]",
                   unsigned(getDlvId()),total,
                   formatRegionList(regs.begin(),regs.end()).c_str());
    // writing regions
    // fixme: optimize (write via big buffer)
    smsc::core::buffers::TmpBuf<unsigned char,200> msgbuf;
    for ( std::vector<regionid_type>::const_iterator ir = regs.begin();
          ir != regs.end(); ++ir ) {
        const regionid_type regId = *ir;
        smsc_log_debug(log_,"processing R=%u",unsigned(regId));
        InputRegionRecord ro;
        ro.regionId = regId;
        getRecord(ro,minRlast);
        FileGuard fg;
        fg.create(makeFilePath(regId,ro.wfn).c_str(),0666);
        fg.seek(ro.woff);
        msgid_type maxMsgId = 0;
        for ( MsgIter i = begin; i != end; ++i ) {
            if (i->serial != regId) continue;
            if (ro.count % fileSize == 0) {
                // need to create new file.
                ro.wfn = ++lastfn_;
                ro.woff = 0;
                std::string fname = makeFilePath(regId,ro.wfn);
                smsc_log_debug(log_,"R=%u/D=%u new file: '%s'",
                               unsigned(regId),
                               unsigned(getDlvId()),
                               fname.c_str() );
                fg.create(fname.c_str(),0666,true,true);
                // fg.seek(ro.woff);
            }
            // writing to a file
            Message& msg = i->msg;
            if (msg.isTextUnique()) {
                msgbuf.setSize(90+strlen(msg.text.getText()));
            }
            // msg.lastTime = ro.wfn;
            // msg.timeLeft = ro.woff;
            maxMsgId = msg.msgId;
            ToBuf tb(msgbuf.get(),msgbuf.getSize());
            tb.skip(LENSIZE);
            tb.set16(::defaultVersion);
            uint8_t state = msg.state & 0x7f;
            if (msg.isTextUnique()) { state |= 0x80; }
            tb.set64(msg.msgId);
            tb.set8(state);
            tb.set64(msg.subscriber);
            // tb.set32(msg.lastTime);
            // tb.set32(msg.timeLeft);
            tb.setCString(msg.userData.c_str());
            if (state >= 0x80) {
                tb.setCString(msg.text.getText());
            } else {
                tb.set32(msg.text.getTextId());
            }

            const size_t buflen = tb.getPos();
            tb.setPos(0);
            tb.set16(uint16_t(buflen-LENSIZE));
            if (log_->isDebugEnabled()) {
                HexDump hd;
                HexDump::string_type dump;
                dump.reserve(hd.hexdumpsize(buflen)+buflen+10);
                hd.hexdump(dump,msgbuf.get(),buflen);
                hd.strdump(dump,msgbuf.get(),buflen);
                smsc_log_debug(log_,"record(%u): %s",buflen,hd.c_str(dump));
            }
            fg.write(msgbuf.get(),buflen);
            ro.woff += unsigned(buflen);
            ++ro.count;
        }
        fg.close();
        // setting data back
        setRecord(ro,maxMsgId);
    }
}


void InputStorage::doTransfer( TransferRequester& req, unsigned reqCount )
{
    const regionid_type regId = req.getRegionId();
    smsc_log_debug(log_,"transfer R=%u/D=%u started, reqCount=%u", regId, getDlvId(), reqCount);
    bool ok = false;
    try {
        InputRegionRecord ro;
        ro.regionId = regId;
        getRecord(ro);
        if (ro.rfn==0) { ro.rfn=1; ro.roff=0; }
        FileGuard fg;
        MessageList msglist;
        smsc::core::buffers::TmpBuf<char,8192> buf;
        while (reqCount>0) {

            if (!fg.isOpened()) {
                // need new file
                if ( ro.rfn<ro.wfn || (ro.rfn==ro.wfn && ro.roff<ro.woff)) {
                    // ok
                } else {
                    smsc_log_debug(log_,"R=%u/D=%u no more files to read", regId, getDlvId());
                    break;
                }
                const std::string fname = makeFilePath(regId,ro.rfn);
                // check if the file exists
                struct stat st;
                if ( -1 == stat( fname.c_str(), &st ) ) {
                    if (ro.roff != 0) {
                        throw InfosmeException(EXC_LOGICERROR,"R=%u/D=%u file '%s' has roff=%u, but is inaccessible",
                                               regId, getDlvId(), fname.c_str(), ro.roff);
                    }
                    if (errno == ENOENT) {
                        smsc_log_debug(log_,"R=%u/D=%u file '%s' does not exist, move on",
                                       regId, getDlvId(), fname.c_str());
                        ++ro.rfn;
                        ro.roff = 0;
                        continue;
                    }
                    throw ErrnoException(errno,"stat('%s')",fname.c_str());
                } else if ( !S_ISREG(st.st_mode) ) {
                    throw InfosmeException(EXC_LOGICERROR,"'%s' is not a file",fname.c_str());
                }
                smsc_log_debug(log_,"R=%u/D=%u trying to open new file '%s'",
                               regId, getDlvId(), fname.c_str());
                fg.ropen(fname.c_str());
                fg.seek(ro.roff);
            }

            try {
                // FIXME: limit the number of reads in deleted storage!
                FileReader fileReader(fg);
                IReader recordReader(*this,msglist,ro,regId);
                fileReader.readRecords(buf,recordReader,reqCount);
                fg.close();
                if (ro.rfn<ro.wfn) {
                    ++ro.rfn;
                    ro.roff=0;
                }
            } catch ( FileDataException& e ) {
                if (ro.rfn < ro.wfn) {
                    smsc_log_error(log_,"R=%u/D=%u RP=%u/%u intermediate file exc: %s",
                                   regId, getDlvId(), ro.rfn, ro.roff, e.what());
                    throw;
                }
                smsc_log_debug(log_,"R=%u/D=%u RP=%u/%u last record garbled, wait until write finish",
                               regId, getDlvId(), ro.rfn, ro.roff );
                break;
            } catch ( std::exception& e ) {
                smsc_log_error(log_,"R=%u/D=%u RP=%u/%u exc: %s",
                               regId, getDlvId(), ro.rfn, ro.roff, e.what());
            }
            buf.SetPos(0);

        } // while we need more messages

        // we have read things
        if ( ! msglist.empty() ) {
            // write back record
            for ( MessageList::iterator i = msglist.begin(); i != msglist.end(); ++i ) {
                if (!i->msg.isTextUnique()) {
                    // NOTE: replacing input ids with real ids here!
                    glossary_.fetchText(i->msg.text,true,true);
                }
            }

            const msgtime_type currentTime(msgtime_type(currentTimeMicro()/tuPerSec));
            req.addNewMessages( currentTime,
                                msglist,
                                msglist.begin(),
                                msglist.end() );
            setRecord(ro,0);
            ok = true;
        }

    } catch (std::exception& e) {
        smsc_log_error(log_,"R=%u/D=%u transfer exc: %s",
                       regId, getDlvId(), e.what());
    }
    smsc_log_debug(log_,"R=%u/D=%u transfer task finished, %s",
                   regId, getDlvId(), ok ?
                   "notifying core" : "no msgs passed" );
    if ( ok ) {
        std::vector< regionid_type > regs;
        regs.push_back(regId);
        core_.deliveryRegions( getDlvId(), regs, true );
    }
}


void InputStorage::getRecord( InputRegionRecord& ro, msgid_type minRlast )
{
    {
        smsc::core::synchronization::MutexGuard mg(lock_);
        RecordList::iterator* ptr = recordHash_.GetPtr(ro.regionId);
        if (!ptr) {
            ro.clear();
            ro.rlast = minRlast;
            recordHash_.Insert(ro.regionId,recordList_.insert(recordList_.begin(),ro));
        } else {
            ro = **ptr;
        }
    }
    smsc_log_debug(log_,"got record for R=%u/D=%u: RP=%u/%u, WP=%u/%u, count=%u, rlast=%llu",
                   unsigned(ro.regionId),
                   unsigned(getDlvId()),
                   unsigned(ro.rfn), unsigned(ro.roff),
                   unsigned(ro.wfn), unsigned(ro.woff),
                   unsigned(ro.count), ulonglong(ro.rlast) );
}


void InputStorage::setRecord( InputRegionRecord& ro, uint64_t maxMsgId )
{
    {
        smsc::core::synchronization::MutexGuard mg(lock_);
        RecordList::iterator* ptr = recordHash_.GetPtr(ro.regionId);
        assert(ptr);
        doSetRecord(*ptr,ro);
        ro = **ptr;
    }
    smsc_log_debug(log_,"set record for R=%u/D=%u: RP=%u/%u, WP=%u/%u, count=%u, rlast=%llu",
                   unsigned(ro.regionId),
                   unsigned(getDlvId()),
                   unsigned(ro.rfn), unsigned(ro.roff),
                   unsigned(ro.wfn), unsigned(ro.woff),
                   unsigned(ro.count), ulonglong(ro.rlast) );
    jnl_.journalRecord(getDlvId(),ro,maxMsgId);
}


void InputStorage::doSetRecord( RecordList::iterator to, const InputRegionRecord& ro )
{
    if (ro.rfn>to->rfn) {
        to->rfn = ro.rfn;
        to->roff = ro.roff;
    } else if (ro.rfn==to->rfn && ro.roff>to->roff) {
        to->roff = ro.roff;
    }

    if (ro.wfn>to->wfn) {
        to->wfn = ro.wfn;
        to->woff = ro.woff;
    } else if (ro.wfn==to->wfn && ro.woff>to->woff) {
        to->woff = ro.woff;
    }

    if (ro.count>to->count) {
        to->count = ro.count;
    }
    if (ro.rlast>to->rlast) {
        to->rlast = ro.rlast;
    }
}


std::string InputStorage::makeFilePath( regionid_type regId, uint32_t fn ) const
{
    if (fn==0) return "";
    char buf[70];
    sprintf(makeDeliveryPath(buf,getDlvId()),"new/%u/%u.data",unsigned(regId),unsigned(fn));
    smsc_log_debug(log_,"filepath for R=%u/D=%u/F=%u is %s",
                   unsigned(regId),
                   unsigned(getDlvId()),
                   unsigned(fn),buf);
    return getCS()->getStorePath() + buf;
}


msgid_type InputStorage::getMinRlast()
{
    MutexGuard mg(lock_);
    int regId;
    RecordList::iterator iter;
    size_t minRlast = size_t(-1);
    for ( RecordHash::Iterator i(recordHash_); i.Next(regId,iter); ) {
        if (iter->rlast < minRlast) {
            minRlast = iter->rlast;
        }
    }
    return minRlast;
}


}
}
