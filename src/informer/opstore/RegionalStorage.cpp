#include <cassert>
#include "RegionalStorage.h"
#include "informer/io/UnlockMutexGuard.h"
#include "informer/io/UTF8.h"
#include "informer/data/CommonSettings.h"
#include "informer/data/DeliveryActivator.h"
#include "informer/data/MessageGlossary.h"
#include "informer/data/RetryPolicy.h"
#include "DeliveryImpl.h"
#include "StoreJournal.h"
#include "util/smstext.h"
#include "sms/sms.h"

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
        // move storing iter to the first unlocked message
        while ( rs_->storingIter_ != l.end() &&
                rs_->storingIter_->serial == MessageLocker::lockedSerial ) {
            ++rs_->storingIter_;
        }
        if (iter != l.begin()) {
            // moving iter before begin
            l.splice(l.begin(),l,iter);
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
log_(smsc::logger::Logger::getInstance("regstore")),
storingIter_(messageList_.end()),
dlv_(dlv),
inputTransferTask_(0),
resendTransferTask_(0),
regionId_(regionId),
ref_(0),
newOrResend_(0),
nextResendFile_(0)
{
    smsc_log_debug(log_,"ctor @%p R=%u/D=%u",
                   this, unsigned(regionId),unsigned(getDlvId()));
}


RegionalStorage::~RegionalStorage()
{
    smsc_log_debug(log_,"dtor R=%u/D=%u",unsigned(regionId_),unsigned(getDlvId()));
    {
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
    smsc_log_debug(log_,"dtor @%p R=%u/D=%u done, list=%p",
                   this, unsigned(regionId_),unsigned(getDlvId()),&messageList_);
}


dlvid_type RegionalStorage::getDlvId() const
{
    return dlv_.getDlvId();
}


const DeliveryInfo& RegionalStorage::getDlvInfo() const
{
    return dlv_.getDlvInfo();
}


DlvState RegionalStorage::getState() const {
    return dlv_.getState();
}


bool RegionalStorage::isFinished()
{
    MutexGuard mg(cacheMon_);
    return messageList_.empty() && !nextResendFile_;
}


bool RegionalStorage::getNextMessage( msgtime_type currentTime, Message& msg )
{
    MsgIter iter;
    RelockMutexGuard mg(cacheMon_);
    const char* from;
    const DeliveryInfo& info = dlv_.getDlvInfo();
    const CommonSettings& cs = info.getCS();
    const dlvid_type dlvId = info.getDlvId();

    if ( dlv_.getState() != DLVSTATE_ACTIVE ) return false;

    bool uploadNextResend = false;
    do { // fake loop

        /// check if we need to request new messages
        if ( !inputTransferTask_ &&
             unsigned(newQueue_.Count()) <= cs.getMinInputQueueSize() ) {

            const bool mayDetachRegion = ( newQueue_.Count()==0 &&
                                           resendQueue_.empty() &&
                                           nextResendFile_==0 );
            try {
                smsc_log_debug(log_,"R=%u/D=%u wants to request input transfer as it has new=%u",
                               unsigned(regionId_),
                               dlvId,
                               newQueue_.Count());
                InputTransferTask* task = 
                    dlv_.source_->createInputTransferTask(*this,
                                                          cs.getInputUploadCount());
                if (task) {
                    dlv_.source_->getDlvActivator().startInputTransfer(task);
                }
                inputTransferTask_ = task;
            } catch (std::exception& e ) {
                smsc_log_warn(log_,"exception requesting input msgs in R=%u/D=%u: %s",
                              unsigned(regionId_),
                              dlvId,
                              e.what());
            }
            if ( !inputTransferTask_ && mayDetachRegion ) {
                // detach from reg senders
                std::vector< regionid_type > regs;
                regs.push_back(regionId_);
                dlv_.source_->getDlvActivator().deliveryRegions(dlvId,regs,false);
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
    if (!m.timeLeft) {
        /// this one is a new message, set its TTL initially
        m.timeLeft  = info.getValidityPeriod();
    }
    const uint8_t prevState = m.state;
    m.state = MSGSTATE_PROCESS;
    msg = m;
    smsc_log_debug(log_,"taking message R=%u/D=%u/M=%llu from %s",
                   unsigned(regionId_),
                   dlvId,
                   ulonglong(m.msgId),from);
    if (prevState != m.state) {
        dlv_.storeJournal_.journalMessage(dlvId,regionId_,m,ml.serial);
        dlv_.activityLog_.incStats(m.state,1,prevState);
    }
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


void RegionalStorage::retryMessage( msgid_type         msgId,
                                    const RetryPolicy& policy,
                                    msgtime_type       currentTime,
                                    int                smppState,
                                    unsigned           nchunks )
{
    const DeliveryInfo& info = dlv_.getDlvInfo();

    RelockMutexGuard mg(cacheMon_);
    MsgIter iter;
    if ( !messageHash_.Pop(msgId,iter) ) {
        // not found
        mg.Unlock();
        throw smsc::util::Exception("message R=%u/D=%u/M=%llu is not found (retryMessage)",
                                    unsigned(regionId_),
                                    unsigned(info.getDlvId()),
                                    ulonglong(msgId));
    }

    timediff_type retryDelay = policy.getRetryTime(info,smppState,iter->msg.retryCount);

    if (retryDelay == 0) {
        // immediate retry
        Message& m = iter->msg;
        // putting the message to the new queue
        assert( m.state == MSGSTATE_PROCESS );
        // m.state = MSGSTATE_PROCESS;
        newQueue_.PushFront(iter);
        mg.Unlock();
        smsc_log_debug(log_,"put message R=%u/D=%u/M=%llu into immediate retry",
                       unsigned(regionId_),
                       unsigned(info.getDlvId()),
                       ulonglong(msgId));
        return;
    } else if ( retryDelay == -1 ) {
        // permanent failure
        doFinalize(mg,iter,currentTime,MSGSTATE_FAILED,smppState,nchunks);
        return;
    }

    Message& m = iter->msg;

    // FIXME: incrementing retry count, should we check here?
    ++m.retryCount;

    // fixing time left.
    m.timeLeft -= timediff_type(time_t(currentTime)-time_t(m.lastTime));
    if ( m.timeLeft > policy.getMinRetryTime() ) {
        // there is enough validity time to try the next time

        MsgLock ml(iter,this);
        if ( m.timeLeft < retryDelay ) retryDelay = m.timeLeft;
        m.timeLeft -= retryDelay;
        m.lastTime = currentTime + retryDelay;
        resendQueue_.insert( std::make_pair(m.lastTime,iter) );
        // FIXME: check if resendQueue become very big and it have
        // messages in the next+1 chunk, then start flush task.

        mg.Unlock();
        const uint8_t prevState = m.state;
        m.state = MSGSTATE_RETRY;
        smsc_log_debug(log_,"put message R=%u/D=%u/M=%llu into retry at %llu",
                       unsigned(regionId_),
                       unsigned(info.getDlvId()),
                       ulonglong(msgId),
                       msgTimeToYmd(m.lastTime) );
        dlv_.storeJournal_.journalMessage(info.getDlvId(),regionId_,m,ml.serial);
        dlv_.activityLog_.incStats(m.state,1,prevState);
    } else {
        doFinalize(mg,iter,currentTime,MSGSTATE_EXPIRED,smppState,nchunks);
    }
}


void RegionalStorage::finalizeMessage( msgid_type   msgId,
                                       msgtime_type currentTime,
                                       uint8_t      state,
                                       int          smppState,
                                       unsigned     nchunks )
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
    doFinalize(mg,iter,currentTime,state,smppState,nchunks);
}


void RegionalStorage::doFinalize(RelockMutexGuard& mg,
                                 MsgIter           iter,
                                 msgtime_type      currentTime,
                                 uint8_t           state,
                                 int               smppState,
                                 unsigned          nchunks )
{
    const dlvid_type dlvId = dlv_.getDlvId();
    MessageList tokill;
    MsgLock ml(iter,this);
    tokill.splice(tokill.begin(),messageList_,iter);
    const bool checkFinal = messageList_.empty() && !nextResendFile_;
    mg.Unlock();
    Message& m = iter->msg;
    if (!nchunks) {
        const char* text = m.text->getText();
        nchunks = evaluateNchunks(text,strlen(text));
    }
    m.lastTime = currentTime;
    m.timeLeft = 0;
    m.retryCount = nchunks;
    const uint8_t prevState = m.state;
    m.state = state;
    smsc_log_debug(log_,"message R=%u/D=%u/M=%llu is finalized, state=%u, smpp=%u, nchunks=%u, checkFin=%d",
                   unsigned(regionId_),
                   unsigned(dlvId),
                   ulonglong(m.msgId),state,smppState,
                   nchunks,
                   checkFinal);
    dlv_.activityLog_.addRecord(currentTime,regionId_,m,smppState,prevState);
    dlv_.storeJournal_.journalMessage(dlvId,regionId_,m,ml.serial);
    if (checkFinal) dlv_.checkFinalize();
}


unsigned RegionalStorage::evaluateNchunks( const char*     outText,
                                           size_t          outLen,
                                           smsc::sms::SMS* sms ) const
{
    const DeliveryInfo& info = dlv_.getDlvInfo();
    try {
        const char* out = outText;
        UTF8::BufType ucstext;
        const bool hasHighBit = smsc::util::hasHighBit(out,outLen);
        if (hasHighBit) {
            info.getCS().getUTF8().convertToUcs2(out,outLen,ucstext);
            outLen = ucstext.GetPos();
            out = ucstext.get();
            if (sms) sms->setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING, DataCoding::UCS2);
        } else if (sms) {
            sms->setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING, DataCoding::LATIN1);
        }

        if ( outLen <= MAX_ALLOWED_MESSAGE_LENGTH && !info.useDataSm() ) {
            if (sms) {
                sms->setBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE, out, unsigned(outLen));
                sms->setIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH, unsigned(outLen));
            }
        } else if ( info.getDeliveryMode() != DLVMODE_SMS ) {
            // ussdpush*
            if ( outLen > MAX_ALLOWED_MESSAGE_LENGTH ) {
                smsc_log_warn(log_,"ussdpush: max allowed msg len reached: %u",unsigned(outLen));
                outLen = MAX_ALLOWED_MESSAGE_LENGTH;
            }
            if (sms) {
                sms->setBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE, out, unsigned(outLen));
                sms->setIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH, unsigned(outLen));
            }
        } else {
            if ( outLen > MAX_ALLOWED_PAYLOAD_LENGTH ) {
                outLen = MAX_ALLOWED_PAYLOAD_LENGTH;
            }
            if (sms) {
                sms->setBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, out, unsigned(outLen));
            }
        }
        
        const unsigned chunkLen = info.getCS().getMaxMessageChunkSize();
        unsigned nchunks;
        if ( chunkLen > 0 && outLen > chunkLen ) {
            nchunks = unsigned(outLen-1) / chunkLen + 1;
        } else {
            nchunks = 1;
        }
        return nchunks;
    } catch ( std::exception& e ) {
        throw InfosmeException(EXC_IOERROR,"bad msg body: '%s'",outText);
    }
    return 0;
}


