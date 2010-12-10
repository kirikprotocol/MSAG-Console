#include "InputBlackList.h"
#include "informer/io/IOConverter.h"
#include "informer/data/CommonSettings.h"
#include "informer/data/DeliveryActivator.h"

namespace eyeline {
namespace informer {

const size_t InputStorage::BlackList::itemsize;
const size_t InputStorage::BlackList::packsize;
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



InputStorage::BlackList::BlackList( InputStorage& is ) :
is_(is),
dropFileOffset_(0),
minMsgId_(1),
maxMsgId_(0),
changing_(false)
{
}


void InputStorage::BlackList::init( msgid_type minRlast )
{
    smsc_log_debug(is_.log_,"D=%u blklist init(minRlast=%llu)",
                   is_.getDlvId(),ulonglong(minRlast));

    maxMsgId_ = minRlast;
    minMsgId_ = maxMsgId_ + 1;

    try {
        FileGuard fg;
        bool isNew;
        const size_t maxSize = openFile(fg,minRlast,&isNew);
        dropFileOffset_ = fg.getPos();
        if (dropFileOffset_ < maxSize) {
            Buf buf;
            readChunk(fg,buf);
            if (dropFileOffset_>=maxSize) {
                dropFileOffset_ = size_t(-1);
            }
        }
        fg.close();
        if (isNew) {
            writeActLog(0);
        }
    } catch ( ErrnoException& e ) {
        smsc_log_debug(is_.log_,"D=%u blklist init exc: %s",is_.getDlvId(),e.what());
        dropFileOffset_ = size_t(-1);
    }
}


/// NOTE: the method should be invoked only once for given msgid
bool InputStorage::BlackList::isMessageDropped( msgid_type msgId )
{
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

        if (dropFileOffset_ == size_t(-1)) {
            smsc_log_debug(is_.log_,"D=%u blklist not uploaded as offset=-1",is_.getDlvId());
        } else if ( msgId > maxMsgId_ ||
                    unsigned(dropMsgHash_.Count()) < getCS()->getSkippedIdsMinCacheSize() ) {
            smsc_log_debug(is_.log_,"D=%u need to upload blklist",is_.getDlvId());
            
            if (dropMsgHash_.Count()==0) {
                maxMsgId_ = is_.getMinRlast();
                minMsgId_ = maxMsgId_ + 1;
                smsc_log_debug(is_.log_,"D=%u blklist has empty hash, using min=%llu max=%llu",
                               is_.getDlvId(), ulonglong(minMsgId_), ulonglong(maxMsgId_));
            } else if (dropMsgHash_.Count() < 20) {

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
                smsc_log_debug(is_.log_,"D=%u blklist after hash scan: min=%llu max=%llu count=%u",
                               is_.getDlvId(), ulonglong(minMsgId_), ulonglong(maxMsgId_),
                               unsigned(dropMsgHash_.Count()) );
            } else {
                smsc_log_debug(is_.log_,"D=%u using current blklist hash range: min=%llu max=%llu count=%u",
                               is_.getDlvId(), ulonglong(minMsgId_), ulonglong(maxMsgId_),
                               unsigned(dropMsgHash_.Count()));
            }
            
            try {
                FileGuard fg;
                const size_t maxSize = openFile(fg, dropFileOffset_ ? 0 : maxMsgId_);
                if (dropFileOffset_) {
                    smsc_log_debug(is_.log_,"D=%u blklist seek to %llu",
                                   is_.getDlvId(),ulonglong(dropFileOffset_));
                    fg.seek(dropFileOffset_);
                }
                Buf buf;
                while ( ! getCS()->isStopping() ) {
                    if (!readChunk(fg,buf)) { break; }
                    if (dropFileOffset_ >= maxSize) {
                        dropFileOffset_ = size_t(-1);
                        break;
                    }
                    if (msgId <= maxMsgId_) {
                        smsc_log_debug(is_.log_,"D=%u we have loaded requested id=%llu min=%llu max=%llu",
                                       is_.getDlvId(), ulonglong(msgId),
                                       ulonglong(minMsgId_), ulonglong(maxMsgId_) );
                        break;
                    }
                }
            } catch ( ErrnoException& e ) {
                smsc_log_debug(is_.log_,"D=%u blklist exc: %s",is_.getDlvId(),e.what());
                dropFileOffset_ = size_t(-1);
            }
            if (msgId>maxMsgId_) {
                what = "more than max";
                res = false;
                break;
            }
        }

        if (res) {
            // trying to get bits
            const uint64_t idx = msgId / packsize;
            uint64_t* item = dropMsgHash_.GetPtr(idx);
            if (!item) {
                res = false;
                what = "not in cache";
                break;
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
                break;
            }

            res = false;
            what = "found, but bit not set";
        }

    } while (false);
    smsc_log_debug(is_.log_,"D=%u/M=%llu is %sin blklist: %s",
                   is_.getDlvId(), msgId, res ? "" : "NOT ", what);
    return res;
}


/// add a list of dropped messages
bool InputStorage::BlackList::addMessages( std::vector<msgid_type>& dropped )
{
    if (dropped.empty()) return false;
    struct ChangeGuard {
        ChangeGuard() : ref_(0) {}
        ~ChangeGuard() { if (ref_) {*ref_ = false;} }
        bool* ref_;
    };
    ChangeGuard cg;
    while (!getCS()->isStopping()) {
        MutexGuard mg(dropMon_);
        if (!changing_) {
            cg.ref_ = &changing_;
            changing_ = true;
            break;
        }
        dropMon_.wait(300);
    }
    if (getCS()->isStopping()) {
        return false;
    }
    // locked
    smsc_log_debug(is_.log_,"D=%u adding %u messages to blklist min=%llu max=%llu count=%u",
                   is_.getDlvId(),unsigned(dropped.size()),
                   ulonglong(minMsgId_), ulonglong(maxMsgId_),
                   unsigned(dropMsgHash_.Count()));
    {
        msgid_type maxId = is_.lastMsgId_;
        std::vector<msgid_type>::iterator i =
            std::upper_bound(dropped.begin(),dropped.end(), maxId);
        if (i != dropped.end()) {
            dropped.erase(i,dropped.end());
            smsc_log_debug(is_.log_,"D=%u some blklist messages have id>max, left=%u",
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
            smsc_log_debug(is_.log_,"D=%u some blklist messages already passed, left=%u",
                           is_.getDlvId(), unsigned(dropped.size()));
        }
        if (dropped.empty()) return false;
    }
    // smsc_log_debug(log_,"D=%u adding %u messages to blklist",
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
        ofg.close();
    }
    FileGuard nfg;
    char fname[100];
    sprintf(makeDeliveryPath(fname,is_.getDlvId()),"new/black.lst");
    nfg.create((getCS()->getStorePath() + fname + ".tmp").c_str(),0666,true,true);

    Buf buf;
    Buf tobuf;
    FromBuf fb(buf.get(),0);
    ToBuf tb(tobuf.get(),tobuf.getSize());
    std::vector<msgid_type>::iterator it = dropped.begin();
    while (true) {
        if ( getCS()->isStopping()) { return false; }
        if (ofg.isOpened() && fb.getPos() >= fb.getLen()) {
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
            if (! getCS()->isStopping()) return false;
            const size_t wasread = readBuf(ofg,buf);
            if (!wasread) break;
            nfg.write(buf.get(),wasread);
        }
    }
    ofg.close();
    nfg.close();

    {
        smsc_log_debug(is_.log_,"D=%u new blklist file is written, renaming",is_.getDlvId());
        MutexGuard mg(dropMon_);
        if ( -1 == rename( (getCS()->getStorePath() + fname + ".tmp").c_str(),
                           (getCS()->getStorePath() + fname + ".new").c_str() ) ) {
            throw ErrnoException(errno,"rename(%s)",fname);
        }
        dropFileOffset_ = 0;
        // if ( minRlast < minMsgId_ ) { minMsgId_ = minRlast; }
        dropMon_.notify();

        smsc_log_debug(is_.log_,"D=%u after new blklist msgs added: dropFileOffset=%u min=%llu max=%llu count=%u",
                       is_.getDlvId(), unsigned(dropFileOffset_),
                       ulonglong(minMsgId_), ulonglong(maxMsgId_),
                       unsigned(dropMsgHash_.Count()) );
    }
    writeActLog(100);
    return true;
}


size_t InputStorage::BlackList::openFile( FileGuard& fg,
                                          msgid_type minRlast,
                                          bool* isNew )
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
        smsc_log_debug(is_.log_,"D=%u blklist l=%llu m=%llu r=%llu mval=%llu",
                       is_.getDlvId(),
                       ulonglong(left), ulonglong(middle), ulonglong(right),
                       ulonglong(midVal));
        if (midVal > minRlast) {
            right = middle;
        } else {
            left = middle+itemsize;
            if (midVal == minRlast) { break; }
        }
    }
    if (left>maxSize) {
        throw InfosmeException(EXC_LOGICERROR,"blklist algo failed, left=%llu, maxSize=%llu",
                               ulonglong(left),ulonglong(maxSize));
    }
    fg.seek(left);
    smsc_log_debug(is_.log_,"D=%u blklist position for M=%llu found: %u",
                   is_.getDlvId(), ulonglong(minRlast),unsigned(left));
    return size_t(maxSize);
}


