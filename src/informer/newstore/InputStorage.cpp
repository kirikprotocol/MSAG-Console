#include <vector>
#include <cassert>
#include <algorithm>
#include "InputStorage.h"
#include "informer/data/CommonSettings.h"
#include "informer/data/InfosmeCore.h"
#include "informer/data/Region.h"
#include "informer/io/FileGuard.h"
#include "informer/io/IOConverter.h"
#include "informer/io/HexDump.h"
#include "core/buffers/TmpBuf.hpp"

namespace {
const unsigned LENSIZE = 2;
const uint16_t defaultVersion = 1;
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
lastMsgId_(0),
glossary_(*this)
{
    smsc_log_debug(log_,"ctor for D=%u", unsigned(dlvId_));
}


InputStorage::~InputStorage()
{
    smsc_log_debug(log_,"dtor for D=%u", unsigned(dlvId_));
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
            smsc_log_debug(log_,"R=%u/D=%u data is not ready: RP=%u/%u, WP=%u/%u",
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


void InputStorage::setRecordAtInit( regionid_type regId,
                                    const InputRegionRecord& ro,
                                    uint64_t maxMsgId )
{
    InputRegionRecord* ptr = regions_.GetPtr(regId);
    if (ptr) {
        doSetRecord(*ptr,ro);
    } else {
        ptr = &regions_.Insert(regId,ro);
    }
    if (maxMsgId>lastMsgId_) lastMsgId_ = maxMsgId;
    if (ptr->wfn>lastfn_) lastfn_ = ptr->wfn;
    smsc_log_debug(log_,"set input record for D=%u/R=%u: RP=%u/%u, WP=%u/%u, count=%u, msgs=%llu, lastfn=%u",
                   unsigned(dlvId_), unsigned(regId),
                   unsigned(ptr->rfn), unsigned(ptr->roff),
                   unsigned(ptr->wfn), unsigned(ptr->woff),
                   unsigned(ptr->count), ulonglong(lastMsgId_),
                   unsigned(lastfn_) );
}


void InputStorage::postInit( std::vector<regionid_type>& regs )
{
    int regId;
    InputRegionRecord* ro;
    for ( smsc::core::buffers::IntHash< InputRegionRecord >::Iterator i(regions_);
          i.Next(regId,ro); ) {
        if ( ro->rfn == ro->wfn && ro->roff == ro->woff ) { continue; }
        regs.push_back(regionid_type(regId));
    }
}


void InputStorage::rollOver()
{
    smsc_log_debug(log_,"FIXME: roll over input storage");
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
            if (i->serial != regId) continue;
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
            tb.skip(LENSIZE);
            tb.set16(::defaultVersion);
            i->msg.toBuf(::defaultVersion,tb);
            const size_t buflen = tb.getPos();
            tb.setPos(0);
            tb.set16(buflen-LENSIZE);
            if (log_->isDebugEnabled()) {
                HexDump hd;
                HexDump::string_type dump;
                dump.reserve(hd.hexdumpsize(buflen)+buflen+10);
                hd.hexdump(dump,msgbuf.get(),buflen);
                hd.strdump(dump,msgbuf.get(),buflen);
                smsc_log_debug(log_,"record(%u): %s",buflen,hd.c_str(dump));
            }
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
        InputRegionRecord ro;
        getRecord(regId,ro);
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
                    smsc_log_debug(log_,"R=%u/D=%u no more files to read", regId, dlvId_);
                    break;
                }
                const std::string fname = makeFilePath(regId,ro.rfn);
                smsc_log_debug(log_,"R=%u/D=%u trying to open new file '%s'",
                               regId, dlvId_, fname.c_str());
                fg.ropen(fname.c_str());
                fg.seek(ro.roff);
            }

            char* ptr = buf.get();
            size_t wasread = fg.read(buf.GetCurPtr(),buf.getSize()-buf.GetPos());
            if (wasread == 0) {
                fg.close();
                if (ptr<buf.GetCurPtr()) {
                    smsc_log_warn(log_,"R=%u/D=%u RP=%u/%u: last record garbled, will wait here",
                                  regId, dlvId_, ro.rfn, ro.roff );
                    if (ro.rfn<ro.wfn) {
                        throw InfosmeException("R=%u/D=%u garbled intermediate file", regId, dlvId_ );
                    }
                    break;
                }
                smsc_log_debug(log_,"R=%u/D=%u RP=%u/%u: EOF",
                               regId, dlvId_, ro.rfn, ro.roff );
                if (ro.rfn<ro.wfn) {
                    ++ro.rfn;
                    ro.roff=0;
                }
                continue;
            }
            
            buf.SetPos(buf.GetPos()+wasread);
            smsc_log_debug(log_,"R=%u/D=%u RP=%u/%u read/inbuf=%u/%u bytes",
                           regId, dlvId_, ro.rfn, ro.roff, wasread, buf.GetPos() );
            while (ptr < buf.GetCurPtr()) {
                
                if (ptr+LENSIZE > buf.GetCurPtr()) {
                    // too few items
                    break;
                }
                FromBuf fb(ptr,LENSIZE);
                const uint16_t reclen = fb.get16();
                if (reclen>10000) {
                    throw InfosmeException("FIXME: R=%u/D=%u RP=%u/%u is corrupted: reclen=%u is too big",
                                           regId, dlvId_, ro.rfn, unsigned(fg.getPos()-(buf.GetCurPtr()-ptr)), reclen);
                }
                if ( ptr+LENSIZE+reclen > buf.GetCurPtr() ) {
                    // read more
                    break;
                }
                smsc_log_debug(log_,"record len is %u", reclen);
                fb.setLen(LENSIZE+reclen);
                msglist.push_back(MessageLocker());
                MessageLocker& mlk = msglist.back();
                mlk.serial = 0;
                const uint16_t version = fb.get16();
                mlk.msg.fromBuf(version,fb);
                if ( fb.getPos() != reclen+LENSIZE ) {
                    throw InfosmeException("FIXME: R=%u/D=%u RP=%u/%u has extra data",
                                           regId, dlvId_, ro.rfn, ro.roff );
                }
                glossary_.bindMessage(mlk.msg.text);
                ptr += reclen+LENSIZE;
                ro.roff += reclen+LENSIZE;
                --count;
                if (count==0) break;

            } // while reading the chunk
            
            if (count == 0) break; // done

            if (ptr>buf.get()) {
                // shifting buffer back
                char* o = buf.get();
                const char* i = ptr;
                const char* e = buf.GetCurPtr();
                for ( ; i < e ; ) {
                    *o++ = *i++;
                }
                buf.SetPos(o-buf.get());
            } else if ( buf.GetPos() >= buf.getSize() ) {
                // resize needed
                buf.reserve(buf.getSize()+buf.getSize()/2+100);
            }

        } // while we need more messages

