#include <vector>
#include <cassert>
#include <algorithm>
#include "InputStorage.h"
#include "informer/data/InfosmeCore.h"
#include "informer/data/Region.h"
#include "informer/io/FileGuard.h"
#include "informer/io/IOConverter.h"
#include "core/buffers/TmpBuf.hpp"

namespace {
const unsigned defaultVersion = 1;
}


namespace eyeline {
namespace informer {

InputStorage::InputStorage( InfosmeCore& core,
                            dlvid_type   dlvId,
                            InputJournal& jnl ) :
log_(smsc::logger::Logger::getInstance("instore")),
core_(core),
jnl_(jnl),
lastfn_(0),
dlvId_(dlvId),
lastMsgId_(0)
{
    smsc_log_debug(log_,"ctor for D=%u, FIXME: read journal", unsigned(dlvId_));
}


InputStorage::~InputStorage()
{
    smsc_log_debug(log_,"dtor");
}


void InputStorage::addNewMessages( MsgIter begin, MsgIter end )
{
    std::vector< regionid_type > regs;
    regs.reserve(32);
    dispatchMessages(begin, end, regs);
    core_.deliveryRegions( dlvId_, regs, true );
}


TransferTask* InputStorage::startTransferTask( TransferRequester& requester,
                                               unsigned           count,
                                               bool               mayDetachRegion )
{
    TransferTask* task = 0;
    if (count==0) return task;
    const regionid_type regId = requester.getRegionId();
    try {
        smsc_log_debug(log_,"start transfer task R=%u/D=%u for %u msgs, mayDetach=%u",
                       unsigned(regId), unsigned(dlvId_), count, mayDetachRegion );
        InputRegionRecord ro;
        getRecord(regId,ro);
        if (ro.rfn==0) { ro.rfn=1; ro.roff=0; }
        if (ro.rfn<ro.wfn || (ro.rfn==ro.wfn && ro.roff<ro.woff)) {
            // ok
            task = new InputTransferTask(requester,count,*this);
            core_.startTransfer(task);
        } else {
            // no data
            smsc_log_debug(log_,"R=%u/D=%u data is not ready: rfn=%u, roff=%u, wfn=%u, woff=%u",
                           regId, dlvId_,
                           ro.rfn, ro.roff, ro.wfn, ro.woff );
        }
    } catch ( std::exception& e ) {
        smsc_log_error(log_,"cannot start transfer task: %s", e.what());
    }
    if (!task && mayDetachRegion) {
        std::vector<regionid_type> regs;
        regs.push_back(regId);
        core_.deliveryRegions(dlvId_,regs,false);
    }
    return task;
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
        i->msg.state = MsgState::input;
        i->locked = regId;
        if (log_->isDebugEnabled()) {
            uint8_t ton, npi;
            uint64_t addr = subscriberToAddress(i->msg.subscriber,ton,npi);
            smsc_log_debug(log_,"adding D=%u/M=%llu for .%u.%u.%llu -> R=%u",
                           unsigned(dlvId_), ulonglong(i->msg.msgId),
                           ton,npi,ulonglong(addr),unsigned(regId));
        }
        if ( std::find(regs.begin(),regs.end(),regId) == regs.end() ) {
            regs.push_back(regId);
        }
        ++count;
    }
    smsc_log_debug(log_,"D=%u add %u new messages, regions: [%s]",
                   unsigned(dlvId_),count,
                   formatRegionList(regs.begin(),regs.end()).c_str());
    // writing regions
    smsc::core::buffers::TmpBuf<unsigned char,200> msgbuf;
    for ( std::vector<regionid_type>::const_iterator ir = regs.begin();
          ir != regs.end(); ++ir ) {
        const regionid_type regId = *ir;
        smsc_log_debug(log_,"processing R=%u",unsigned(regId));
        InputRegionRecord ro;
        getRecord(regId,ro);
        FileGuard fg;
        fg.create(makeFilePath(regId,ro.wfn).c_str());
        fg.seek(ro.woff);
        msgid_type maxMsgId = 0;
        for ( MsgIter i = begin; i != end; ++i ) {
            if (i->locked != regId) continue;
            if (ro.count % fileSize == 0) {
                // need to create new file.
                ro.wfn = ++lastfn_;
                ro.woff = 0;
                std::string fname = makeFilePath(regId,ro.wfn);
                smsc_log_debug(log_,"D=%u/R=%u new file: '%s'",
                               unsigned(dlvId_), unsigned(regId), fname.c_str() );
                fg.create(fname.c_str(),true,true);
                // fg.seek(ro.woff);
            }
            // writing to a file
            if (i->msg.isTextUnique()) {
                msgbuf.setSize(90+strlen(i->msg.text->getText()));
            }
            i->msg.lastTime = ro.wfn;
            i->msg.timeLeft = ro.woff;
            maxMsgId = i->msg.msgId;
            ToBuf tb(msgbuf.get(),msgbuf.getSize());
            tb.skip(4);
            const unsigned buflen = 
                unsigned(i->msg.toBuf(::defaultVersion,tb).buf - msgbuf.get());
            tb.setPos(0);
            tb.set32(buflen-4);
            fg.write(msgbuf.get(),buflen);
            ro.woff += buflen;
            ++ro.count;
        }
        fg.close();
        // setting data back
        setRecord(regId,ro,maxMsgId);
    }
}


void InputStorage::doTransfer( TransferRequester& req, unsigned count )
{
    const regionid_type regId = req.getRegionId();
    smsc_log_debug(log_,"transfer R=%u/D=%u started, count=%u", regId, dlvId_, count);
    try {
        smsc::core::buffers::TmpBuf<unsigned char,200> buf;
        InputRegionRecord ro;
        getRecord(regId,ro);
        if (ro.rfn==0) { ro.rfn=1; ro.roff=0; }
        FileGuard fg;
        MessageList msglist;
        while (count>0) {
            if (!fg.isOpened()) {
                // need new file
                if ( ro.rfn<ro.wfn || (ro.rfn==ro.wfn && ro.roff<ro.woff)) {
                    // ok
                } else {
                    smsc_log_debug(log_,"R=%u/D=%u no more files to read", regId, dlvId_);
                    break;
                }
                const std::string fname = makeFilePath(regId,ro.rfn);
                smsc_log_debug(log_,"R=%u/D=%u trying to open new file '%s'",
                               regId, dlvId_, fname.c_str());
                fg.ropen(fname.c_str());
                fg.seek(ro.roff);
            }
            // reading a message length
            unsigned readlen = fg.read(buf,4);
            if (readlen == 0) {
                smsc_log_debug(log_,"R=%u/D=%u rfn=%u, roff=%u: EOF",
                               regId, dlvId_, ro.rfn, ro.roff );
                fg.close();
                if (ro.rfn<ro.wfn) {
                    ++ro.rfn;
                    ro.roff=0;
                }
                continue;
            } else if (readlen != 4) {
                throw InfosmeException("FIXME: R=%u/D=%u rfn=%u, roff=%u is corrupted: cannot read msglen",
                                       regId, dlvId_, ro.rfn, ro.roff );
                
            }
            ro.roff += 4;
            FromBuf fb(buf.get(),4);
            const unsigned msglen = fb.get32();
            if (msglen>10000) {
                throw InfosmeException("FIXME: R=%u/D=%u rfn=%u, roff=%u is corrupted: msglen=%u is too big",
                                       regId, dlvId_, ro.rfn, ro.roff, msglen);
            }
            buf.setSize(msglen);
            readlen = fg.read(buf,msglen);
            if (readlen != msglen) {
                throw InfosmeException("FIXME: R=%u/D=%u rfn=%u, roff=%u is corrupted: msglen=%u != readlen=%u",
                                       regId, dlvId_, ro.rfn, ro.roff, msglen, readlen);
            }
            ro.roff += msglen;
            msglist.push_back(MessageLocker());
            MessageLocker& mlk = msglist.back();
            mlk.locked = 0;
            fb.buflen = msglen;
            fb.setPos(0);
            readlen = unsigned(mlk.msg.fromBuf(::defaultVersion,fb).buf - buf.get());
            if (readlen != msglen) {
                throw InfosmeException("FIXME: R=%u/D=%u rfn=%u, roff=%u is corrupted: msglen=%u != msg.len=%u",
                                       regId, dlvId_, ro.rfn, ro.roff, msglen, readlen);
            }
            glossary_.bindMessage(mlk.msg.text);
            --count;
        } // while we need more messages
        if ( ! msglist.empty() ) {
            // write back record
            const msgtime_type currentTime = currentTimeMicro() / tuPerSec;
            req.addNewMessages( currentTime,
                                msglist,
                                msglist.begin(),
                                msglist.end() );
            setRecord(regId,ro,0);
        }

    } catch (std::exception& e) {
        smsc_log_error(log_,"transfer failed R=%u/D=%u: %s",
                       regId, dlvId_, e.what());
    }
    smsc_log_debug(log_,"transfer task R=%u/D=%u finished", regId, dlvId_);
}


void InputStorage::getRecord( regionid_type regId, InputRegionRecord& ro )
{
    {
        smsc::core::synchronization::MutexGuard mg(lock_);
        InputRegionRecord* ptr = regions_.GetPtr(regId);
        if (!ptr) {
            ro.clear();
            regions_.Insert(regId,ro);
        } else {
            ro = *ptr;
        }
    }
    smsc_log_debug(log_,"got record for D=%u R=%u: rfn=%u, roff=%u, wfn=%u, woff=%u, count=%u",
                   unsigned(dlvId_), unsigned(regId),
                   unsigned(ro.rfn), unsigned(ro.roff),
                   unsigned(ro.wfn), unsigned(ro.woff),
                   unsigned(ro.count) );
}


void InputStorage::setRecord( regionid_type regId, InputRegionRecord& ro, uint64_t maxMsgId )
{
    {
        smsc::core::synchronization::MutexGuard mg(lock_);
        InputRegionRecord* ptr = regions_.GetPtr(regId);
        assert(ptr);
        if (ro.rfn>ptr->rfn) {
            ptr->rfn = ro.rfn;
            ptr->roff = ro.roff;
        } else if (ro.rfn==ptr->rfn && ro.roff>ptr->roff) {
            ptr->roff = ro.roff;
        }
        if (ro.wfn>ptr->wfn) {
            ptr->wfn = ro.wfn;
            ptr->woff = ro.woff;
        } else if (ro.wfn==ptr->wfn && ro.woff>ptr->woff) {
            ptr->woff = ro.woff;
        }
        if (ro.count>ptr->count) {
            ptr->count = ro.count;
        }
        ro = *ptr;
    }
    smsc_log_debug(log_,"set record for D=%u R=%u: rfn=%u, roff=%u, wfn=%u, woff=%u, count=%u",
                   unsigned(dlvId_), unsigned(regId),
                   unsigned(ro.rfn), unsigned(ro.roff),
                   unsigned(ro.wfn), unsigned(ro.woff),
                   unsigned(ro.count) );
    jnl_.journalRecord(dlvId_,regId,ro,maxMsgId);
}


std::string InputStorage::makeFilePath( regionid_type regId, uint32_t fn ) const
{
    if (fn==0) return "";
    char buf[50];
    sprintf(buf,"%u/%u/%u.data",unsigned(dlvId_),unsigned(regId),unsigned(fn));
    smsc_log_debug(log_,"filepath for R=%u F=%u is %s",unsigned(regId),unsigned(fn),buf);
    return jnl_.getPath() + buf;
}

}
}