size_t InputStorage::BlackList::readBuf( FileGuard& fg, Buf& buf )
{
    const size_t requested = itemsize * getCS()->getSkippedIdsChunkSize();
    buf.setSize( requested );
    const size_t wasread = fg.read( buf.get(), requested );
    if ( wasread % itemsize != 0 ) {
        throw InfosmeException(EXC_BADFILE, "blklist wasread=%llu, not divided by %u",
                               ulonglong(wasread), unsigned(itemsize));
    }
    return wasread;
}


/// read the next chunk from blacklist file
bool InputStorage::BlackList::readChunk( FileGuard& fg, Buf& buf )
{
    const size_t wasread = readBuf(fg,buf);
    smsc_log_debug(is_.log_,"D=%u read blklist chunk at pos %llu, size=%u",
                   is_.getDlvId(), ulonglong(fg.getPos()-wasread),unsigned(wasread));
    dropFileOffset_ += wasread;
    if (wasread==0) {
        smsc_log_debug(is_.log_,"D=%u blklist file is processed, offset=-1",is_.getDlvId());
        dropFileOffset_ = size_t(-1);
        return false;
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
            throw InfosmeException(EXC_LOGICERROR,"D=%u blklist not sorted",is_.getDlvId());
        }
        maxMsgId_ = msgId;
    }
    smsc_log_debug(is_.log_,"D=%u after blklist chunk read: min=%llu max=%llu count=%u offset=%u",
                   is_.getDlvId(), ulonglong(minMsgId_), ulonglong(maxMsgId_),
                   unsigned(dropMsgHash_.Count()),
                   unsigned(dropFileOffset_));
    return true;
}


void InputStorage::BlackList::writeActLog( unsigned sleepTime )
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
        if ( getCS()->isStopping()) { break; }
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
        for ( size_t i = 0; i < wasread; i += itemsize ) {
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

}
}