        // we have read things
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
    smsc_log_debug(log_,"got record for D=%u/R=%u: RP=%u/%u, WP=%u/%u, count=%u",
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
        doSetRecord(*ptr,ro);
        ro = *ptr;
    }
    smsc_log_debug(log_,"set record for D=%u/R=%u: RP=%u/%u, WP=%u/%u, count=%u",
                   unsigned(dlvId_), unsigned(regId),
                   unsigned(ro.rfn), unsigned(ro.roff),
                   unsigned(ro.wfn), unsigned(ro.woff),
                   unsigned(ro.count) );
    jnl_.journalRecord(dlvId_,regId,ro,maxMsgId);
}


void InputStorage::doSetRecord( InputRegionRecord& to, const InputRegionRecord& ro )
{
    if (ro.rfn>to.rfn) {
        to.rfn = ro.rfn;
        to.roff = ro.roff;
    } else if (ro.rfn==to.rfn && ro.roff>to.roff) {
        to.roff = ro.roff;
    }

    if (ro.wfn>to.wfn) {
        to.wfn = ro.wfn;
        to.woff = ro.woff;
    } else if (ro.wfn==to.wfn && ro.woff>to.woff) {
        to.woff = ro.woff;
    }

    if (ro.count>to.count) {
        to.count = ro.count;
    }
}


std::string InputStorage::makeFilePath( regionid_type regId, uint32_t fn ) const
{
    if (fn==0) return "";
    char buf[70];
    sprintf(makeDeliveryPath(dlvId_,buf),"%u/%u.data",unsigned(regId),unsigned(fn));
    smsc_log_debug(log_,"filepath for D=%u/R=%u/F=%u is %s",
                   unsigned(dlvId_),unsigned(regId),unsigned(fn),buf);
    return jnl_.getCS().getStorePath() + buf;
}

}
}
