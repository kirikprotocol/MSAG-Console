#include <vector>
#include <cassert>
#include <algorithm>
#include "InputStorage.h"
#include "InputBlackList.h"
#include "informer/data/CommonSettings.h"
#include "informer/data/DeliveryActivator.h"
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
#include "core/threads/Thread.hpp"
#include "system/status.h"

namespace {

const unsigned LENSIZE = 2;
const uint16_t defaultVersion = 1;

}

namespace eyeline {
namespace informer {

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
        msg.timeLeft = -1; // waiting for pvss
        msg.userData = fb.getCString();
        if (state & 0x80) {
            MessageText(fb.getCString()).swap(msg.text);
        } else {
            MessageText(0,int32_t(fb.get32())).swap(msg.text);
        }
        if (isDropped) {
            msg.lastTime = currentTime_;
            msg.timeLeft = 0;
            msg.retryCount = 0;
            // we have to add kill record in activity log
            is_.activityLog_->addRecord(currentTime_,regId_,msg,0,MSGSTATE_INPUT);
        } else {
            is_.core_.startPvssCheck(msg);
        }
        return true;
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
            glossary_.fetchText(i->msg.text);
        }
        activityLog_->addRecord(currentTime,i->serial,i->msg,0);
    }
    core_.deliveryRegions( getDlvId(), regs, true );
}


void InputStorage::dropMessages( const std::vector<msgid_type>& msgids )
{
    // FIXME: optimize (skv) sort the vector outside please
    std::vector<msgid_type> msgIds(msgids);
    std::sort(msgIds.begin(),msgIds.end());
    blackList_->addMessages(msgIds);
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
    msgid_type minRlast = lastMsgId_;
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
    } while ( true );
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
        if ( msg.text.getTextId() < MessageText::uniqueId ) {
            throw InfosmeException(EXC_IOERROR,"invalid input glossary index %d",msg.text.getTextId());
        } else if ( msg.text.getTextId() == MessageText::uniqueId ) {
            if ( ! msg.text.getText() ) {
                throw InfosmeException(EXC_IOERROR,"invalid text: both glossary index and text are absent");
            }
        } else {
            if ( msg.text.getText() ) {
                throw InfosmeException(EXC_IOERROR,"invalid text: both glossary index and text are present");
            }
        }
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
    // FIXME: optimize (write regions via big buffer)
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


void InputStorage::doTransfer( TransferRequester& req, size_t reqCount )
{
    const regionid_type regId = req.getRegionId();
    smsc_log_debug(log_,"transfer R=%u/D=%u started, reqCount=%u", regId, getDlvId(), reqCount);
    bool ok = false;

    InputRegionRecord ro;
    ro.regionId = regId;
    getRecord(ro);
    if (ro.rfn==0) { ro.rfn=1; ro.roff=0; }
    FileGuard fg;
    MessageList msglist;
    smsc::core::buffers::TmpBuf<char,8192> buf;
    while (reqCount>0) {

        try {

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
                FileReader fileReader(fg);
                IReader recordReader(*this,msglist,ro,regId);
                reqCount -= fileReader.readRecords(buf,recordReader,reqCount);
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

            if (core_.isStopping()) {
                smsc_log_debug(log_,"R=%u/D=%u transfer cancelled at stop",
                               regId, getDlvId() );
                ok = false;
                break;
            }

            // we have read things
            if ( ! msglist.empty() ) {

                if ( ! core_.isStopping() ) {
                    for ( MessageList::iterator i = msglist.begin(); i != msglist.end(); ++i ) {
                        if (!i->msg.isTextUnique()) {
                            // NOTE: replacing input ids with real ids here!
                            glossary_.fetchText(i->msg.text,true);
                        }
                    }
                }

                // wait until all pvss requests finish
                const msgtime_type currentTime(msgtime_type(currentTimeMicro()/tuPerSec));
                for ( MessageList::iterator i = msglist.begin(); i != msglist.end(); ) {
                    timediff_type ttl;
                    while ( (ttl = i->msg.timeLeft) == -1 ) {
                        // wait until pvss request is finished
                        // struct timespec ts = {0, 3000000};
                        // nanosleep(&ts,0);
                        smsc_log_debug(log_,"R=%u/D=%u/M=%llu waits until PVSS result",
                                       regId, getDlvId(), i->msg.msgId );
                        smsc::core::threads::Thread::Yield();
                    }
                    if ( ttl == 0 ) {
                        // message should be removed from list
                        uint8_t len, ton, npi;
                        const uint64_t addr = subscriberToAddress(i->msg.subscriber,len,ton,npi);
                        smsc_log_info(log_,"R=%u/D=%u/M=%llu A=.%u.%u.%0*.*llu is blocked by PVSS",
                                      regId, getDlvId(), i->msg.msgId,
                                      ton,npi,len,len,ulonglong(addr));
                        i->msg.state = MSGSTATE_FAILED;
                        // const int smppState = smsc::system::Status::ILLEGALSUBSCRIBER;
                        const int smppState = smsc::system::Status::DENIEDBYACCESSMASK;
                        activityLog_->addRecord(currentTime,regId,i->msg,
                                                smppState,
                                                MSGSTATE_INPUT);
                        i = msglist.erase(i);
                    } else {
                        ++i;
                    }
                }

                if (core_.isStopping()) {
                    smsc_log_debug(log_,"R=%u/D=%u transfer cancelled at stop",
                                   regId, getDlvId() );
                    ok = false;
                    break;
                }

                req.addNewMessages( currentTime,
                                    msglist,
                                    msglist.begin(),
                                    msglist.end() );
                ok = true;
            }
            setRecord(ro,0);

        } catch (std::exception& e) {
            smsc_log_error(log_,"R=%u/D=%u transfer exc: %s",
                           regId, getDlvId(), e.what());
            break;
        }

    } // while we need more messages

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