void RegionalStorage::stopTransfer( bool finalizeAll )
{
    MutexGuard mg(cacheMon_);
    if ( inputTransferTask_ ) {
        inputTransferTask_->stop();
        smsc_log_warn(log_,"R=%u/D=%u FIXME should we wait until input transfer task stops?",
                      regionId_, dlv_.getDlvId() );
    }
    if ( finalizeAll ) {
        smsc_log_warn(log_,"R=%u/D=%u FIXME make all messages fail (state=cancel?)",
                      regionId_, dlv_.getDlvId() );
    }
}


size_t RegionalStorage::rollOver()
{
    const DeliveryInfo& info = dlv_.getDlvInfo();
    const dlvid_type dlvId = info.getDlvId();
    RelockMutexGuard mg(cacheMon_);
    if ( storingIter_ != messageList_.end() ) {
        throw InfosmeException(EXC_LOGICERROR,
                               "rolling in R=%u/D=%u is already in progress",
                               unsigned(regionId_), dlvId);
    }
    size_t written = 0;
    if ( nextResendFile_ ) {
        msgtime_type nrf;
        do {
            nrf = nextResendFile_;
            mg.Unlock();
            written += dlv_.storeJournal_.journalNextResend(dlvId,regionId_,nrf);
            mg.Lock();
        } while (nrf != nextResendFile_);
    }
    storingIter_ = messageList_.begin();
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
        if ( dlv_.source_->getDlvActivator().isStopping() ) {
            mg.Lock();
            storingIter_ = messageList_.end();
            break;
        }
        mg.Lock();
        // FIXME: calculate delay based on throughput restriction
        cacheMon_.wait(30); 
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
        m.lastTime = msg.lastTime;
        m.timeLeft = msg.timeLeft;
        m.retryCount = msg.retryCount;
        m.state = msg.state;
        (*iter)->serial = serial;
    }
}


