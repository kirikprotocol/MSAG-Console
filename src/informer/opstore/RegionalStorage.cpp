#include <cassert>
#include "RegionalStorage.h"
#include "informer/io/UnlockMutexGuard.h"
#include "informer/io/RelockMutexGuard.h"
#include "informer/data/CommonSettings.h"
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
        if (iter->serial == MessageLocker::lockedSerial) {
            smsc::core::synchronization::Condition& c = rs_->getCnd(iter);
            while (iter->serial == MessageLocker::lockedSerial) {
                c.WaitOn(rs_->cacheMon_);
            }
        }
        serial = iter->serial; // get the previous serial
        iter->serial = MessageLocker::lockedSerial;
        MessageList& l = rs_->messageList_;
        if (iter != l.begin()) {
            // moving iter before begin
            l.splice(l.begin(),l,iter);
        } else if (iter == rs_->storingIter_) {
            // simply move storingiter forward
            ++rs_->storingIter_;
        }
    }

    ~MsgLock()
    {
        MutexGuard mg(rs_->cacheMon_);
        iter_->serial = serial;
        rs_->getCnd(iter_).Signal();
    }
private:
    MsgIter          iter_;
    RegionalStorage* rs_;
public:
    // used to pass to journal
    regionid_type    serial;
};



RegionalStorage::RegionalStorage( DeliveryInfo&        dlvInfo,
                                  regionid_type        regionId,
                                  StoreJournal&        storeJournal,
                                  InputMessageSource&  messageSource ) :
log_(smsc::logger::Logger::getInstance("regstore")),
storingIter_(messageList_.end()),
dlvInfo_(dlvInfo),
storeJournal_(storeJournal),
messageSource_(messageSource),
transferTask_(0),
regionId_(regionId),
ref_(0)
{
    smsc_log_debug(log_,"ctor R=%u/D=%u",
                   unsigned(regionId),unsigned(dlvInfo_.getDlvId()));
}


RegionalStorage::~RegionalStorage()
{
    MutexGuard mg(cacheMon_);
    if (transferTask_) { transferTask_->stop(); }
    while (transferTask_) {
        cacheMon_.wait(100);
    }
}


bool RegionalStorage::getMessage( msgid_type msgId, Message& msg )
{
    MutexGuard mg(cacheMon_);
    MsgIter* ptr = messageHash_.GetPtr(msgId);
    if (!ptr) {
        smsc_log_debug(log_,"Message R=%u/D=%u/M=%u is not found (getMessage)",
                       unsigned(regionId_), unsigned(dlvInfo_.getDlvId()), unsigned(msgId));
        return false; 
    }
    msg = (*ptr)->msg;
    return true;
}


