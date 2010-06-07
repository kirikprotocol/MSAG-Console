#include <cassert>
#include "RegionalStorage.h"
#include "TaskInfo.h"
#include "StoreLog.h"
#include "scag/util/RelockMutexGuard.h"
#include "scag/util/UnlockMutexGuard.h"

using namespace smsc::core::synchronization;
using namespace scag2::util;

namespace scag2 {
namespace prototypes {
namespace infosme {

class RegionalStorage::MsgLock
{
public:
    /// it also move iter
    MsgLock(MsgIter iter, RegionalStorage* s) :
    iter_(iter),
    rs_(s)
    {
        // rs must be locked
        if (iter->locked) {
            smsc::core::synchronization::Condition& c = rs_->getCnd(iter);
            while (iter->locked) {
                c.WaitOn(rs_->cacheLock_);
            }
        }
        iter->locked = true;
        if (iter == rs_->storingIter_) {
            ++rs_->storingIter_;
        }
        // moving iter before begin
        MessageList& l = rs_->messageList_;
        l.splice( rs_->validItems_, l, iter );
        rs_->validItems_ = iter;
    }

    ~MsgLock()
    {
        MutexGuard mg(rs_->cacheLock_);
        iter_->locked = false;
        rs_->getCnd(iter_).Signal();
    }
private:
    MsgIter          iter_;
    RegionalStorage* rs_;
};



RegionalStorage::RegionalStorage( const DlvInfo&  dlvInfo,
                                  regionid_type   regionId,
                                  StoreLog&       storeLog,
                                  MessageSource&  messageSource ) :
log_(smsc::logger::Logger::getInstance("str.reg")),
storingIter_(messageList_.end()),
validItems_(messageList_.begin()),
alog_(&storeLog),
messageSource_(&messageSource),
uploadTasks_(0),
dlvInfo_(&dlvInfo),
regionId_(regionId),
usage_(0)
{
    smsc_log_debug(log_,"ctor task=%u region=%u",dlvInfo_->getDlvId(),regionId);
}


RegionalStorage::~RegionalStorage()
{
    MutexGuard mg(cacheLock_);
    while (uploadTasks_ > 0) {
        UnlockMutexGuard umg(cacheLock_);
        struct timespec tsp = {0,10000000L};
        nanosleep(&tsp,0);
    }
}


bool RegionalStorage::getMessage( msgid_type msgId, Message& msg )
{
    MutexGuard mg(cacheLock_);
    MsgIter* ptr = messageHash_.GetPtr(msgId);
    if (!ptr) {
        smsc_log_debug(log_,"Message %u:%u is not found in region %u",
                       dlvInfo_->getDlvId(), msgId, regionId_ );
        return false; 
    }
    msg = (*ptr)->msg;
    return true;
}


bool RegionalStorage::getNextMessage( msgtime_type currentTime, Message& msg )
{
    MsgIter iter;
    RelockMutexGuard mg(cacheLock_);
    const char* from;
    do { // fake loop

        /// check if we need to request new messages
        if ( !uploadTasks_ &&
             unsigned(newQueue_.Count()) < dlvInfo_->getMinNewQueueSize() &&
             resendQueue_.size() < dlvInfo_->getMaxResendQueueSize() ) {
            ++uploadTasks_;
            const unsigned newQueueSize = unsigned(newQueue_.Count());
            const unsigned resendQueueSize = unsigned(resendQueue_.size());
            mg.Unlock();
            try {
                smsc_log_debug(log_,"%u/%u wants to request new messages as it has new/resend=%u/%u",
                               dlvInfo_->getDlvId(), getRegionId(),
                               newQueueSize, resendQueueSize);
                messageSource_->requestNewMessages(*this,
                                                   getDlvId(),
                                                   getRegionId(),
                                                   dlvInfo_->getUploadCount());
                mg.Lock();
            } catch (std::exception& e ) {
                smsc_log_warn(log_,"exception requesting new msgs: %s",e.what());
                mg.Lock();
                if (uploadTasks_>0) --uploadTasks_;
            }
        }

        if ( !resendQueue_.empty() ) {
            ResendQueue::iterator v = resendQueue_.begin();
            if ( v->first <= currentTime ) {
                // get from the resendQueue
                iter = v->second;
                from = "resendQueue";
                messageHash_.Insert(iter->msg.msgId,iter);
                resendQueue_.erase(resendQueue_.begin());
                break;
            }
        }

        /// checking newQueue
        if ( newQueue_.Pop(iter) ) {
            // success
            messageHash_.Insert(iter->msg.msgId,iter);
            from = "newQueue";
            break;
        }

        return false;

    } while ( false );

    MsgLock ml(iter,this);
    mg.Unlock();

    Message& m = iter->msg;
    m.lastTime = currentTime;
    // FIXME: reset m.timeLeft for new messages
    m.state = MsgState::process;
    msg = m;
    smsc_log_debug(log_,"taking message %u/%u/%u from %s",
                   dlvInfo_->getDlvId(),m.msgId,regionId_,from);
    alog_->writeMessage(dlvInfo_->getDlvId(),regionId_,m);
    return true;
}


void RegionalStorage::messageSent( msgid_type msgId,
                                   msgtime_type currentTime )
    // const char* receipt )
{
    RelockMutexGuard mg(cacheLock_);
    MsgIter* ptr = messageHash_.GetPtr(msgId);
    if (!ptr) {
        throw smsc::util::Exception("message %u:%u is not found in region %u",
                                    dlvInfo_->getDlvId(),msgId,regionId_);
    }
    MsgLock ml(*ptr,this);
    mg.Unlock();
    Message& m = (*ptr)->msg;
    m.lastTime = currentTime;
    m.state = MsgState::sent;
    alog_->writeMessage(dlvInfo_->getDlvId(),regionId_,m);
}


void RegionalStorage::retryMessage( msgid_type   msgId,
                                    msgtime_type currentTime,
                                    msgtime_type retryDelay,  // in how many seconds try again
                                    int          smppState )
{
    RelockMutexGuard mg(cacheLock_);
    MsgIter iter;
    if ( !messageHash_.Pop(msgId,iter) ) {
        // not found
        throw smsc::util::Exception("message %u:%u is not found in region %u",
                                    dlvInfo_->getDlvId(), msgId, regionId_);
    }
    MsgLock ml(iter,this);
    Message& m = iter->msg;
    // fixing time left according
    m.timeLeft -= currentTime - m.lastTime;
    if ( m.timeLeft > dlvInfo_->getMinRetryTime() ) {
        // there is enough validity time to try the next time
        if ( m.timeLeft < retryDelay ) retryDelay = m.timeLeft;
        retryDelay += currentTime;
        resendQueue_.insert( std::make_pair(retryDelay,iter) );
        mg.Unlock();
        m.lastTime = currentTime;
        m.state = MsgState::retry;
        char fmtime[20];
        smsc_log_debug(log_,"put message %u:%u into region %u retry at %s",
                       dlvInfo_->getDlvId(), msgId, regionId_, formatMsgTime(fmtime,retryDelay) );
        alog_->writeMessage(dlvInfo_->getDlvId(),regionId_,m);
    } else {
        // not enough time to retry
        // moving element to a not-valid part of the list
        assert(validItems_ == iter);
        ++validItems_;
        mg.Unlock();
        m.lastTime = currentTime;
        m.state = MsgState::expired;
        smsc_log_debug(log_,"message %u:%u is expired in region %u, smpp=%u",
                       dlvInfo_->getDlvId(), msgId, regionId_, smppState );
        alog_->writeMessage(dlvInfo_->getDlvId(),regionId_,m);
        destroy(m);
    }
}


void RegionalStorage::finalizeMessage( msgid_type   msgId,
                                       msgtime_type currentTime,
                                       uint8_t      state,
                                       int          smppState )
{
    RelockMutexGuard mg(cacheLock_);
    MsgIter iter;
    if ( !messageHash_.Pop(msgId,iter) ) {
        throw smsc::util::Exception("message %u:%u is not found in region %u",
                                    dlvInfo_->getDlvId(), msgId, regionId_ );
    }
    MsgLock ml(iter,this);
    assert(validItems_ == iter);
    ++validItems_;
    mg.Unlock();
    Message& m = iter->msg;
    m.lastTime = currentTime;
    m.state = state;
    smsc_log_debug(log_,"message %u:%u is finalized in region %u, state=%u, smpp=%u",
                   dlvInfo_->getDlvId(),msgId,regionId_,state,smppState);
    alog_->writeMessage(dlvInfo_->getDlvId(),regionId_,m);
    destroy(m);
}


void RegionalStorage::addNewMessages( msgtime_type currentTime,
                                      MessageList& listFrom,
                                      MsgIter      iter1,
                                      MsgIter      iter2 )
{
    smsc_log_debug(log_,"adding new messages to storage");
    for ( MsgIter i = iter1; i != iter2; ++i ) {
        Message& m = i->msg;
        // m.lastTime = currentTime;
        m.state = MsgState::input;
        alog_->writeMessage(dlvInfo_->getDlvId(),regionId_,m);
    }
    MutexGuard mg(cacheLock_);
    for ( MsgIter i = iter1; i != iter2; ++i ) {
        smsc_log_debug(log_,"inserting msg %u into new queue",i->msg.msgId);
        newQueue_.Push(i);
    }
    messageList_.splice( validItems_, listFrom, iter1, iter2 );
    validItems_ = iter1;
    if ( log_->isDebugEnabled() ) {
        std::string s;
        s.reserve(300);
        unsigned count = 0;
        for ( MsgIter i = messageList_.begin(); i != messageList_.end(); ++i ) {
            if ( i == storingIter_ ) {
                s.append(" s");
            }
            if ( i == validItems_ ) {
                s.append(" v");
            }
            char buf[10];
            sprintf(buf," %u",i->msg.msgId);
            s.append(buf);
            ++count;
        }
        smsc_log_debug(log_,"messages (%u) in %u/%u: %s",
                       count,dlvInfo_->getDlvId(),regionId_,s.c_str());
    }
}


void RegionalStorage::rollOver()
{
    RelockMutexGuard mg(cacheLock_);
    if ( storingIter_ != messageList_.end() ) {
        smsc_log_debug(log_,"rolling is already in progress");
        return;
    }
    // cleanup invalid elements
    for ( MsgIter iter = messageList_.begin();
          iter != validItems_;
          ) {
        if ( ! iter->locked ) {
            iter = messageList_.erase(iter);
        } else {
            ++iter;
        }
    }

    storingIter_ = messageList_.begin();
    while ( true ) {
        MsgIter iter = storingIter_;
        if ( iter == messageList_.end() ) break;
        {
            MsgLock ml(iter,this);
            mg.Unlock();
            alog_->writeMessage(dlvInfo_->getDlvId(),regionId_,iter->msg);
        }
        // FIXME: restriction on throughput
        mg.Lock();
    }
}


void RegionalStorage::uploadFinished()
{
    MutexGuard mg(cacheLock_);
    if (uploadTasks_ > 0) --uploadTasks_;
    // FIXME: notify
}


void RegionalStorage::destroy( Message& msg )
{
    smsc_log_debug(log_,"dtor message (%u/%u)",dlvInfo_->getDlvId(),msg.msgId);
}


void RegionalStorage::usage( bool incr )
{
    if ( incr ) {
        MutexGuard mg(cacheLock_);
        ++usage_;
    } else {
        {
            MutexGuard mg(cacheLock_);
            if (!usage_) {
                std::terminate();
            }
            if (--usage_) return;
        }
        delete this;
    }
}

}
}
}