bool RegionalStorage::postInit()
{
    int sent = 0;
    // int retry = 0;
    int process = 0;
    for ( MsgIter i = messageList_.begin(); i != messageList_.end(); ++i ) {
        Message& m = i->msg;
        // bind to glossary
        dlv_.source_->getGlossary().bindText(m.text);
        switch (m.state) {
        case MSGSTATE_INPUT:
            throw InfosmeException(EXC_LOGICERROR,
                                   "input msg D=%u/M=%llu in opstore",
                                   getDlvId(),m.msgId);
        case MSGSTATE_PROCESS:
            // need to be moved to newQueue
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
            // ++retry;
            break;
        default:
            throw InfosmeException(EXC_LOGICERROR,
                                   "final state D=%u/M=%llu in opstore",
                                   getDlvId(), m.msgId );
        }
    }
    // syncing stats
    // dlv_.activityLog_.incStats(MSGSTATE_RETRY,retry); taking from actlog
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
        m.lastTime = currentTime;
        m.timeLeft = 0;
        m.state = MSGSTATE_PROCESS;
        m.retryCount = 0;
        smsc_log_debug(log_,"new input msg R=%u/D=%u/M=%llu",
                       unsigned(regionId_),
                       unsigned(info.getDlvId()),
                       ulonglong(m.msgId));
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
}


void RegionalStorage::resendIO( bool isInputDirection )
{
    /// this method is invoked from ResendTransferTask
    smsc_log_debug(log_,"FIXME: R=%u/D=%u resend IO dir=%s",
                   regionId_, dlv_.getDlvInfo().getDlvId(),
                   isInputDirection ? "in" : "out");
}

}
}
