#include <cassert>
#include "RegionalStorage.h"
#include "informer/io/UnlockMutexGuard.h"
#include "informer/io/RelockMutexGuard.h"
#include "informer/data/CommonSettings.h"
#include "informer/data/MessageGlossary.h"
#include "DeliveryImpl.h"
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



RegionalStorage::RegionalStorage( DeliveryImpl&        dlv,
                                  regionid_type        regionId ) :
log_(dlv.log_),
storingIter_(messageList_.end()),
dlv_(dlv),
inputTransferTask_(0),
resendTransferTask_(0),
regionId_(regionId),
ref_(0),
newOrResend_(0),
nextResendFile_(0)
{
    smsc_log_debug(log_,"ctor R=%u/D=%u", unsigned(regionId),unsigned(getDlvId()));
}


RegionalStorage::~RegionalStorage()
{
    smsc_log_debug(log_,"dtor R=%u/D=%u",unsigned(regionId_),unsigned(getDlvId()));
    MutexGuard mg(cacheMon_);
    if (inputTransferTask_) { inputTransferTask_->stop(); }
    if (resendTransferTask_) { resendTransferTask_->stop(); }
    while (inputTransferTask_) {
        cacheMon_.wait(100);
    }
    while (resendTransferTask_) {
        cacheMon_.wait(100);
    }
}


dlvid_type RegionalStorage::getDlvId() const
{
    return dlv_.getDlvId();
}


const DeliveryInfo& RegionalStorage::getDlvInfo() const
{
    return dlv_.getDlvInfo();
}


/*
bool RegionalStorage::getMessage( msgid_type msgId, Message& msg )
{
    MutexGuard mg(cacheMon_);
    MsgIter* ptr = messageHash_.GetPtr(msgId);
    if (!ptr) {
        smsc_log_debug(log_,"Message R=%u/D=%u/M=%llu is not found (getMessage)",
                       unsigned(regionId_),
                       unsigned(dlv_.getDlvId()),
                       ulonglong(msgId));
        return false; 
    }
    msg = (*ptr)->msg;
    return true;
}
 */


