#include <vector>
#include <cassert>
#include <algorithm>
#include "InputStorage.h"
#include "informer/data/CommonSettings.h"
#include "informer/data/DeliveryActivator.h"
#include "informer/data/Region.h"
#include "informer/io/FileGuard.h"
#include "informer/io/FileReader.h"
#include "informer/io/IOConverter.h"
#include "informer/io/HexDump.h"
#include "core/buffers/TmpBuf.hpp"

namespace {
using namespace eyeline::informer;

const unsigned LENSIZE = 2;
const uint16_t defaultVersion = 1;

class IReader : public FileReader::RecordReader
{
public:
    IReader( DeliveryActivator& core, 
             MessageList& ml,
             InputRegionRecord& iro ) : core_(core), msglist_(ml), ro_(iro) {}

    virtual bool isStopping() {
        return core_.isStopping();
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
    virtual void readRecordData( size_t filePos, FromBuf& fb ) {
        msglist_.push_back(MessageLocker());
        MessageLocker& mlk = msglist_.back();
        mlk.serial = 0;
        const uint16_t version = fb.get16();
        if (version!=1) {
            throw InfosmeException(EXC_BADFILE,"unsupported version %u at %llu",
                                   version,ulonglong(filePos));
        }
        Message& msg = mlk.msg;
        msg.subscriber = fb.get64();
        msg.msgId = fb.get64();
        msg.lastTime = fb.get32();
        msg.timeLeft = fb.get32();
        msg.userData = fb.getCString();
        msg.state = fb.get8();
        if (msg.state & 0x80) {
            msg.state &= 0x7f;
            msg.text.reset(new MessageText(fb.getCString(),0));
        } else {
            // FIXME: optimize if textptr already has glossary!
            msg.text.reset(new MessageText(0,fb.get32()));
        }
        ro_.roff += fb.getLen();
    }
private:
    DeliveryActivator& core_;
    MessageList&       msglist_;
    InputRegionRecord& ro_;
};

}


namespace eyeline {
namespace informer {

InputStorage::InputStorage( DeliveryActivator& core,
                            InputJournal&      jnl ) :
log_(smsc::logger::Logger::getInstance("instore")),
core_(core),
rollingIter_(recordList_.end()),
jnl_(jnl),
lastfn_(0),
activityLog_(0),
lastMsgId_(0)
{
}


InputStorage::~InputStorage()
{
}


void InputStorage::init( ActivityLog& actLog )
{
    activityLog_ = &actLog;
    try {
        glossary_.init( jnl_.getCS().getStorePath(), getDlvId());
    } catch ( std::exception& e ) {
        smsc_log_error(log_,"D=%u glossary init failed: %s",getDlvId(),e.what());
        throw;
    }
}


void InputStorage::addNewMessages( MsgIter begin, MsgIter end )
{
    std::vector< regionid_type > regs;
    regs.reserve(32);
    dispatchMessages(begin, end, regs);
    msgtime_type currentTime(msgtime_type(currentTimeMicro()/tuPerSec));
    // FIXME: we need to bind to glossary before
    for ( MsgIter i = begin; i != end; ++i ) {
        glossary_.bindText(i->msg.text);
        activityLog_->addRecord( currentTime, i->serial, i->msg, 0);
    }
    core_.deliveryRegions( getDlvId(), regs, true );
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
    smsc_log_debug(log_,"set input record for R=%u/D=%u: RP=%u/%u, WP=%u/%u, count=%u, msgs=%llu, lastfn=%u",
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
    for ( RecordHash::Iterator i(recordHash_); i.Next(regId,iter); ) {
        if ( iter->rfn == iter->wfn && iter->roff == iter->woff ) {
            continue; 
        }
        regs.push_back(regionid_type(regId));
    }
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
    unsigned count = 0;
    for ( MsgIter i = begin; i != end; ++i ) {
        const regionid_type regId = rf.findRegion( i->msg.subscriber );
        i->msg.msgId = ++lastMsgId_;
        i->msg.state = MSGSTATE_INPUT;
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
        ++count;
    }
    smsc_log_debug(log_,"D=%u add %u new messages, regions: [%s]",
                   unsigned(getDlvId()),count,
                   formatRegionList(regs.begin(),regs.end()).c_str());
    // writing regions
    smsc::core::buffers::TmpBuf<unsigned char,200> msgbuf;
    for ( std::vector<regionid_type>::const_iterator ir = regs.begin();
          ir != regs.end(); ++ir ) {
        const regionid_type regId = *ir;
        smsc_log_debug(log_,"processing R=%u",unsigned(regId));
        InputRegionRecord ro;
        ro.regionId = regId;
        getRecord(ro);
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
                msgbuf.setSize(90+strlen(msg.text->getText()));
            }
            msg.lastTime = ro.wfn;
            msg.timeLeft = ro.woff;
            maxMsgId = msg.msgId;
            ToBuf tb(msgbuf.get(),msgbuf.getSize());
            tb.skip(LENSIZE);
            tb.set16(::defaultVersion);
            // i->msg.toBuf(::defaultVersion,tb);
            tb.set64(msg.subscriber);
            tb.set64(msg.msgId);
            tb.set32(msg.lastTime);
            tb.set32(msg.timeLeft);
            tb.setCString(msg.userData.c_str());
            if (msg.isTextUnique()) {
                tb.set8(msg.state | 0x80);
                tb.setCString(msg.text->getText());
            } else {
                tb.set8(msg.state & 0x7f);
                tb.set32(msg.text->getTextId());
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


void InputStorage::doTransfer( TransferRequester& req, unsigned count )
{
    const regionid_type regId = req.getRegionId();
    smsc_log_debug(log_,"transfer R=%u/D=%u started, count=%u", regId, getDlvId(), count);
    try {
        InputRegionRecord ro;
        ro.regionId = regId;
        getRecord(ro);
        if (ro.rfn==0) { ro.rfn=1; ro.roff=0; }
        FileGuard fg;
        MessageList msglist;
        smsc::core::buffers::TmpBuf<char,8192> buf;
        while (count>0) {

            if (!fg.isOpened()) {
                // need new file
                if ( ro.rfn<ro.wfn || (ro.rfn==ro.wfn && ro.roff<ro.woff)) {
                    // ok
                } else {
                    smsc_log_debug(log_,"R=%u/D=%u no more files to read", regId, getDlvId());
                    break;
                }
                const std::string fname = makeFilePath(regId,ro.rfn);
                smsc_log_debug(log_,"R=%u/D=%u trying to open new file '%s'",
                               regId, getDlvId(), fname.c_str());
                fg.ropen(fname.c_str());
                fg.seek(ro.roff);
            }

            try {
                FileReader fileReader(fg);
                IReader recordReader(core_,msglist,ro);
                fileReader.readRecords(buf,recordReader,count);
                fg.close();
                if (ro.rfn<ro.wfn) {
                    ++ro.rfn;
                    ro.roff=0;
                }
            } catch ( FileDataException& e ) {
                if (ro.rfn < ro.wfn) {
                    smsc_log_error(log_,"R=%u/D=%u RP=%u/%u: garbled intermediate file: %s",
                                   regId, getDlvId(), ro.rfn, ro.roff, e.what());
                    throw;
                }
                smsc_log_debug(log_,"R=%u/D=%u RP=%u/%u: last record garbled, wait until write finish",
                               regId, getDlvId(), ro.rfn, ro.roff );
                break;
            } catch ( std::exception& e ) {
                smsc_log_error(log_,"R=%u/D=%u RP=%u/%u: %s",
                               regId, getDlvId(), ro.rfn, ro.roff, e.what());
            }

        } // while we need more messages

        // we have read things
        if ( ! msglist.empty() ) {
            // write back record
            for ( MessageList::iterator i = msglist.begin(); i != msglist.end(); ++i ) {
                glossary_.bindText( i->msg.text );
            }

            const msgtime_type currentTime(msgtime_type(currentTimeMicro()/tuPerSec));
            req.addNewMessages( currentTime,
                                msglist,
                                msglist.begin(),
                                msglist.end() );
            setRecord(ro,0);
        }

    } catch (std::exception& e) {
        smsc_log_error(log_,"transfer failed R=%u/D=%u: %s",
                       regId, getDlvId(), e.what());
    }
    smsc_log_debug(log_,"transfer task R=%u/D=%u finished", regId, getDlvId());
}


void InputStorage::getRecord( InputRegionRecord& ro )
{
    {
        smsc::core::synchronization::MutexGuard mg(lock_);
        RecordList::iterator* ptr = recordHash_.GetPtr(ro.regionId);
        if (!ptr) {
            ro.clear();
            recordHash_.Insert(ro.regionId,recordList_.insert(recordList_.begin(),ro));
        } else {
            ro = **ptr;
        }
    }
    smsc_log_debug(log_,"got record for R=%u/D=%u: RP=%u/%u, WP=%u/%u, count=%u",
                   unsigned(ro.regionId),
                   unsigned(getDlvId()),
                   unsigned(ro.rfn), unsigned(ro.roff),
                   unsigned(ro.wfn), unsigned(ro.woff),
                   unsigned(ro.count) );
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
    smsc_log_debug(log_,"set record for R=%u/D=%u: RP=%u/%u, WP=%u/%u, count=%u",
                   unsigned(ro.regionId),
                   unsigned(getDlvId()),
                   unsigned(ro.rfn), unsigned(ro.roff),
                   unsigned(ro.wfn), unsigned(ro.woff),
                   unsigned(ro.count) );
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
}


std::string InputStorage::makeFilePath( regionid_type regId, uint32_t fn ) const
{
    if (fn==0) return "";
    char buf[70];
    sprintf(makeDeliveryPath(getDlvId(),buf),"new/%u/%u.data",unsigned(regId),unsigned(fn));
    smsc_log_debug(log_,"filepath for R=%u/D=%u/F=%u is %s",
                   unsigned(regId),
                   unsigned(getDlvId()),
                   unsigned(fn),buf);
    return jnl_.getCS().getStorePath() + buf;
}

}
}
