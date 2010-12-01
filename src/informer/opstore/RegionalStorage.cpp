#include <cassert>
#include "RegionalStorage.h"
#include "informer/io/UnlockMutexGuard.h"
#include "informer/io/UTF8.h"
#include "informer/io/FileReader.h"
#include "informer/io/DirListing.h"
#include "informer/data/CommonSettings.h"
#include "informer/data/DeliveryActivator.h"
#include "informer/data/UserInfo.h"
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


namespace {

const size_t LENSIZE = 2;

struct StopRollingGuard
{
    StopRollingGuard( RelockMutexGuard& rmg, unsigned& roll, bool unlockAfter = true ) :
    rmg_(rmg), roll_(roll) {
        ++roll_;
        if (unlockAfter) rmg_.Unlock();
    }
    ~StopRollingGuard() {
        if (!rmg_.isLocked()) rmg_.Lock();
        --roll_;
    }
private:
    RelockMutexGuard& rmg_;
    unsigned&         roll_;
};


class ResendReader : public FileReader::RecordReader
{
public:
    ResendReader( MessageList& msgList, volatile bool& stopFlag ) :
    msgList_(msgList), stopFlag_(stopFlag) {}

    virtual bool isStopping() {
        return stopFlag_;
    }

    virtual size_t recordLengthSize() const {
        return LENSIZE;
    }

    virtual size_t readRecordLength( size_t filePos, FromBuf& fb )
    {
        size_t rl(fb.get16());
        if (rl > 10000) {
            throw InfosmeException(EXC_BADFILE,"record at %llu has invalid len: %u",
                                   ulonglong(filePos), unsigned(rl));
        }
        return rl;
    }

    virtual bool readRecordData( size_t filePos, FromBuf& fb )
    {
        try {
            msgList_.push_back(MessageLocker());
            Message& msg = msgList_.rbegin()->msg;
            msg.state = MSGSTATE_RETRY;
            msg.msgId = fb.get64();
            msg.lastTime = fb.get32();
            msg.timeLeft = fb.get32();
            msg.subscriber = fb.get64();
            msg.userData = fb.getCString();
            const uint8_t readstate = fb.get8();
            if (readstate) {
                MessageText(fb.getCString()).swap(msg.text);
            } else {
                MessageText(0,fb.get32()).swap(msg.text);
            }

            if (fb.getPos() != fb.getLen()) {
                throw InfosmeException(EXC_BADFILE,"record at %llu has extra data",
                                       ulonglong(filePos));
            }
        } catch ( std::exception& e ) {
            msgList_.pop_back();
            throw;
        }
        return true;
    }

private:
    MessageList&   msgList_;
    volatile bool& stopFlag_;
};

}


RegionalStorage::RegionalStorage( DeliveryImpl&        dlv,
                                  regionid_type        regionId ) :
log_(smsc::logger::Logger::getInstance("regstore")),
storingIter_(messageList_.end()),
dlv_(&dlv),
inputTransferTask_(0),
resendTransferTask_(0),
regionId_(regionId),
stopRolling_(0),
newOrResend_(0),
nextResendFile_(findNextResendFile())
{
    smsc_log_debug(log_,"ctor @%p R=%u/D=%u",
                   this, unsigned(regionId),unsigned(getDlvId()));
}


RegionalStorage::~RegionalStorage()
{
    smsc_log_debug(log_,"dtor @%p R=%u/D=%u",
                   this, regionId_,getDlvId());
}


dlvid_type RegionalStorage::getDlvId() const
{
    return dlv_->getDlvId();
}


const DeliveryInfo& RegionalStorage::getDlvInfo() const
{
    return *dlv_->getDlvInfo();
}


DlvState RegionalStorage::getState() const {
    return dlv_->getState();
}


bool RegionalStorage::isFinished()
{
    MutexGuard mg(cacheMon_);
    return messageList_.empty() && !nextResendFile_;
}