bool RegionalStorage::getNextMessage( msgtime_type currentTime, Message& msg )
{
    MsgIter iter;
    RelockMutexGuard mg(cacheMon_);
    const char* from;
    do { // fake loop

        /// check if we need to request new messages
        if ( !transferTask_ &&
             unsigned(newQueue_.Count()) <= dlvInfo_.getMinInputQueueSize() &&
             resendQueue_.size() <= dlvInfo_.getMaxResendQueueSize() ) {
            // ++uploadTasks_;
            const unsigned newQueueSize = unsigned(newQueue_.Count());
            const unsigned resendQueueSize = unsigned(resendQueue_.size());
            // mg.Unlock();
            try {
                smsc_log_debug(log_,"R=%u/D=%u wants to request input transfer as it has new/resend=%u/%u",
                               unsigned(regionId_),
                               unsigned(dlvInfo_.getDlvId()),
                               newQueueSize, resendQueueSize);
                // mg.Lock();
                transferTask_ = messageSource_.startTransferTask(*this,
                                                                 dlvInfo_.getUploadCount(),
                                                                 resendQueue_.empty() && newQueue_.Count()==0);
            } catch (std::exception& e ) {
                smsc_log_warn(log_,"exception requesting input msgs in R=%u/D=%u: %s",
                              unsigned(regionId_),
                              unsigned(dlvInfo_.getDlvId()),
                              e.what());
                // mg.Lock();
                // if (uploadTasks_>0) --uploadTasks_;
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
    m.timeLeft = dlvInfo_.getMessageValidityTime();
    m.state = MSGSTATE_PROCESS;
    msg = m;
    smsc_log_debug(log_,"taking message R=%u/D=%u/M=%u from %s",
                   unsigned(regionId_),
                   unsigned(dlvInfo_.getDlvId()),
                   unsigned(m.msgId),from);
    storeJournal_.journalMessage(dlvInfo_.getDlvId(),regionId_,m,ml.serial);
    return true;
}


void RegionalStorage::messageSent( msgid_type msgId,
                                   msgtime_type currentTime )
    // const char* receipt )
{
    RelockMutexGuard mg(cacheMon_);
    MsgIter* ptr = messageHash_.GetPtr(msgId);
    if (!ptr) {
        throw smsc::util::Exception("message R=%u/D=%u/M=%u is not found (messageSent)",
                                    unsigned(regionId_),
                                    unsigned(dlvInfo_.getDlvId()),
                                    unsigned(msgId));
    }
    MsgLock ml(*ptr,this);
    mg.Unlock();
    Message& m = (*ptr)->msg;
    m.lastTime = currentTime;
    m.state = MSGSTATE_SENT;
    storeJournal_.journalMessage(dlvInfo_.getDlvId(),regionId_,m,ml.serial);
}


void RegionalStorage::retryMessage( msgid_type   msgId,
                                    msgtime_type currentTime,
                                    // msgtime_type retryDelay,  // in how many seconds try again
                                    int          smppState )
{
    msgtime_type retryDelay = dlvInfo_.getCommonSettings().getRetryTime(dlvInfo_.getRetryPolicyName(),smppState);

    RelockMutexGuard mg(cacheMon_);
    MsgIter iter;
    if ( !messageHash_.Pop(msgId,iter) ) {
        // not found
        throw smsc::util::Exception("message R=%u/D=%u/M=%u is not found (retryMessage)",
                                    unsigned(regionId_),
                                    unsigned(dlvInfo_.getDlvId()),
                                    unsigned(msgId));
    }
    MessageList tokill;
    MsgLock ml(iter,this);
    Message& m = iter->msg;
    // fixing time left according
    m.timeLeft -= timediff_type(time_t(currentTime)-time_t(m.lastTime));
    if ( m.timeLeft > timediff_type(dlvInfo_.getMinRetryTime()) ) {
        // there is enough validity time to try the next time
        if ( m.timeLeft < timediff_type(retryDelay) ) retryDelay = m.timeLeft;
        m.timeLeft -= retryDelay;
        retryDelay += currentTime;
        resendQueue_.insert( std::make_pair(retryDelay,iter) );
        mg.Unlock();
        m.lastTime = retryDelay;
        m.state = MSGSTATE_RETRY;
        char fmtime[20];
        smsc_log_debug(log_,"put message R=%u/D=%u/M=%u into retry at %s",
                       unsigned(regionId_),
                       unsigned(dlvInfo_.getDlvId()),
                       unsigned(msgId), formatMsgTime(fmtime,retryDelay) );
        storeJournal_.journalMessage(dlvInfo_.getDlvId(),regionId_,m,ml.serial);
    } else {
        // not enough time to retry
        // moving element to a not-valid part of the list
        tokill.splice(tokill.begin(),messageList_,iter);
        mg.Unlock();
        m.lastTime = currentTime;
        m.state = MSGSTATE_EXPIRED;
        smsc_log_debug(log_,"message R=%u/D=%u/M=%u is expired, smpp=%u",
                       unsigned(regionId_),
                       unsigned(dlvInfo_.getDlvId()),
                       unsigned(msgId), smppState );
        storeJournal_.journalMessage(dlvInfo_.getDlvId(),regionId_,m,ml.serial);
        // destroy(m);
    }
}


void RegionalStorage::finalizeMessage( msgid_type   msgId,
                                       msgtime_type currentTime,
                                       uint8_t      state,
                                       int          smppState )
{
    RelockMutexGuard mg(cacheMon_);
    MsgIter iter;
    if ( !messageHash_.Pop(msgId,iter) ) {
        throw smsc::util::Exception("message R=%u/D=%u/M=%u is not found (finalizeMessage)",
                                    unsigned(regionId_),
                                    unsigned(dlvInfo_.getDlvId()),
                                    unsigned(msgId));
    }
    MessageList tokill;
    MsgLock ml(iter,this);
    tokill.splice(tokill.begin(),messageList_,iter);
    mg.Unlock();
    Message& m = iter->msg;
    m.lastTime = currentTime;
    m.state = state;
    smsc_log_debug(log_,"message R=%u/D=%u/M=%u is finalized, state=%u, smpp=%u",
                   unsigned(regionId_),
                   unsigned(dlvInfo_.getDlvId()),
                   unsigned(msgId),state,smppState);
    storeJournal_.journalMessage(dlvInfo_.getDlvId(),regionId_,m,ml.serial);
    // destroy(m);
}


void RegionalStorage::transferFinished( TransferTask* task )
{
    MutexGuard mg(cacheMon_);
    if ( task == transferTask_ ) {
        transferTask_ = 0;
    } else {
        task = 0;
    }
    if (task) {
        delete task;
    }
    cacheMon_.notify();
    smsc_log_debug(log_,"transfer has been finished");
}


void RegionalStorage::addNewMessages( msgtime_type currentTime,
                                      MessageList& listFrom,
                                      MsgIter      iter1,
                                      MsgIter      iter2 )
{
    smsc_log_debug(log_,"adding new messages to storage R=%u/D=%u",
                   unsigned(regionId_),
                   unsigned(dlvInfo_.getDlvId()) );
    for ( MsgIter i = iter1; i != iter2; ++i ) {
        Message& m = i->msg;
        // m.lastTime = currentTime;
        m.state = MSGSTATE_INPUT;
        smsc_log_debug(log_,"new input msg R=%u/D=%u/M=%u",
                       unsigned(regionId_),
                       unsigned(dlvInfo_.getDlvId()),
                       unsigned(m.msgId));
        regionid_type serial = 0;
        storeJournal_.journalMessage(dlvInfo_.getDlvId(),regionId_,m,serial);
        i->serial = serial;
    }
    MutexGuard mg(cacheMon_);
    for ( MsgIter i = iter1; i != iter2; ++i ) {
        newQueue_.Push(i);
    }
    messageList_.splice( messageList_.begin(), listFrom, iter1, iter2 );
    if ( log_->isDebugEnabled() ) {
        std::string s;
        s.reserve(300);
        unsigned count = 0;
        for ( MsgIter i = messageList_.begin(); i != messageList_.end(); ++i ) {
            if ( i == storingIter_ ) {
                s.append(" s");
            }
            char buf[10];
            sprintf(buf," %u",unsigned(i->msg.msgId));
            s.append(buf);
            ++count;
        }
        smsc_log_debug(log_,"messages (%u) in R=%u/D=%u: %s",
                       count,
                       unsigned(regionId_),
                       unsigned(dlvInfo_.getDlvId()),
                       s.c_str());
    }
}


size_t RegionalStorage::rollOver()
{
    RelockMutexGuard mg(cacheMon_);
    if ( storingIter_ != messageList_.end() ) {
        throw InfosmeException("logic error: rolling in R=%u/D=%u is already in progress",
                               unsigned(regionId_),
                               unsigned(dlvInfo_.getDlvId()));
    }
    storingIter_ = messageList_.begin();
    size_t written = 0;
    while ( true ) {
        MsgIter iter = storingIter_;
        if ( iter == messageList_.end() ) break;
        ++storingIter_;
        {
            MsgLock ml(iter,this);
            mg.Unlock();
            smsc_log_debug(log_,"rolling R=%u/D=%u/M=%u",
                           regionId_,dlvInfo_.getDlvId(),ulonglong(iter->msg.msgId));
            written += storeJournal_.journalMessage(dlvInfo_.getDlvId(),regionId_,iter->msg,ml.serial);
        }
        smsc_log_debug(log_,"FIXME: place the restriction on throughput here");
        mg.Lock();
    }
    return written;
}


void RegionalStorage::setRecordAtInit( Message& msg, regionid_type serial )
{
    smsc_log_debug(log_,"adding R=%u/D=%u/M=%llu state=%s serial=%u",
                   unsigned(regionId_),
                   unsigned(dlvInfo_.getDlvId()),
                   ulonglong(msg.msgId),
                   msgStateToString(MsgState(msg.state)),
                   serial );
    MsgIter* iter = messageHash_.GetPtr(msg.msgId);
    if (!iter) {
        // no such message
        if (msg.state<uint8_t(MSGSTATE_FINAL)) {
            // NOTE: we do not bind to glossary here
            // need to create message
            iter = &messageHash_.Insert(msg.msgId,
                                        messageList_.insert(messageList_.end(),MessageLocker()));
            (*iter)->serial = serial;
            (*iter)->msg = msg;
        }
    } else if (msg.state>=uint8_t(MSGSTATE_FINAL)) {
        // msg found and is final
        messageList_.erase(*iter);
        messageHash_.Delete(msg.msgId);
    } else {
        // non-final msg found
        Message& m = (*iter)->msg;
        assert(m.msgId==msg.msgId);
        m.state = msg.state;
        m.lastTime = msg.lastTime;
        m.timeLeft = msg.timeLeft;
        (*iter)->serial = serial;
    }
}


bool RegionalStorage::postInit()
{
    unsigned total = 0;
    for ( MsgIter i = messageList_.begin(); i != messageList_.end(); ++i ) {
        ++total;
        Message& m = i->msg;
        if (m.state == MSGSTATE_INPUT) {
            // need to be moved to newQueue
            messageHash_.Delete( m.msgId );
            newQueue_.Push(i);
        } else if (m.state == MSGSTATE_RETRY) {
            // need to be moved to resendQueue
            messageHash_.Delete( m.msgId );
            resendQueue_.insert( std::make_pair(m.lastTime,i));
        }
    }
    smsc_log_debug(log_,"D=%u/R=%u postInit: total/new/resend=%u/%u/%u",
                   total,unsigned(newQueue_.Count()),unsigned(resendQueue_.size()));
    return !messageList_.empty();
}


/*
void RegionalStorage::destroy( Message& msg )
{
    smsc_log_debug(log_,"dtor message R=%u/D=%u/M=%llu",
                   unsigned(regionId_),
                   unsigned(dlvInfo_.getDlvId()),
                   ulonglong(msg.msgId));
}
 */

}
}