bool RegionalStorage::getNextMessage( msgtime_type currentTime, Message& msg )
{
    MsgIter iter;
    RelockMutexGuard mg(cacheMon_);
    const char* from;
    const DeliveryInfo& info = dlv_.getDlvInfo();
    const CommonSettings& cs = info.getCS();
    const dlvid_type dlvId = info.getDlvId();

    if ( info.getState() != DLVSTATE_ACTIVE ) return false;

    bool uploadNextResend = false;
    do { // fake loop

        /// check if we need to request new messages
        if ( !inputTransferTask_ &&
             unsigned(newQueue_.Count()) <= cs.getMinInputQueueSize() ) {
            const unsigned newQueueSize = unsigned(newQueue_.Count());
            // const bool retries = hasRetries();
            try {
                smsc_log_debug(log_,"R=%u/D=%u wants to request input transfer as it has new=%u",
                               unsigned(regionId_),
                               dlvId,
                               newQueueSize);
                // mg.Lock();
                inputTransferTask_ = dlv_.source_->startInputTransfer(*this,
                                                                      cs.getInputUploadCount(),
                                                                      newQueue_.Count()==0 &&
                                                                      resendQueue_.empty() &&
                                                                      nextResendFile_==0);
            } catch (std::exception& e ) {
                smsc_log_warn(log_,"exception requesting input msgs in R=%u/D=%u: %s",
                              unsigned(regionId_),
                              dlvId,
                              e.what());
                // mg.Lock();
                // if (uploadTasks_>0) --uploadTasks_;
            }
        }

        if ( !resendQueue_.empty() ) {
            if (++newOrResend_ > 3 ) {
                newOrResend_ = 0;
                ResendQueue::iterator v = resendQueue_.begin();
                if ( nextResendFile_ && v->first >= nextResendFile_ ) {
                    // we cannot take from resend queue until we load from file
                    uploadNextResend = true;
                } else {
                    if (nextResendFile_ && 
                        v->first + cs.getMinTimeToUploadResendFile() > nextResendFile_) {
                        uploadNextResend = true;
                    }
                    if ( v->first <= currentTime ) {
                        // get from the resendQueue
                        iter = v->second;
                        from = "resendQueue";
                        messageHash_.Insert(iter->msg.msgId,iter);
                        resendQueue_.erase(resendQueue_.begin());
                        break;
                    }
                }
            }
        } else {
            // resend queue is empty
            if (nextResendFile_ &&
                currentTime + cs.getMinTimeToUploadResendFile() > nextResendFile_) {
                uploadNextResend = true;
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

    if (uploadNextResend && !resendTransferTask_) {
        smsc_log_debug(log_,"FIXME: start upload next resend task here");
    }

    MsgLock ml(iter,this);
    mg.Unlock();

    Message& m = iter->msg;
    m.lastTime = currentTime;
    m.timeLeft = info.getMessageValidityTime();
    const uint8_t prevState = m.state;
    m.state = MSGSTATE_TAKEN;
    msg = m;
    smsc_log_debug(log_,"taking message R=%u/D=%u/M=%llu from %s",
                   unsigned(regionId_),
                   dlvId,
                   ulonglong(m.msgId),from);
    dlv_.storeJournal_.journalMessage(dlvId,regionId_,m,ml.serial);
    dlv_.activityLog_.incStats(m.state,1,prevState);
    return true;
}


void RegionalStorage::messageSent( msgid_type msgId,
                                   msgtime_type currentTime )
{
    RelockMutexGuard mg(cacheMon_);
    MsgIter* ptr = messageHash_.GetPtr(msgId);
    const DeliveryInfo& info = dlv_.getDlvInfo();
    if (!ptr) {
        throw smsc::util::Exception("message R=%u/D=%u/M=%llu is not found (messageSent)",
                                    unsigned(regionId_),
                                    unsigned(info.getDlvId()),
                                    ulonglong(msgId));
    }
    MsgLock ml(*ptr,this);
    mg.Unlock();
    Message& m = (*ptr)->msg;
    m.lastTime = currentTime;
    const uint8_t prevState = m.state;
    m.state = MSGSTATE_SENT;
    dlv_.storeJournal_.journalMessage(info.getDlvId(),regionId_,m,ml.serial);
    dlv_.activityLog_.incStats(m.state,1,prevState);
}


void RegionalStorage::retryMessage( msgid_type   msgId,
                                    msgtime_type currentTime,
                                    int          smppState )
{
    const DeliveryInfo& info = dlv_.getDlvInfo();
    msgtime_type retryDelay = info.getCS().getRetryTime(info.getRetryPolicyName(),smppState);

    RelockMutexGuard mg(cacheMon_);
    MsgIter iter;
    if ( !messageHash_.Pop(msgId,iter) ) {
        // not found
        throw smsc::util::Exception("message R=%u/D=%u/M=%llu is not found (retryMessage)",
                                    unsigned(regionId_),
                                    unsigned(info.getDlvId()),
                                    ulonglong(msgId));
    }
    if (retryDelay <= 0) {
        // immediate retry
        Message& m = iter->msg;
        // putting the message to the new queue
        assert( m.state == MSGSTATE_PROCESS || m.state == MSGSTATE_TAKEN );
        m.state = MSGSTATE_PROCESS;
        newQueue_.PushFront(iter);
        mg.Unlock();
        smsc_log_debug(log_,"put message R=%u/D=%u/M=%llu into immediate retry",
                       unsigned(regionId_),
                       unsigned(info.getDlvId()),
                       ulonglong(msgId));
        return;
    }

    MessageList tokill; // must be prior the msglock!
    MsgLock ml(iter,this);
    Message& m = iter->msg;
    // fixing time left according
    m.timeLeft -= timediff_type(time_t(currentTime)-time_t(m.lastTime));
    if ( m.timeLeft > timediff_type(info.getMinRetryTime()) ) {
        // there is enough validity time to try the next time
        if ( m.timeLeft < timediff_type(retryDelay) ) retryDelay = m.timeLeft;
        m.timeLeft -= retryDelay;
        retryDelay += currentTime;
        resendQueue_.insert( std::make_pair(retryDelay,iter) );
        // FIXME: check if resendQueue become very big and it have
        // messages in the next+1 chunk, then start flush task.

        mg.Unlock();
        m.lastTime = retryDelay;
        const uint8_t prevState = m.state;
        m.state = MSGSTATE_RETRY;
        smsc_log_debug(log_,"put message R=%u/D=%u/M=%llu into retry at %llu",
                       unsigned(regionId_),
                       unsigned(info.getDlvId()),
                       ulonglong(msgId),
                       msgTimeToYmd(retryDelay) );
        dlv_.storeJournal_.journalMessage(info.getDlvId(),regionId_,m,ml.serial);
        dlv_.activityLog_.incStats(m.state,1,prevState);
    } else {
        // not enough time to retry
        // moving element to a not-valid part of the list
        tokill.splice(tokill.begin(),messageList_,iter);
        mg.Unlock();
        m.lastTime = currentTime;
        const uint8_t prevState = m.state;
        m.state = MSGSTATE_EXPIRED;
        smsc_log_debug(log_,"message R=%u/D=%u/M=%llu is expired, smpp=%u",
                       unsigned(regionId_),
                       unsigned(info.getDlvId()),
                       unsigned(msgId), smppState );
        dlv_.storeJournal_.journalMessage(info.getDlvId(),regionId_,m,ml.serial);
        dlv_.activityLog_.addRecord(currentTime,regionId_,m,smppState,prevState);
    }
}


void RegionalStorage::finalizeMessage( msgid_type   msgId,
                                       msgtime_type currentTime,
                                       uint8_t      state,
                                       int          smppState )
{
    const DeliveryInfo& info = dlv_.getDlvInfo();
    RelockMutexGuard mg(cacheMon_);
    MsgIter iter;
    if ( !messageHash_.Pop(msgId,iter) ) {
        throw smsc::util::Exception("message R=%u/D=%u/M=%llu is not found (finalizeMessage)",
                                    unsigned(regionId_),
                                    unsigned(info.getDlvId()),
                                    ulonglong(msgId));
    }
    MessageList tokill;
    MsgLock ml(iter,this);
    tokill.splice(tokill.begin(),messageList_,iter);
    mg.Unlock();
    Message& m = iter->msg;
    m.lastTime = currentTime;
    m.timeLeft = 0;
    const uint8_t prevState = m.state;
    m.state = state;
    smsc_log_debug(log_,"message R=%u/D=%u/M=%llu is finalized, state=%u, smpp=%u",
                   unsigned(regionId_),
                   unsigned(info.getDlvId()),
                   ulonglong(msgId),state,smppState);
    dlv_.activityLog_.addRecord(currentTime,regionId_,m,smppState,prevState);
    dlv_.storeJournal_.journalMessage(info.getDlvId(),regionId_,m,ml.serial);
    // destroy(m);
}


void RegionalStorage::stopTransfer( bool finalizeAll )
{
    MutexGuard mg(cacheMon_);
    if ( inputTransferTask_ ) {
        // FIXME: should we wait until it stop?
        inputTransferTask_->stop();
    }
    if ( finalizeAll ) {
        // FIXME: make all messages fail
    }
}


size_t RegionalStorage::rollOver()
{
    const DeliveryInfo& info = dlv_.getDlvInfo();
    RelockMutexGuard mg(cacheMon_);
    if ( storingIter_ != messageList_.end() ) {
        throw InfosmeException("logic error: rolling in R=%u/D=%u is already in progress",
                               unsigned(regionId_),
                               unsigned(info.getDlvId()));
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
            written += dlv_.storeJournal_.journalMessage(info.getDlvId(),
                                                         regionId_,iter->msg,ml.serial);
        }
        // smsc_log_debug(log_,"FIXME: place the restriction on throughput here");
        mg.Lock();
    }
    return written;
}


void RegionalStorage::setRecordAtInit( Message& msg, regionid_type serial )
{
    smsc_log_debug(log_,"adding R=%u/D=%u/M=%llu state=%s serial=%u",
                   unsigned(regionId_),
                   unsigned(getDlvId()),
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
    int sent = 0;
    int retry = 0;
    int process = 0;
    for ( MsgIter i = messageList_.begin(); i != messageList_.end(); ++i ) {
        Message& m = i->msg;
        // bind to glossary
        dlv_.source_->getGlossary().bindMessage(m.text);
        switch (m.state) {
        case MSGSTATE_INPUT:
            throw InfosmeException("logic error: input msg D=%u/M=%llu in opstore",
                                   getDlvId(),m.msgId);
        case MSGSTATE_PROCESS:
        case MSGSTATE_TAKEN:
            // need to be moved to newQueue
            // NOTE: taken messages are moved into new queue also.
            m.state = MSGSTATE_PROCESS;
            messageHash_.Delete( m.msgId );
            newQueue_.Push(i);
            ++process;
            break;
        case MSGSTATE_SENT:
            // message should be in messageHash_
            ++sent;
            break;
        case MSGSTATE_RETRY:
            messageHash_.Delete(m.msgId);
            resendQueue_.insert( std::make_pair(m.lastTime,i) );
            ++retry;
            break;
        default:
            throw InfosmeException("logic error: final state D=%u/M=%llu in opstore",
                                   getDlvId(), m.msgId );
        }
    }
    // syncing stats
    dlv_.activityLog_.incStats(MSGSTATE_RETRY,retry);
    dlv_.activityLog_.incStats(MSGSTATE_SENT,sent);
    dlv_.activityLog_.incStats(MSGSTATE_PROCESS,process);
    return !messageList_.empty();
}


void RegionalStorage::transferFinished( InputTransferTask* task )
{
    MutexGuard mg(cacheMon_);
    if ( task == inputTransferTask_ ) {
        inputTransferTask_ = 0;
    } else {
        task = 0;
    }
    if (task) {
        delete task;
    }
    cacheMon_.notify();
    smsc_log_debug(log_,"input transfer has been finished");
}


void RegionalStorage::transferFinished( ResendTransferTask* task )
{
    MutexGuard mg(cacheMon_);
    if ( task == resendTransferTask_ ) {
        resendTransferTask_ = 0;
    } else {
        task = 0;
    }
    if (task) {
        delete task;
    }
    cacheMon_.notify();
    smsc_log_debug(log_,"resend transfer has been finished");
}


void RegionalStorage::addNewMessages( msgtime_type currentTime,
                                      MessageList& listFrom,
                                      MsgIter      iter1,
                                      MsgIter      iter2 )
{
    const DeliveryInfo& info = dlv_.getDlvInfo();
    smsc_log_debug(log_,"adding new messages to storage R=%u/D=%u",
                   unsigned(regionId_),
                   unsigned(info.getDlvId()));
    for ( MsgIter i = iter1; i != iter2; ++i ) {
        Message& m = i->msg;
        // m.lastTime = currentTime;
        m.state = MSGSTATE_PROCESS;
        smsc_log_debug(log_,"new input msg R=%u/D=%u/M=%llu",
                       unsigned(regionId_),
                       unsigned(info.getDlvId()),
                       unsigned(m.msgId));
        regionid_type serial = 0;
        dlv_.activityLog_.addRecord(currentTime,regionId_,m,0);
        dlv_.storeJournal_.journalMessage(info.getDlvId(),regionId_,m,serial);
        i->serial = serial;
    }
    MutexGuard mg(cacheMon_);
    for ( MsgIter i = iter1; i != iter2; ++i ) {
        newQueue_.Push(i);
    }
    messageList_.splice( messageList_.begin(), listFrom, iter1, iter2 );
    /*
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
                       unsigned(info.getDlvId()),
                       s.c_str());
    }
     */
}


void RegionalStorage::resendIO( bool isInputDirection )
{
    smsc_log_debug(log_,"FIXME: R=%u/D=%u resend IO dir=%s",
                   regionId_, dlv_.getDlvInfo().getDlvId(),
                   isInputDirection ? "in" : "out");
}

}
}