unsigned RegionalStorage::getNextMessage( usectime_type usecTime,
                                          int weekTime, Message& msg )
{
    MsgIter iter;
    RelockMutexGuard mg(cacheMon_);
    const char* from;
    const DeliveryInfo& info = *dlv_->getDlvInfo();
    const dlvid_type dlvId = info.getDlvId();

    if ( dlv_->getState() != DLVSTATE_ACTIVE ) {
        smsc_log_debug(log_,"R=%u/D=%u is not active",regionId_,dlvId);
        return 6*tuPerSec;
    }

    // FIXME: optimize return this date as next time.
    if ( !info.checkActiveTime(weekTime) ) {
        smsc_log_debug(log_,"R=%u/D=%u not on active period weekTime=%u",
                       regionId_, dlvId, weekTime );
        return 5*tuPerSec;
    }

    /// check speed control
    usectime_type ret = dlv_->activityLog_.getUserInfo().isReadyAndConsumeQuant(usecTime);
    if (ret>0) {
        smsc_log_debug(log_,"R=%u/D=%u not ready by user limit, wait=%lluus",
                       regionId_, dlvId, ret);
        if (ret > 1*tuPerSec) ret = 1*tuPerSec;
        return unsigned(ret);
    }

    const msgtime_type currentTime(msgtime_type(usecTime/tuPerSec));

    bool uploadNextResend = false;
    do { // fake loop

        /// check if we need to request new messages
        if ( !inputTransferTask_ &&
             unsigned(newQueue_.Count()) <= getCS()->getInputMinQueueSize() ) {

            const bool mayDetachRegion = ( newQueue_.Count()==0 &&
                                           resendQueue_.empty() &&
                                           nextResendFile_==0 );
            try {
                smsc_log_debug(log_,"R=%u/D=%u wants to request input transfer as it has new=%u",
                               unsigned(regionId_),
                               dlvId,
                               newQueue_.Count());
                InputTransferTask* task = 
                    dlv_->source_->createInputTransferTask(*this,
                                                           getCS()->getInputTransferChunkSize());
                if (task) {
                    dlv_->source_->getDlvActivator().startInputTransfer(task);
                }
                inputTransferTask_ = task;
            } catch (std::exception& e ) {
                smsc_log_warn(log_,"R=%u/D=%u request input msgs, exc: %s",
                              unsigned(regionId_),
                              dlvId,
                              e.what());
            }
            if ( !inputTransferTask_ && mayDetachRegion ) {
                // detach from reg senders
                std::vector< regionid_type > regs;
                regs.push_back(regionId_);
                dlv_->source_->getDlvActivator().deliveryRegions(dlvId,regs,false);
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
                        v->first + getCS()->getResendMinTimeToUpload() > nextResendFile_) {
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
                currentTime + getCS()->getResendMinTimeToUpload() > nextResendFile_) {
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

        // message is not found, please try in a second
        return tuPerSec + 12;

    } while ( false );

    if (uploadNextResend && !resendTransferTask_) {
        try {
            ResendTransferTask* task = new ResendTransferTask(*this,true);
            dlv_->source_->getDlvActivator().startResendTransfer(task);
            resendTransferTask_ = task;
        } catch ( std::exception& e ) {
            smsc_log_warn(log_,"R=%u/D=%u resend in task, exc: %s",
                          regionId_, dlvId, e.what());
        }
    }

    MsgLock ml(iter,this);
    mg.Unlock();

    Message& m = iter->msg;
    m.lastTime = currentTime;
    if (!m.timeLeft) {
        /// this one is a new message, set its TTL initially
        m.timeLeft  = info.getValidityPeriod();
        if (m.timeLeft <= 0) m.timeLeft = getCS()->getValidityPeriodDefault();
    }
    const uint8_t prevState = m.state;
    m.state = MSGSTATE_PROCESS;
    msg = m;
    smsc_log_debug(log_,"taking message R=%u/D=%u/M=%llu from %s",
                   unsigned(regionId_),
                   dlvId,
                   ulonglong(m.msgId),from);
    if (prevState != m.state) {
        dlv_->storeJournal_.journalMessage(dlvId,regionId_,m,ml.serial);
        dlv_->activityLog_.incStats(m.state,1,prevState);
    }
    return 0;
}


void RegionalStorage::messageSent( msgid_type msgId,
                                   msgtime_type currentTime )
{
    RelockMutexGuard mg(cacheMon_);
    MsgIter* ptr = messageHash_.GetPtr(msgId);
    const DeliveryInfo& info = *dlv_->getDlvInfo();
    if (!ptr) {
        throw InfosmeException(EXC_NOTFOUND,"message R=%u/D=%u/M=%llu is not found (messageSent)",
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
    dlv_->storeJournal_.journalMessage(info.getDlvId(),regionId_,m,ml.serial);
    dlv_->activityLog_.incStats(m.state,1,prevState);
}


void RegionalStorage::retryMessage( msgid_type         msgId,
                                    const RetryPolicy& policy,
                                    msgtime_type       currentTime,
                                    int                smppState,
                                    unsigned           nchunks )
{
    const DeliveryInfo& info = *dlv_->getDlvInfo();
    const dlvid_type dlvId = info.getDlvId();

    RelockMutexGuard mg(cacheMon_);
    MsgIter iter;
    if ( !messageHash_.Pop(msgId,iter) ) {
        // not found
        throw InfosmeException(EXC_NOTFOUND,"message R=%u/D=%u/M=%llu is not found (retryMessage)",
                               unsigned(regionId_), dlvId, ulonglong(msgId));
    }

    timediff_type retryDelay = -1;
    if ( info.wantRetryOnFail() ) {
        retryDelay = info.getRetryInterval(iter->msg.retryCount);
        if (retryDelay!=-1) {
            timediff_type smscrd = policy.getRetryInterval( info.isTransactional(),
                                                            smppState,
                                                            iter->msg.retryCount );
            if (smscrd==-1) {
                retryDelay = smscrd;
            } else if (smscrd > retryDelay) {
                retryDelay = smscrd;
            }
        }
    }

    if (retryDelay == 0) {
        // immediate retry
        Message& m = iter->msg;
        // putting the message to the new queue
        assert( m.state == MSGSTATE_PROCESS );
        // m.state = MSGSTATE_PROCESS;
        newQueue_.PushFront(iter);
        mg.Unlock();
        smsc_log_debug(log_,"put message R=%u/D=%u/M=%llu into immediate retry",
                       regionId_, dlvId, ulonglong(msgId));
        return;
    } else if ( retryDelay == -1 ) {
        // permanent failure
        doFinalize(mg,iter,currentTime,MSGSTATE_FAILED,smppState,nchunks);
        return;
    }

    Message& m = iter->msg;

    // fixing time left.
    m.timeLeft -= timediff_type(time_t(currentTime)-time_t(m.lastTime));

    if ( m.retryCount < Message::maxRetryCount ) {
        ++m.retryCount;
    } else {
        // no more retries, failed
        m.timeLeft = 0;
    }

    if ( m.timeLeft > getCS()->getRetryMinTimeToLive() ) {
        // there is enough validity time to try the next time

        MsgLock ml(iter,this);
        if ( m.timeLeft < retryDelay ) retryDelay = m.timeLeft;
        m.timeLeft -= retryDelay;
        m.lastTime = currentTime + retryDelay;
        resendQueue_.insert( std::make_pair(m.lastTime,iter) );
        if ( !resendTransferTask_ &&
             resendQueue_.size() > getCS()->getResendQueueMaxSize() ) {
            const msgtime_type queueStartTime = std::min(currentTime,resendQueue_.begin()->first);
            msgtime_type queueLastChunk = resendQueue_.rbegin()->first;
            queueLastChunk -= queueLastChunk % getCS()->getResendUploadPeriod();
            if ( queueLastChunk > queueStartTime +
                 getCS()->getResendMinTimeToUpload() + getCS()->getResendUploadPeriod() ) {
                try {
                    ResendTransferTask* task = new ResendTransferTask(*this,false);
                    dlv_->source_->getDlvActivator().startResendTransfer(task);
                    resendTransferTask_ = task;
                } catch (std::exception& e) {
                    smsc_log_warn(log_,"R=%u/D=%u resend out task, exc: %s",
                                  regionId_, dlvId, e.what() );
                }
            }
        }

        mg.Unlock();
        const uint8_t prevState = m.state;
        m.state = MSGSTATE_RETRY;
        smsc_log_debug(log_,"put message R=%u/D=%u/M=%llu into retry at %llu",
                       regionId_, dlvId, ulonglong(msgId),
                       msgTimeToYmd(m.lastTime) );
        dlv_->storeJournal_.journalMessage(info.getDlvId(),regionId_,m,ml.serial);
        dlv_->activityLog_.incStats(m.state,1,prevState);
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
    const DeliveryInfo& info = *dlv_->getDlvInfo();
    RelockMutexGuard mg(cacheMon_);
    MsgIter iter;
    if ( !messageHash_.Pop(msgId,iter) ) {
        throw InfosmeException(EXC_NOTFOUND,"message R=%u/D=%u/M=%llu is not found (finalizeMessage)",
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
    const dlvid_type dlvId = dlv_->getDlvId();
    MessageList tokill;
    MsgLock ml(iter,this);
    tokill.splice(tokill.begin(),messageList_,iter);
    const bool checkFinal = messageList_.empty() && !nextResendFile_;
    mg.Unlock();
    Message& m = iter->msg;
    if (!nchunks) {
        const char* text = m.text.getText();
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
    dlv_->activityLog_.addRecord(currentTime,regionId_,m,smppState,prevState);
    /*
    if (dlv_->getDlvInfo()->wantFinalMsgRecords()) {
        dlv_->source_->getDlvActivator().getFinalLog()
            .addMsgRecord(currentTime,
                          dlvId,
                          dlv_->getUserInfo().getUserId(),
                          m,
                          smppState);
    }
     */
    dlv_->storeJournal_.journalMessage(dlvId,regionId_,m,ml.serial);
    if (checkFinal) dlv_->checkFinalize();
}


unsigned RegionalStorage::evaluateNchunks( const char*     outText,
                                           size_t          outLen,
                                           smsc::sms::SMS* sms ) const
{
    const DeliveryInfo& info = *dlv_->getDlvInfo();
    try {
        const char* out = outText;
        UTF8::BufType ucstext;
        const bool hasHighBit = smsc::util::hasHighBit(out,outLen);
        if (hasHighBit) {
            getCS()->getUTF8().convertToUcs2(out,outLen,ucstext);
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
        
        const unsigned chunkLen = getCS()->getSlicedMessageSize();
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
    }
    if ( resendTransferTask_ ) {
        resendTransferTask_->stop();
    }
    while ( inputTransferTask_ || resendTransferTask_ ) {
        cacheMon_.wait(100);
    }
    if ( finalizeAll ) {
        smsc_log_warn(log_,"R=%u/D=%u FIXME make all messages fail (state=cancel?)",
                      regionId_, dlv_->getDlvId() );
    }
}


size_t RegionalStorage::rollOver()
{
    const DeliveryInfo& info = *dlv_->getDlvInfo();
    const dlvid_type dlvId = info.getDlvId();
    RelockMutexGuard mg(cacheMon_);
    while (stopRolling_) {
        cacheMon_.wait(100);
    }

    if ( storingIter_ != messageList_.end() ) {
        throw InfosmeException(EXC_LOGICERROR,
                               "rolling in R=%u/D=%u is already in progress",
                               unsigned(regionId_), dlvId);
    }
    size_t written = 0;
    storingIter_ = messageList_.begin();
    while ( true ) {
        MsgIter iter = storingIter_;
        if ( iter == messageList_.end() ) break;
        ++storingIter_;
        {
            MsgLock ml(iter,this);
            mg.Unlock();
            written += dlv_->storeJournal_.journalMessage(info.getDlvId(),
                                                         regionId_,iter->msg,ml.serial);
        }
        if ( dlv_->source_->getDlvActivator().isStopping() ) {
            mg.Lock();
            storingIter_ = messageList_.end();
            break;
        }
        mg.Lock();
        // FIXME: optimize calculate delay based on throughput restriction
        cacheMon_.wait(30); 
    }
    return written;
}


void RegionalStorage::setRecordAtInit( Message& msg, regionid_type serial )
{
    smsc_log_debug(log_,"adding R=%u/D=%u/M=%llu state=%s txt=%d/'%s' serial=%u",
                   unsigned(regionId_),
                   unsigned(getDlvId()),
                   ulonglong(msg.msgId),
                   msgStateToString(MsgState(msg.state)),
                   msg.text.getTextId(),
                   msg.text.getText() ? msg.text.getText() : "",
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
            (*iter)->msg.swap(msg);
        }
    } else if (msg.state>=uint8_t(MSGSTATE_FINAL)) {
        // msg found and is final
        messageList_.erase(*iter);
        messageHash_.Delete(msg.msgId);
        iter = 0;
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
    /*
    if (iter) {
        Message& m = (*iter)->msg;
        smsc_log_debug(log_,"R=%u/D=%u/M=%llu after add: state=%s txt=%d/'%s'",
                       regionId_, getDlvId(), ulonglong(m.msgId),
                       msgStateToString(MsgState(m.state)),
                       m.text.getTextId(),
                       m.text.getText() ? m.text.getText() : "");
    }
     */
}


bool RegionalStorage::postInit()
{
    int sent = 0;
    // int retry = 0;
    int process = 0;
    smsc_log_debug(log_,"R=%u/D=%u postInit",regionId_,getDlvId());
    for ( MsgIter i = messageList_.begin(); i != messageList_.end(); ++i ) {
        Message& m = i->msg;
        // bind to glossary
        if ( !m.text.isUnique()) {
            dlv_->source_->getGlossary().fetchText(m.text);
        }
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
    // dlv_->activityLog_.incStats(MSGSTATE_RETRY,retry); taking from actlog
    dlv_->activityLog_.incStats(MSGSTATE_SENT,sent);
    dlv_->activityLog_.incStats(MSGSTATE_PROCESS,process);
    return !messageList_.empty();
}


void RegionalStorage::transferFinished( InputTransferTask* task )
{
    MutexGuard mg(cacheMon_);
    if ( task == inputTransferTask_ ) {
        inputTransferTask_ = 0;
    }
    cacheMon_.notify();
    smsc_log_debug(log_,"input transfer has been finished");
}


void RegionalStorage::transferFinished( ResendTransferTask* task )
{
    MutexGuard mg(cacheMon_);
    if ( task == resendTransferTask_ ) {
        resendTransferTask_ = 0;
    }
    cacheMon_.notify();
    smsc_log_debug(log_,"resend transfer has been finished");
}


void RegionalStorage::addNewMessages( msgtime_type currentTime,
                                      MessageList& listFrom,
                                      MsgIter      iter1,
                                      MsgIter      iter2 )
{
    const dlvid_type dlvId = dlv_->getDlvId();
    smsc_log_debug(log_,"adding new messages to storage R=%u/D=%u",
                   unsigned(regionId_), dlvId );
    RelockMutexGuard mg(cacheMon_);
    StopRollingGuard srg(mg,stopRolling_);
    for ( MsgIter i = iter1; i != iter2; ++i ) {
        Message& m = i->msg;
        m.lastTime = currentTime;
        m.timeLeft = 0;
        m.state = MSGSTATE_PROCESS;
        m.retryCount = 0;
        smsc_log_debug(log_,"new input msg R=%u/D=%u/M=%llu",
                       unsigned(regionId_), dlvId,
                       ulonglong(m.msgId));
        regionid_type serial = 0;
        dlv_->activityLog_.addRecord(currentTime,regionId_,m,0);
        dlv_->storeJournal_.journalMessage(dlvId,regionId_,m,serial);
        i->serial = serial;
    }
    mg.Lock();
    for ( MsgIter i = iter1; i != iter2; ++i ) {
        newQueue_.Push(i);
    }
    messageList_.splice( messageList_.begin(), listFrom, iter1, iter2 );
    cacheMon_.notify();
}


void RegionalStorage::resendIO( bool isInputDirection, volatile bool& stopFlag )
{
    /// this method is invoked from ResendTransferTask
    const dlvid_type dlvId = dlv_->getDlvId();
    smsc_log_debug(log_,"FIXME: R=%u/D=%u resend %s task started",
                   regionId_, dlvId, isInputDirection ? "in" : "out");

    if ( isInputDirection ) {

        // need to input
        if (!nextResendFile_) return;

        try {

            // reading just one file
            char fpath[100];
            const ulonglong ymdTime = msgTimeToYmd(nextResendFile_);
            makeResendFilePath(fpath,ymdTime);

            FileGuard fg;
            fg.ropen( (getCS()->getStorePath() + fpath).c_str() );

            // reading the whole file
            smsc::core::buffers::TmpBuf<char,8192> buf;
            MessageList msgList;
            ResendReader resendReader(msgList,stopFlag);
            FileReader fileReader(fg);
            const size_t total = fileReader.readRecords(buf,resendReader);
            fg.close();
            if (stopFlag) return;
            smsc_log_info(log_,"R=%u/D=%u resend file %llu has been read %u records",
                          regionId_, dlvId, ymdTime, unsigned(total));
            {
                // checking
                MutexGuard mg(cacheMon_);
                for ( MsgIter i = msgList.begin(); i != msgList.end(); ) {
                    // check that this element is not in the resendQueue
                    bool found = false;
                    std::pair< ResendQueue::iterator, ResendQueue::iterator > ab =
                        resendQueue_.equal_range( i->msg.lastTime );
                    for ( ResendQueue::iterator j = ab.first; j != ab.second; ++j ) {
                        if ( j->second->msg.msgId == i->msg.msgId ) {
                            found = true;
                            break;
                        }
                    }
                    if ( found ) {
                        i = msgList.erase(i);
                    } else {
                        ++i;
                    }
                }
            }
            {
                RelockMutexGuard mg(cacheMon_);
                StopRollingGuard srg(mg,stopRolling_);

                // storing into journal
                for ( MsgIter i = msgList.begin(); i != msgList.end(); ++i ) {
                    regionid_type serial = 0;
                    dlv_->storeJournal_.journalMessage(dlvId,regionId_,i->msg,serial);
                    i->serial = serial;
                    if (stopFlag) return;
                }

                mg.Lock();
                for ( MsgIter i = msgList.begin(); i != msgList.end(); ++i ) {
                    resendQueue_.insert( std::make_pair(i->msg.lastTime,i));
                }
                messageList_.splice(messageList_.begin(), msgList, msgList.begin(), msgList.end());

                // we have to delete the file and find the next resend file
                mg.Unlock();
                unlink((getCS()->getStorePath() + fpath).c_str());
                const msgtime_type nexttime = findNextResendFile();

                mg.Lock();
                nextResendFile_ = nexttime;
                cacheMon_.notify();
            }

        } catch ( std::exception& e ) {
            smsc_log_warn(log_,"R=%u/D=%u resend in task exc: %s",
                          regionId_, dlvId, e.what());
        }
        return;

    }

    // output
    throw InfosmeException(EXC_NOTIMPL,"R=%u/D=%u resend out is not implemented",
                           regionId_, dlvId );
}


void RegionalStorage::makeResendFilePath( char* fpath,
                                          msgtime_type nextTime )
{
    sprintf(makeDeliveryPath(fpath,getDlvId()),"resend/");
    if ( nextTime ) {
        const ulonglong date = msgTimeToYmd(nextTime);
        sprintf(fpath + strlen(fpath),"%llu.jnl",date);
    }
}


msgtime_type RegionalStorage::findNextResendFile()
{
    char buf[100];
    makeResendFilePath(buf,0);
    try {
        std::vector< std::string > list;
        makeDirListing( NoDotsNameFilter(),S_IFREG)
            .list( (getCS()->getStorePath() + buf).c_str(), list );
        std::sort(list.begin(), list.end());
        for ( std::vector<std::string>::iterator i = list.begin();
              i != list.end(); ++i ) {
            int shift = 0;
            ulonglong ymd;
            sscanf(i->c_str(),"%llu.jnl%n",&ymd,&shift);
            if (!shift) {
                smsc_log_debug(log_,"R=%u/D=%u file '%s' is not resend file",
                               regionId_, dlv_->getDlvId(),i->c_str());
                continue;
            }
            return ymdToMsgTime(ymd);
        }
    } catch ( std::exception& e ) {
        smsc_log_debug(log_,"R=%u/D=%u next resend file is not found",
                       regionId_, dlv_->getDlvId());
    }
    return 0;
}

}
}
