#include <cassert>
#include "RegionalStorage.h"
#include "UnlockMutexGuard.h"
#include "RelockMutexGuard.h"
#include "StoreJournal.h"

namespace eyeline {
namespace informer {

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



RegionalStorage::RegionalStorage( const DeliveryInfo&  dlvInfo,
                                  regionid_type        regionId,
                                  StoreJournal&        storeLog,
                                  InputMessageSource&  messageSource ) :
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
    smsc_log_debug(log_,"ctor R=%u/D=%u",
                   unsigned(regionId),unsigned(dlvInfo_->getDlvId()));
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
        smsc_log_debug(log_,"Message R=%u/D=%u/M=%u is not found (getMessage)",
                       unsigned(regionId_), unsigned(dlvInfo_->getDlvId()), unsigned(msgId));
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
             unsigned(newQueue_.Count()) <= dlvInfo_->getMinInputQueueSize() &&
             resendQueue_.size() <= dlvInfo_->getMaxResendQueueSize() ) {
            ++uploadTasks_;
            const unsigned newQueueSize = unsigned(newQueue_.Count());
            const unsigned resendQueueSize = unsigned(resendQueue_.size());
            mg.Unlock();
            try {
                smsc_log_debug(log_,"R=%u/D=%u wants to request input messages as it has new/resend=%u/%u",
                               unsigned(regionId_),
                               unsigned(dlvInfo_->getDlvId()),
                               newQueueSize, resendQueueSize);
                messageSource_->requestInputMessages(*this,
                                                     dlvInfo_->getUploadCount());
                mg.Lock();
            } catch (std::exception& e ) {
                smsc_log_warn(log_,"exception requesting input msgs in R=%u/D=%u: %s",
                              unsigned(regionId_),
                              unsigned(dlvInfo_->getDlvId()),
                              e.what());
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
    smsc_log_debug(log_,"taking message R=%u/D=%u/M=%u from %s",
                   unsigned(regionId_),
                   unsigned(dlvInfo_->getDlvId()),
                   unsigned(m.msgId),from);
    alog_->journalMessage(dlvInfo_->getDlvId(),regionId_,m);
    return true;
}


void RegionalStorage::messageSent( msgid_type msgId,
                                   msgtime_type currentTime )
    // const char* receipt )
{
    RelockMutexGuard mg(cacheLock_);
    MsgIter* ptr = messageHash_.GetPtr(msgId);
    if (!ptr) {
        throw smsc::util::Exception("message R=%u/D=%u/M=%u is not found (messageSent)",
                                    unsigned(regionId_),
                                    unsigned(dlvInfo_->getDlvId()),
                                    unsigned(msgId));
    }
    MsgLock ml(*ptr,this);
    mg.Unlock();
    Message& m = (*ptr)->msg;
    m.lastTime = currentTime;
    m.state = MsgState::sent;
    alog_->journalMessage(dlvInfo_->getDlvId(),regionId_,m);
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
        throw smsc::util::Exception("message R=%u/D=%u/M=%u is not found (retryMessage)",
                                    unsigned(regionId_),
                                    unsigned(dlvInfo_->getDlvId()),
                                    unsigned(msgId));
    }
    MsgLock ml(iter,this);
    Message& m = iter->msg;
    // fixing time left according
    m.timeLeft -= timediff_type(time_t(currentTime)-time_t(m.lastTime));
    if ( m.timeLeft > int(dlvInfo_->getMinRetryTime()) ) {
        // there is enough validity time to try the next time
        if ( m.timeLeft < int(retryDelay) ) retryDelay = m.timeLeft;
        retryDelay += currentTime;
        resendQueue_.insert( std::make_pair(retryDelay,iter) );
        mg.Unlock();
        m.lastTime = currentTime;
        m.state = MsgState::retry;
        char fmtime[20];
        smsc_log_debug(log_,"put message R=%u/D=%u/M=%u into retry at %s",
                       unsigned(regionId_),
                       unsigned(dlvInfo_->getDlvId()),
                       unsigned(msgId), formatMsgTime(fmtime,retryDelay) );
        alog_->journalMessage(dlvInfo_->getDlvId(),regionId_,m);
    } else {
        // not enough time to retry
        // moving element to a not-valid part of the list
        assert(validItems_ == iter);
        ++validItems_;
        mg.Unlock();
        m.lastTime = currentTime;
        m.state = MsgState::expired;
        smsc_log_debug(log_,"message R=%u/D=%u/M=%u is expired, smpp=%u",
                       unsigned(regionId_),
                       unsigned(dlvInfo_->getDlvId()),
                       unsigned(msgId), smppState );
        alog_->journalMessage(dlvInfo_->getDlvId(),regionId_,m);
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
        throw smsc::util::Exception("message R=%u/D=%u/M=%u is not found (finalizeMessage)",
                                    unsigned(regionId_),
                                    unsigned(dlvInfo_->getDlvId()),
                                    unsigned(msgId));
    }
    MsgLock ml(iter,this);
    assert(validItems_ == iter);
    ++validItems_;
    mg.Unlock();
    Message& m = iter->msg;
    m.lastTime = currentTime;
    m.state = state;
    smsc_log_debug(log_,"message R=%u/D=%u/M=%u is finalized, state=%u, smpp=%u",
                   unsigned(regionId_),
                   unsigned(dlvInfo_->getDlvId()),
                   unsigned(msgId),state,smppState);
    alog_->journalMessage(dlvInfo_->getDlvId(),regionId_,m);
    destroy(m);
}


void RegionalStorage::addNewMessages( msgtime_type currentTime,
                                      MessageList& listFrom,
                                      MsgIter      iter1,
                                      MsgIter      iter2 )
{
    smsc_log_debug(log_,"adding new messages to storage R=%u/D=%u",
                   unsigned(regionId_),
                   unsigned(dlvInfo_->getDlvId()) );
    for ( MsgIter i = iter1; i != iter2; ++i ) {
        Message& m = i->msg;
        // m.lastTime = currentTime;
        m.state = MsgState::input;
        smsc_log_debug(log_,"new input msg R=%u/D=%u/M=%u",
                       unsigned(regionId_),
                       unsigned(dlvInfo_->getDlvId()),
                       unsigned(m.msgId));
        alog_->journalMessage(dlvInfo_->getDlvId(),regionId_,m);
    }
    MutexGuard mg(cacheLock_);
    for ( MsgIter i = iter1; i != iter2; ++i ) {
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
            sprintf(buf," %u",unsigned(i->msg.msgId));
            s.append(buf);
            ++count;
        }
        smsc_log_debug(log_,"messages (%u) in R=%u/D=%u: %s",
                       unsigned(regionId_),
                       count,unsigned(dlvInfo_->getDlvId()),
                       s.c_str());
    }
}


void RegionalStorage::rollOver()
{
    RelockMutexGuard mg(cacheLock_);
    if ( storingIter_ != messageList_.end() ) {
        smsc_log_debug(log_,"rolling in R=%u/D=%u is already in progress",
                       unsigned(regionId_),
                       unsigned(dlvInfo_->getDlvId()));
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
            alog_->journalMessage(dlvInfo_->getDlvId(),regionId_,iter->msg);
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
    smsc_log_debug(log_,"dtor message R=%u/D=%u/M=%u",
                   unsigned(regionId_),
                   unsigned(dlvInfo_->getDlvId()),
                   unsigned(msg.msgId));
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
