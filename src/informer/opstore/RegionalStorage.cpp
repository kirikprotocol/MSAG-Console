#include <cassert>
#include "RegionalStorage.h"
#include "informer/io/UnlockMutexGuard.h"
#include "informer/io/FileReader.h"
#include "informer/io/DirListing.h"
#include "informer/io/IOConverter.h"
#include "informer/data/CommonSettings.h"
#include "informer/data/Region.h"
#include "informer/data/DeliveryActivator.h"
#include "informer/data/UserInfo.h"
#include "informer/data/MessageGlossary.h"
#include "informer/data/RetryPolicy.h"
#include "informer/data/ActivityLog.h"
#include "DeliveryImpl.h"
#include "StoreJournal.h"
#include "system/status.h"

namespace eyeline {
namespace informer {

#ifdef MAGICTYPECHECK
struct RegionalStorage::MsgLock : public RegionalStorage::MsgList::ItemLock
{
    typedef MessageList container_type;

    MsgLock(MsgList& ml) : ItemLock(ml) {}

    bool lock( MsgIter iter ) {
        if (ItemLock::lock(iter)) {
            iter->ticheck();
            return true;
        }
        return false;
    }
    void unlock() {
        it_->ticheck();
        ItemLock::unlock();
    }
    bool pop( MsgIter iter, MessageList& holder ) {
        if (ItemLock::pop(iter,holder)) {
            iter->ticheck();
            return true;
        }
        return false;
    }
};
#endif

/*
struct RegionalStorage::MsgLock
{
    MsgLock( MsgIter it, RelockMutexGuard& mg,
             RegionalStorage* regstore,
             bool exclusive = false ) :
    rs(regstore), iter(it), rmg(mg),
    serial(exclusive ? MessageLocker::lockedDelete :
           MessageLocker::lockedSerial) {}

    inline ~MsgLock() throw() {
        if (!rs) { return; }
        if (serial >= MessageLocker::lockedSerial) { return; }
        // assert(rs->cacheMon_->isLocked());
        rmg.Lock();
        iter->serial = serial;
        --(iter->users);
        rs->getCnd(iter).Signal();
    }

    bool tryLock()
    {
        assert(rs &&
#ifdef CHECKCONTENTION
               rs->cacheMon_.isLocked() &&
#endif
               serial >= MessageLocker::lockedSerial);
        if (iter->serial == MessageLocker::lockedDelete) {
            return false;
        }
        ++iter->users;
        if (iter->serial == MessageLocker::lockedSerial) {
            smsc::core::synchronization::Condition& c = rs->getCnd(iter);
            while (iter->serial == MessageLocker::lockedSerial) {
                c.WaitOn(rs->cacheMon_);
            }
            if (iter->serial == MessageLocker::lockedDelete) {
                --iter->users;
                return false;
            }
        }
        {
            regionid_type s = iter->serial;
            iter->serial = serial;
            serial = s;
        }
        if (iter->serial == MessageLocker::lockedDelete) {
            // wait until number of users become 1
            if (iter->users>1) {
                smsc::core::synchronization::Condition& c = rs->getCnd(iter);
                while (iter->users>1) {
                    c.WaitOn(rs->cacheMon_);
                }
            }
        }
        {
            // move storing iterator
            MessageList& l = rs->messageList_;
            MsgIter& si = rs->storingIter_;
            while ( si != l.end() && si->serial >= MessageLocker::lockedSerial ) {
                ++si;
            }
            // move to the beginning of the list
            // if (iter != l.begin()) {
            // l.splice(l.begin(),l,iter);
            // }
        }
        return true;
    }


public:
    RegionalStorage*  rs;
    MsgIter           iter;
    RelockMutexGuard& rmg;
    regionid_type     serial; // used to be passed to journal
};
 */

namespace {
const size_t LENSIZE = 2;
}

/*
class RegionalStorage::StopRollingGuard
{
public:
    StopRollingGuard( RelockMutexGuard& rmg, RegionalStorage& rs, bool unlockAfter = true ) :
    rmg_(rmg), rs_(rs) {
        ++rs_.stopRolling_;
        smsc_log_debug(rs_.log_,"R=%u/D=%u rolling is forbidden, stop=%u",
                       rs_.getRegionId(), rs_.getDlvId(), rs_.stopRolling_);
        if (unlockAfter) rmg_.Unlock();
    }
    ~StopRollingGuard() {
        rmg_.Lock();
        if (!--rs_.stopRolling_) {
            smsc_log_debug(rs_.log_,"R=%u/D=%u rolling is allowed, stop=%u",
                           rs_.getRegionId(), rs_.getDlvId(), rs_.stopRolling_);
        }
    }

private:
    RelockMutexGuard& rmg_;
    RegionalStorage&  rs_;
};
 */


namespace {

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

    virtual size_t readRecordLength( size_t filePos, char* buf, size_t buflen )
    {
        assert(buflen>=LENSIZE);
        size_t rl( FromBuf(buf,buflen).get16() );
        if (rl > 10000) {
            throw InfosmeException(EXC_BADFILE,"record at %llu has invalid len: %u",
                                   ulonglong(filePos), unsigned(rl));
        }
        return rl;
    }

    virtual bool readRecordData( size_t filePos, char* buf, size_t buflen )
    {
        FromBuf fb(buf,buflen);
        try {
            msgList_.push_back(MessageLocker());
            Message& msg = msgList_.rbegin()->msg;
            const uint8_t stateVersion = fb.get8();
            const uint8_t version = stateVersion & 0x7f;
            msg.state = MSGSTATE_RETRY;
            msg.msgId = fb.get64();
            msg.lastTime = fb.get32();
            msg.timeLeft = fb.get32();
            if ( version > 2 ) {
                msg.retryCount = fb.get16();
            } else {
                msg.retryCount = 1;
            }
            msg.subscriber = fb.get64();
            msg.msgUserData = fb.getCString();
            if ( version >= 2 ) {
                MessageFlags(fb.getCString()).swap(msg.flags);
            } else if ( version != 1 ) {
                throw InfosmeException(EXC_BADFILE,"record at %llu has wrong version",
                                       ulonglong(filePos));
            }
            if (stateVersion & 0x80) {
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

smsc::logger::Logger* makeLogger( dlvid_type dlvId, regionid_type regionId )
{
    char buf[20];
    sprintf(buf,"r.%02u.%01u",dlvId % 100,regionId % 10);
    return smsc::logger::Logger::getInstance(buf);
}

}

RegionalStorage::RegionalStorage( DeliveryImpl&     dlv,
                                  const RegionPtr&  region,
                                  msgtime_type      nextResendTime ) :
log_(makeLogger(dlv.getDlvId(),region->getRegionId())),
cacheMon_( MTXWHEREAMI ),
messageList_(),
cancelling_(false),
dlv_(&dlv),
inputTransferTask_(0),
resendTransferTask_(0),
region_(region),
// stopRolling_(0),
newOrResend_(0),
inputRequestGrantTime_(0),
numberOfInputReqGrant_(0),
nextResendFile_(nextResendTime ? nextResendTime : dlv.findNextResendFile(region->getRegionId()))
{
    assert(region_.get());
    smsc_log_debug(log_,"ctor @%p R=%u/D=%u",
                   this, unsigned(getRegionId()),unsigned(getDlvId()));
}


RegionalStorage::~RegionalStorage()
{
    {
        smsc::core::synchronization::MutexGuard mg(cacheMon_ MTXWHEREPOST);
        // checking the list
        /*
        for ( MsgIter iter = messageList_.begin(); iter != messageList_.end(); ++iter ) {
            if ( iter->serial == MessageLocker::lockedSerial ) {
                smsc_log_error(log_,"R=%u/D=%u/M=%llu is locked in dtor",
                               getRegionId(), getDlvId(), iter->msg.msgId );
            }
        }
         */
    }
    smsc_log_debug(log_,"dtor @%p R=%u/D=%u",
                   this, getRegionId(), getDlvId());
}


dlvid_type RegionalStorage::getDlvId() const
{
    return dlv_->getDlvId();
}


const DeliveryInfo& RegionalStorage::getDlvInfo() const
{
    return dlv_->getDlvInfo();
}


DeliveryInfo& RegionalStorage::getDlvInfo()
{
    return dlv_->getDlvInfo();
}


DlvState RegionalStorage::getState() const {
    return dlv_->getState();
}


bool RegionalStorage::isFinished()
{
    smsc::core::synchronization::MutexGuard mg(cacheMon_ MTXWHEREPOST);
    smsc_log_debug( log_,"R=%u/D=%u list.empty=%d tasks=%d/%d next=%u",
                    getRegionId(), getDlvId(), messageList_.empty(),
                    inputTransferTask_ != 0,
                    resendTransferTask_ != 0,
                    nextResendFile_ );
    return messageList_.empty() && !nextResendFile_;
}


bool RegionalStorage::isEndDateReached( msgtime_type currentTime )
{
    msgtime_type activeTime = currentTime;
    const DeliveryInfo& info = getDlvInfo();
    if ( info.isBoundToLocalTime() ) {
        region_->getLocalWeekTime(currentTime, &activeTime);
    }
    const msgtime_type endDate = info.getEndDate();
    return ( endDate && endDate < activeTime );
}



int RegionalStorage::getNextMessage( usectime_type usecTime,
                                     int weekTime, Message& msg )
{
    MsgIter iter;
    RelockMutexGuard mg(cacheMon_ MTXWHEREPOST);
    // const char* from;
    const DeliveryInfo& info = dlv_->getDlvInfo();
    const dlvid_type dlvId = info.getDlvId();

    if ( dlv_->getState() != DLVSTATE_ACTIVE ) {
        smsc_log_debug(log_,"R=%u/D=%u is not active",getRegionId(),dlvId);
        return 6*tuPerSec;
    } else if ( cancelling_ ) {
        smsc_log_debug(log_,"R=%u/D=%u is cancelling",getRegionId(),dlvId);
        return 5*tuPerSec;
    }

    // how many seconds to wait until activeStart / activeEnd.
    int secondsReady = info.checkActiveTime(weekTime);
    if ( secondsReady > 0 ) {
        smsc_log_debug(log_,"R=%u/D=%u not on active period weekTime=%u (need to wait %u seconds)",
                       getRegionId(), dlvId, weekTime, secondsReady );
        if ( secondsReady > 100 ) { secondsReady = 100; }
        return secondsReady*tuPerSec;
    }

    /// check speed control
    {
        usectime_type ret = dlv_->dlvInfo_->getUserInfo().isReadyAndConsumeQuant(usecTime);
        if (ret>0) {
            // smsc_log_debug(log_,"R=%u/D=%u not ready by user limit, wait=%lluus",
            // getRegionId(), dlvId, ret);
            if (ret > 3*tuPerSec) ret = 3*tuPerSec;
            return int(ret);
        }
    }

    const msgtime_type currentTime(msgtime_type(usecTime/tuPerSec));
    /*
    {
        // NOTE: this check is moved into RegionSender.
        const msgtime_type ed = info.getEndDate();
        if ( ed != 0 && currentTime > ed ) {
            // the dlv should be stopped
            mg.Unlock();
            info.getUserInfo().getDA().startCancelThread(dlvId);
            dlv_->setState(DLVSTATE_PAUSED);
            return 10*tuPerSec;
        }
    }
     */

    msgtime_type uploadNextResend = 0;
    enum {
        FROMNONE = 0,
        FROMNEW = 1,
        FROMRESEND = 2
    } from = FROMNONE;
    do { // fake loop

        /// what is the period b/w messages [500..100M] microseconds
        usectime_type requestPeriod = tuPerSec / 2;
        if ( inputRequestGrantTime_ > 0 ) {
            const usectime_type rp = usecTime - inputRequestGrantTime_;
            if ( numberOfInputReqGrant_ > 0 &&
                 rp > getCS()->getInputRequestAverageTime() * tuPerSec ) {
                requestPeriod = rp / numberOfInputReqGrant_;
                if ( requestPeriod < 500LL ) { requestPeriod = 500LL; }
                else if ( requestPeriod > 100LL*tuPerSec ) { requestPeriod = 100LL*tuPerSec; }
                numberOfInputReqGrant_ = 0;
                inputRequestGrantTime_ = 0;
            }
        }

        const unsigned minQueueSize = 2 +
            unsigned(getCS()->getInputMinQueueTime()*tuPerSec / requestPeriod);

        /// check if we need to request new messages
        if ( !inputTransferTask_ &&
             unsigned(newQueue_.Count()) <= minQueueSize ) {

            const bool mayDetachRegion = ( newQueue_.Count()==0 &&
                                           resendQueue_.empty() &&
                                           nextResendFile_==0 &&
                                           !resendTransferTask_ );
            try {
                unsigned transferChunkSize =
                    unsigned(getCS()->getInputTransferChunkTime()*tuPerSec/requestPeriod) + 3;
                if ( transferChunkSize > 2000 ) { transferChunkSize = 2000; }
                smsc_log_debug(log_,"R=%u/D=%u to request input trans, newSz=%u, minSz=%u, reqInt=%llu, chunk=%llu",
                               unsigned(getRegionId()),
                               dlvId,
                               unsigned(newQueue_.Count()),
                               unsigned(minQueueSize),
                               ulonglong(requestPeriod),
                               ulonglong(transferChunkSize));
                InputTransferTask* task = 
                    dlv_->source_->createInputTransferTask(*this,
                                                           transferChunkSize );
                if (task) {
                    info.getUserInfo().getDA().startInputTransfer(task);
                }
                inputTransferTask_ = task;
            } catch (std::exception& e ) {
                smsc_log_warn(log_,"R=%u/D=%u request input msgs, exc: %s",
                              unsigned(getRegionId()),
                              dlvId,
                              e.what());
            }
            if ( !inputTransferTask_ && mayDetachRegion ) {
                // detach from reg senders
                std::vector< regionid_type > regs;
                regs.push_back(getRegionId());
                info.getUserInfo().getDA().deliveryRegions(dlvId,regs,false);
            }
        }

        if (nextResendFile_) {
            msgtime_type startTime = currentTime;
            if (!resendQueue_.empty()) {
                ResendQueue::iterator v = resendQueue_.begin();
                if (v->first < startTime) startTime = v->first;
            }
            if (startTime + getCS()->getResendMinTimeToUpload() >= nextResendFile_) {
                uploadNextResend = startTime;
            }
        }

        if ( !resendQueue_.empty() ) {
            if ( ++newOrResend_ > 3 || newQueue_.Count() == 0 ) {
                newOrResend_ = 0;
                ResendQueue::iterator v = resendQueue_.begin();
                if ( nextResendFile_ && v->first >= nextResendFile_ ) {
                    // cannot take from resend queue until we load it
                } else if ( v->first <= currentTime ) {
                    iter = v->second;
                    from = FROMRESEND;
                    resendQueue_.erase(resendQueue_.begin());
                    messageHash_.Insert(iter->msg.msgId,iter);
                    break;
                }
            }
        }

        /// checking newQueue
        if ( newQueue_.Pop(iter) ) {
            // success
            if ( !inputRequestGrantTime_ ) {
                inputRequestGrantTime_ = usecTime;
                numberOfInputReqGrant_ = 0;
            }
            ++numberOfInputReqGrant_;
            messageHash_.Insert(iter->msg.msgId,iter);
            from = FROMNEW;
            break;
        }

        from = FROMNONE;

    } while ( false );

    if (uploadNextResend && !resendTransferTask_) {
        try {
            smsc_log_debug(log_,"R=%u/D=%u wants resend-in as curTime=%u start=%+d next=%+d",
                           getRegionId(), dlvId,
                           currentTime, int(uploadNextResend-currentTime),
                           int(nextResendFile_-uploadNextResend) );
            ResendTransferTask* task = new ResendTransferTask(*this,true);
            info.getUserInfo().getDA().startResendTransfer(task);
            resendTransferTask_ = task;
        } catch ( std::exception& e ) {
            smsc_log_warn(log_,"R=%u/D=%u resend in task, exc: %s",
                          getRegionId(), dlvId, e.what());
        }
    }

    if ( from == FROMNONE ) {
        // message is not found, please try in a second
        smsc_log_debug(log_,"R=%u/D=%u fail: newSize=0, reSize=%u (first=%+d) nextFile=%+d",
                       getRegionId(), dlvId, unsigned(resendQueue_.size()),
                       resendQueue_.empty() ? 0U :
                       unsigned(resendQueue_.begin()->first - currentTime),
                       nextResendFile_ ? int(nextResendFile_ - currentTime) : -1);
        return int(tuPerSec + 12);
    }


    MsgLock ml(messageList_);
    const msgid_type msgId = iter->msg.msgId;
    if (!ml.lock(iter)) {
        smsc_log_error(log_,"R=%u/D=%u/M=%llu cannot be locked from getNext",
                       getRegionId(), dlvId, ulonglong(msgId) );
        return (tuPerSec/2+15);
    }
    Message& m = iter->msg;
    bool needsave = false;
    if ( from == FROMRESEND ) {
        const timediff_type uptonow = timediff_type(currentTime - m.lastTime);
        if ( uptonow > 0 && getCS()->isRecalcTTLRequested() ) {
            int oldWeekTime = region_->getLocalWeekTime( m.lastTime );
            const timediff_type actualTTL = 
                info.recalcTTL( oldWeekTime, m.timeLeft, uptonow);
            if ( actualTTL <= 0 ) {
                messageHash_.Delete(m.msgId);
                mg.Unlock();
                dlv_->dlvInfo_->getUserInfo().restoreQuant();
                smsc_log_debug(log_,"R=%u/D=%u/M=%llu retry msg is expired, weekTime=%d, ttl=%d, uptonow=%d -> actTTL=%d",
                               getRegionId(), dlvId, m.msgId,
                               oldWeekTime, m.timeLeft, uptonow, actualTTL );
                const int smppState = smsc::system::Status::DELIVERYTIMEDOUT;
                m.timeLeft = 0;
                doFinalize(ml,currentTime,MSGSTATE_EXPIRED,smppState,0);
                return 10;
            }
            m.timeLeft = actualTTL;
            needsave = true;
        }
    }
    mg.Unlock();
    if ( from == FROMNEW && !m.timeLeft ) {
        /// this one is a new message, set its TTL initially
        m.timeLeft  = info.getMessageTimeToLive();
        // if (m.timeLeft <= 0) m.timeLeft = getCS()->getValidityPeriodDefault();
        needsave = true;
    }
    m.lastTime = currentTime;
    const uint8_t prevState = m.state;
    m.state = MSGSTATE_PROCESS;
    msg = m;
    smsc_log_debug(log_,"taking message R=%u/D=%u/M=%llu from %s",
                   unsigned(getRegionId()),
                   dlvId,
                   ulonglong(msgId),
                   (from == FROMNEW )? "newQueue" : "resendQueue");
    if (prevState != m.state) {
        needsave = true;
    }
    if (needsave) {
        dlv_->storeJournal_->journalMessage(dlvId,getRegionId(),*iter);
        dlv_->dlvInfo_->incMsgStats(*region_,currentTime,m.state,1,prevState);
    }
    return secondsReady;
}


void RegionalStorage::messageSent( msgid_type msgId,
                                   msgtime_type currentTime,
                                   unsigned nchunks )
{
    const DeliveryInfo& info = dlv_->getDlvInfo();
    MsgLock ml(messageList_);
    MsgIter iter;
    {
        smsc::core::synchronization::MutexGuard mg(cacheMon_ MTXWHEREPOST);
        MsgIter* ptr = messageHash_.GetPtr(msgId);
        if (!ptr) {
            throw InfosmeException(EXC_NOTFOUND,"message R=%u/D=%u/M=%llu is not found (messageSent)",
                                   unsigned(getRegionId()),
                                   unsigned(info.getDlvId()),
                                   ulonglong(msgId));
        }
        iter = *ptr;
        if (!ml.lock(iter)) {
            smsc_log_error(log_,"R=%u/D=%u/M=%llu cannot be locked from msgSent",
                           unsigned(getRegionId()),
                           unsigned(info.getDlvId()),
                           ulonglong(msgId));
            return;
        }
    }
    Message& m = iter->msg;
    m.lastTime = currentTime;
    const uint8_t prevState = m.state;
    m.state = MSGSTATE_SENT;
    dlv_->activityLog_->addRecord(currentTime,*region_,m,0,nchunks,prevState);
    dlv_->storeJournal_->journalMessage(info.getDlvId(),getRegionId(),*iter);
}


void RegionalStorage::retryMessage( msgid_type         msgId,
                                    const RetryPolicy& policy,
                                    msgtime_type       currentTime,
                                    int                smppState,
                                    unsigned           nchunks,
                                    bool               fixTransactional )
{
    const DeliveryInfo& info = dlv_->getDlvInfo();
    const dlvid_type dlvId = info.getDlvId();

    RelockMutexGuard mg(cacheMon_ MTXWHEREPOST);
    MsgIter iter;
    if ( !messageHash_.Pop(msgId,iter) ) {
        // not found
        throw InfosmeException(EXC_NOTFOUND,"message R=%u/D=%u/M=%llu is not found (retryMessage)",
                               unsigned(getRegionId()), dlvId, ulonglong(msgId));
    }
    
    MsgLock ml(messageList_);
    if (!ml.lock(iter)) {
        mg.Unlock();
        smsc_log_error(log_,"R=%u/D=%u/M=%llu cannot be locked from retryMsg",
                       unsigned(getRegionId()),
                       unsigned(info.getDlvId()),
                       ulonglong(msgId));
        return;
    }


    timediff_type retryDelay = -1;
    {
        bool trans = false;
        if (!iter->msg.flags.hasTransactional(trans)) {
            trans = info.isTransactional();
        }
        timediff_type smscrd = policy.getRetryInterval(trans,
                                                       smppState,
                                                       iter->msg.retryCount);
        timediff_type ownRetryDelay = 0;
        if (smscrd==0 || smscrd==-1) {
            // take what smsc says
            retryDelay = smscrd;
        } else if ( info.wantRetryOnFail() ) {
            // retries are allowed
            retryDelay = ownRetryDelay = info.getRetryInterval(iter->msg.retryCount);
            if (retryDelay==-1) {
                // no retries
            } else if (smscrd > retryDelay) {
                retryDelay = smscrd;
            }
        }
        smsc_log_debug(log_,"R=%u/D=%u/M=%llu ownRetry=%d attempt=%d trans=%d smppState=%d smscrd=%d -> retryDelay=%d",
                       getRegionId(), dlvId, ulonglong(msgId),
                       ownRetryDelay, iter->msg.retryCount, trans,
                       smppState, smscrd, retryDelay );
    }

    Message& m = iter->msg;
    if (m.state >= MSGSTATE_FINAL) {
        throw InfosmeException(EXC_NOTFOUND,"msg R=%u/D=%u/M=%llu is already final in retryMessage",
                               unsigned(getRegionId()), dlvId, ulonglong(msgId));
    }

    // fixing time left.
    m.timeLeft -= timediff_type(time_t(currentTime)-time_t(m.lastTime));
    if (m.timeLeft < 0) {
        m.timeLeft = 0;
    }

    if ( retryDelay == -1 ) {
        // permanent failure
        mg.Unlock();
        doFinalize(ml,currentTime,MSGSTATE_FAILED,smppState,nchunks,fixTransactional);
        return;
    } else if (cancelling_) {
        mg.Unlock();
        smsc_log_warn(log_,"R=%u/D=%u/M=%llu is trying to retry=%d but cancelling",
                      getRegionId(),dlvId,ulonglong(msgId),
                      retryDelay );
        doFinalize(ml,currentTime,MSGSTATE_FAILED,smppState,nchunks,fixTransactional);
        return;
    } else if (isEndDateReached(currentTime)) {
        mg.Unlock();
        smsc_log_warn(log_,"R=%u/D=%u/M=%llu is trying to retry=%d but enddate reached",
                      getRegionId(),dlvId,ulonglong(msgId),
                      retryDelay );
        doFinalize(ml,currentTime,MSGSTATE_FAILED,smppState,nchunks,fixTransactional);
        return;
    }

    if (retryDelay == 0 && m.timeLeft>0) {
        // immediate retry
        // putting the message to the new queue
        // assert( m.state == MSGSTATE_PROCESS );
        uint8_t prevState = m.state;
        m.state = MSGSTATE_PROCESS;
        if ( newQueue_.Count() == 0 && resendQueue_.empty() ) {
            // signalling to bind
            std::vector< regionid_type > regs;
            regs.push_back(getRegionId());
            info.getUserInfo().getDA().deliveryRegions(dlvId,regs,true);
        }
        newQueue_.PushFront(iter);
        mg.Unlock();
        if (prevState != MSGSTATE_PROCESS) {
            dlv_->dlvInfo_->incMsgStats(*region_,currentTime,MSGSTATE_PROCESS,1,prevState);
        }
        smsc_log_debug(log_,"put message R=%u/D=%u/M=%llu into immediate retry",
                       getRegionId(), dlvId, ulonglong(msgId));
        return;
    }


    if ( (m.retryCount++ >= Message::maxRetryCount) ||
         (m.timeLeft <= getCS()->getRetryMinTimeToLive()) ) {

        mg.Unlock();
        doFinalize(ml,currentTime,MSGSTATE_EXPIRED,smppState,nchunks,fixTransactional);
        return;

    }

    // there is enough validity time to try the next time

    if (resendQueue_.empty() && newQueue_.Count() == 0) {
        // signalling to bind
        std::vector< regionid_type > regs;
        regs.push_back(getRegionId());
        info.getUserInfo().getDA().deliveryRegions(dlvId,regs,true);
    }

    /*
     MsgLock ml(iter,mg,this);
     if ( !ml.tryLock() ) {
     // cannot lock
     smsc_log_warn(log_,"R=%u/D=%u/M=%llu is final (retryMessage)",
     getRegionId(), dlvId, ulonglong(msgId) );
     return;
     }
     */

    if ( m.timeLeft < retryDelay + getCS()->getRetryMinTimeToLive() ) {
        retryDelay = m.timeLeft - getCS()->getRetryMinTimeToLive();
    }
    m.timeLeft -= retryDelay;
    m.lastTime = currentTime + retryDelay;
    resendQueue_.insert( std::make_pair(m.lastTime,iter) );
    if ( !resendTransferTask_ &&
         resendQueue_.size() > getCS()->getResendQueueMaxSize() ) {
        const msgtime_type queueStartTime = std::min(currentTime,resendQueue_.begin()->first);
        msgtime_type queueLastChunk = resendQueue_.rbegin()->first;
        queueLastChunk -= queueLastChunk % getCS()->getResendUploadPeriod();
        const msgtime_type limitTime = queueStartTime +
            getCS()->getResendMinTimeToUpload() + 
            getCS()->getResendUploadPeriod();
        if ( queueLastChunk > limitTime ) {
            try {
                smsc_log_debug(log_,"R=%u/D=%u wants resend-out as qSize/max=%u/%u start=%+d last/limit=+%d/%+d chunk=%u",
                               getRegionId(), dlvId,
                               unsigned(resendQueue_.size()),
                               unsigned(getCS()->getResendQueueMaxSize()),
                               int(queueStartTime-currentTime),
                               int(queueLastChunk-queueStartTime),
                               int(limitTime-queueStartTime),
                               unsigned(getCS()->getResendUploadPeriod()));
                ResendTransferTask* task = new ResendTransferTask(*this,false);
                info.getUserInfo().getDA().startResendTransfer(task);
                resendTransferTask_ = task;
            } catch (std::exception& e) {
                smsc_log_warn(log_,"R=%u/D=%u resend out task, exc: %s",
                              getRegionId(), dlvId, e.what() );
            }
        }
    }

    mg.Unlock();
    uint8_t prevState = m.state;
    m.state = MSGSTATE_RETRY;
    /*
     * fixTransactional should not be set here
    if (fixTransactional && prevState != MSGSTATE_SENT) {
        m.state = MSGSTATE_SENT;
        dlv_->activityLog_->addRecord(currentTime,*region_,m,smppState,MSGSTATE_SENT,1,prevState);
        prevState = MSGSTATE_SENT;
    }
     */
    smsc_log_debug(log_,"put message R=%u/D=%u/M=%llu into retry at %llu/%+u",
                   getRegionId(), dlvId, ulonglong(msgId),
                   msgTimeToYmd(m.lastTime),
                   int(m.lastTime - currentTime) );
    dlv_->activityLog_->addRecord(currentTime,*region_,m,smppState,
                                  nchunks, prevState);
    dlv_->storeJournal_->journalMessage(info.getDlvId(),getRegionId(),*iter);
}


void RegionalStorage::finalizeMessage( msgid_type   msgId,
                                       msgtime_type currentTime,
                                       uint8_t      state,
                                       int          smppState,
                                       unsigned     nchunks,
                                       bool         fixTransactional )
{
    const DeliveryInfo& info = dlv_->getDlvInfo();
    MsgLock ml(messageList_);
    {
        smsc::core::synchronization::MutexGuard mg(cacheMon_ MTXWHEREPOST);
        MsgIter iter;
        if ( !messageHash_.Pop(msgId,iter) ) {
            throw InfosmeException(EXC_NOTFOUND,"message R=%u/D=%u/M=%llu is not found (finalizeMessage)",
                                   unsigned(getRegionId()),
                                   unsigned(info.getDlvId()),
                                   ulonglong(msgId));
        }
        if (!ml.lock(iter)) {
            smsc_log_error(log_,"R=%u/D=%u/M=%llu cannot be locked from finMsg",
                           unsigned(getRegionId()),
                           unsigned(info.getDlvId()),
                           ulonglong(msgId));
            return;
        }
    }
    doFinalize(ml,currentTime,state,smppState,nchunks,fixTransactional);
}


void RegionalStorage::doFinalize(MsgLock&          ml,
                                 msgtime_type      currentTime,
                                 uint8_t           state,
                                 int               smppState,
                                 unsigned          nchunks,
                                 bool              fixTransactional )
{
    const dlvid_type dlvId = dlv_->getDlvId();
    MsgIter iter = ml.getIter();
    const msgid_type msgId = iter->msg.msgId;
    MessageList tokill;
    if ( !ml.pop(iter,tokill) ) {
        // cannot lock iter
        smsc_log_error(log_,"R=%u/D=%u/M=%llu cannot be locked from finalize(st=%u,smpp=%d)",
                       unsigned(getRegionId()), unsigned(dlvId), ulonglong(msgId),
                       state, smppState );
        return;
    }
    const bool checkFinal = messageList_.empty();
    Message& m = iter->msg;
    m.lastTime = currentTime;
    uint8_t prevState = m.state;
    if ( fixTransactional && prevState != MSGSTATE_SENT ) {
        m.state = MSGSTATE_SENT;
        dlv_->activityLog_->addRecord(currentTime,*region_,m,0,nchunks,prevState);
        prevState = MSGSTATE_SENT;
    }
    m.state = state;
    smsc_log_debug(log_,"message R=%u/D=%u/M=%llu is finalized, state=%u, smpp=%u, ttl=%d, nchunks=%u, checkFin=%d",
                   unsigned(getRegionId()),
                   unsigned(dlvId),
                   ulonglong(m.msgId),state,smppState,
                   m.timeLeft,
                   nchunks,
                   checkFinal);
    dlv_->activityLog_->addRecord(currentTime,*region_,m,smppState,nchunks,prevState);
    dlv_->storeJournal_->journalMessage(dlvId,getRegionId(),*iter);
    if (checkFinal) dlv_->checkFinalize();
}


size_t RegionalStorage::rollOver( SpeedControl<usectime_type,tuPerSec>& speedControl )
{
    const DeliveryInfo& info = dlv_->getDlvInfo();
    const dlvid_type dlvId = info.getDlvId();
    size_t written = 0;

    /*
    RelockMutexGuard mg(cacheMon_);
    while (stopRolling_) {
        smsc_log_debug(log_,"stop rolling in effect");
        cacheMon_.wait(100);
    }

    if ( storingIter_ != messageList_.end() ) {
        throw InfosmeException(EXC_LOGICERROR,
                               "rolling in R=%u/D=%u is already in progress",
                               unsigned(getRegionId()), dlvId);
    }
    storingIter_ = messageList_.begin();
    while ( true ) {
        MsgIter iter = storingIter_;
        if ( iter == messageList_.end() ) break;
        usectime_type currentTime = currentTimeMicro() % flipTimePeriod;
        usectime_type sleepTime = speedControl.isReady(currentTime,maxSnailDelay);
        if ( sleepTime > 10000 ) {
            cacheMon_.wait(unsigned(sleepTime/1000));
        }

        do {
            ++storingIter_;
        } while ( storingIter_ != messageList_.end() && storingIter_->serial >= MessageLocker::lockedSerial );

        {
            MsgLock ml(iter,mg,this);
            if ( ml.tryLock() ) {
                mg.Unlock();
                // storing the item
                const size_t chunk =
                    dlv_->storeJournal_->journalMessage(info.getDlvId(),
                                                        getRegionId(),iter->msg,ml.serial);
                if ( chunk ) {
                    speedControl.consumeQuant(int(chunk));
                    written += chunk;
                }
            }
        }
        if ( getCS()->isStopping() ) {
            storingIter_ = messageList_.end();
            break;
        }
    }
     */
    for ( messageList_.startRolling();; ) {
        {
            MsgLock ml(messageList_);
            if (!messageList_.advanceRolling(ml) ) { break; }
            MsgIter iter = ml.getIter();
            const size_t chunk =
                dlv_->storeJournal_->journalMessage(dlvId,getRegionId(),*iter);
            if ( chunk ) {
                speedControl.consumeQuant(int(chunk));
                written += chunk;
            }
        }
        if ( getCS()->isStopping() ) {
            // messageList_.stopRolling();
            break;
        }
        usectime_type currentTime = currentTimeMicro() % flipTimePeriod;
        usectime_type sleepTime = speedControl.isReady(currentTime,maxSnailDelay);
        if ( sleepTime > 10000 ) {
            smsc::core::synchronization::MutexGuard mg(cacheMon_ MTXWHEREPOST);
            cacheMon_.wait(unsigned(sleepTime/1000));
        }
    }
    return written;
}


void RegionalStorage::setRecordAtInit( Message& msg, regionid_type serial )
{
    smsc_log_debug(log_,"adding R=%u/D=%u/M=%llu state=%s txt=%d/'%s' serial=%u",
                   unsigned(getRegionId()),
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
            // MsgLock ml(this);
            MessageList& ml(messageList_.getContainer());
            iter = &messageHash_.Insert(msg.msgId,
                                        ml.insert(ml.begin(),MessageLocker()));
            // (*iter)->numberOfLocks = 0;
            (*iter)->serial = serial;
            (*iter)->msg.swap(msg);
        }
    } else if (msg.state>=uint8_t(MSGSTATE_FINAL)) {
        // msg found and is final
        messageList_.getContainer().erase(*iter);
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
}


bool RegionalStorage::postInit()
{
    int sent = 0;
    int process = 0;
    smsc_log_debug(log_,"R=%u/D=%u postInit",getRegionId(),getDlvId());
    // RegionPtr regPtr;
    // dlv_->source_->getDlvActivator().getRegion(getRegionId(),regPtr);

    const msgtime_type currentTime = currentTimeSeconds();
    // static const msgtime_type daynight = 24*3600;
    RelockMutexGuard mg(cacheMon_ MTXWHEREPOST);
    MessageList& mlist(messageList_.getContainer());
    for ( MsgIter i = mlist.begin(); i != mlist.end(); ) {
        Message& m = i->msg;
        const msgid_type msgId = m.msgId;
        // bind to glossary
        if ( !m.text.isUnique()) {
            dlv_->dlvInfo_->getGlossary().fetchText(m.text);
        }
        if (!m.text.getText()) {
            smsc_log_error(log_,"R=%u/D=%u/M=%llu message text is null in postInit, erasing",
                           getRegionId(),getDlvId(),msgId);
            messageHash_.Delete(msgId);
            MsgIter iter = i;
            ++i;
            messageList_.getContainer().erase(iter);
            continue;
        }
        if ( region_.get() && m.state == MSGSTATE_SENT ) {
            const timediff_type uptonow = currentTime - m.lastTime;
            if ( m.timeLeft < uptonow ) {
                // we should check message expiration
                int weekTime = region_->getLocalWeekTime( m.lastTime );
                const timediff_type actualTTL =
                    dlv_->dlvInfo_->recalcTTL(weekTime, m.timeLeft, uptonow);
                if (actualTTL<=0) {
                    smsc_log_debug(log_,"R=%u/D=%u/M=%llu message is expired, weekTime=%d, ttl=%d, uptonow=%d -> actTTL=%d",
                                   getRegionId(),getDlvId(),msgId,
                                   weekTime, m.timeLeft, uptonow, actualTTL );
                    m.lastTime = currentTime;
                    m.timeLeft = 0;
                    m.state = 0; // override state to get correct stats
                    MsgLock ml(messageList_);
                    MsgIter iter = i;
                    ++i;
                    if (!ml.lock(iter)) {
                        smsc_log_error(log_,"R=%u/D=%u/M=%llu cannot lock in postInit",
                                       getRegionId(),getDlvId(),msgId);
                    }
                    const int smppState = smsc::system::Status::EXPIREDATSTART;
                    unsigned nchunks = 0; // unknown
                    messageHash_.Delete(msgId);
                    mg.Unlock();
                    doFinalize(ml,currentTime,MSGSTATE_EXPIRED,smppState,nchunks);
                    continue;
                }
            }
        }
        if (log_->isDebugEnabled()) {
            uint8_t ton, npi, len;
            const uint64_t addr = subscriberToAddress(m.subscriber,len,ton,npi);
            smsc_log_debug(log_,"R=%u/D=%u/M=%llu state=%s A=.%u.%u.%*.*llu txtId=%u/'%s' last=%+d ttl=%d",
                           getRegionId(), getDlvId(), ulonglong(msgId),
                           msgStateToString(MsgState(m.state)),
                           ton,npi,len,len,ulonglong(addr),
                           m.text.getTextId(),
                           m.text.getText() ? m.text.getText() : "",
                           int(currentTime - m.lastTime), int(m.timeLeft));
        }

        switch (m.state) {
        case MSGSTATE_INPUT:
            throw InfosmeException(EXC_LOGICERROR,
                                   "input msg D=%u/M=%llu in opstore",
                                   getDlvId(),msgId);
        case MSGSTATE_PROCESS:
            // need to be moved to newQueue
            messageHash_.Delete( msgId );
            newQueue_.Push(i);
            ++process;
            break;
        case MSGSTATE_SENT:
            // message should be in messageHash_
            ++sent;
            break;
        case MSGSTATE_RETRY:
            messageHash_.Delete(msgId);
            resendQueue_.insert( std::make_pair(m.lastTime,i) );
            // ++retry;
            break;
        default:
            throw InfosmeException(EXC_LOGICERROR,
                                   "final state D=%u/M=%llu in opstore",
                                   getDlvId(), msgId );
        }
        ++i;
    }
    // syncing stats
    dlv_->dlvInfo_->initMsgStats(getRegionId(),MSGSTATE_SENT,sent);
    dlv_->dlvInfo_->initMsgStats(getRegionId(),MSGSTATE_PROCESS,process);
    return ( !messageList_.empty() || nextResendFile_);
}


void RegionalStorage::cancelOperativeStorage()
{
    if (cancelling_) return;
    smsc_log_info(log_,"R=%u/D=%u FIXME: cancelling operative storage",
                  getRegionId(), getDlvId());
    try {
        {
            smsc::core::synchronization::MutexGuard mg(cacheMon_ MTXWHEREPOST);
            if (cancelling_) return;
            cancelling_ = true;
            // make sure that there is no transfer
            doStopTransfer();
            // cleanup the queues
            resendQueue_.clear();
            messageHash_.Empty();
            newQueue_.Clear();
            char fpath[100];
            dlv_->makeResendFilePath(fpath,getRegionId(),0);
            if ( nextResendFile_ ) {
                try {
                    FileGuard::rmdirs( (getCS()->getStorePath() + fpath).c_str() );
                } catch ( std::exception& e ) {
                    smsc_log_warn(log_,"R=%u/D=%u resend dir cancel exc: %s",
                                  getRegionId(), getDlvId(), e.what());
                }
            }
            nextResendFile_ = 0;
        }
        const msgtime_type currentTime = currentTimeSeconds();
        const int smppState = smsc::system::Status::CANCELFAIL;
        
        MsgLock ml(messageList_);
        while ( ml.lockAny() ) {
            doFinalize(ml,currentTime,MSGSTATE_FAILED,smppState,0);
        }
        {
            // post check
            smsc::core::synchronization::MutexGuard mg(cacheMon_ MTXWHEREPOST);
            if ( !resendQueue_.empty() || messageHash_.Count() || newQueue_.Count() ) {
                smsc_log_error(log_,"R=%u/D=%u has non-empty store after cancel: resend.empty=%d hash=%u new=%u",
                               getRegionId(), getDlvId(), resendQueue_.empty(),
                               unsigned(messageHash_.Count()), unsigned(newQueue_.Count()));
            }
        }
        dlv_->checkFinalize();
    } catch (std::exception& e) {
        smsc_log_warn(log_,"R=%u/D=%u cancel store exc: %s",getRegionId(), getDlvId(), e.what());
    }
    smsc::core::synchronization::MutexGuard mg(cacheMon_ MTXWHEREPOST);
    cancelling_ = false;
}


void RegionalStorage::doStopTransfer()
{
    if ( inputTransferTask_ ) {
        inputTransferTask_->stop();
    }
    if ( resendTransferTask_ ) {
        resendTransferTask_->stop();
    }
    while ( inputTransferTask_ || resendTransferTask_ ) {
        cacheMon_.wait(100);
    }
}


void RegionalStorage::transferFinished( InputTransferTask* task )
{
    smsc::core::synchronization::MutexGuard mg(cacheMon_ MTXWHEREPOST);
    if ( task == inputTransferTask_ ) {
        inputTransferTask_ = 0;
    }
    cacheMon_.notify();
    smsc_log_debug(log_,"input transfer has been finished");
}


void RegionalStorage::transferFinished( ResendTransferTask* task )
{
    smsc::core::synchronization::MutexGuard mg(cacheMon_ MTXWHEREPOST);
    if ( task == resendTransferTask_ ) {
        resendTransferTask_ = 0;
    }
    cacheMon_.notify();
    smsc_log_debug(log_,"resend transfer has been finished");
}


void RegionalStorage::addNewMessages( msgtime_type currentTime,
                                      MessageList& listFrom )
{
    const dlvid_type dlvId = dlv_->getDlvId();
    const regionid_type regionId = getRegionId();
    smsc_log_debug(log_,"adding new messages to storage R=%u/D=%u",
                   unsigned(regionId), dlvId );
    do {
        if (cancelling_) {
            smsc_log_warn(log_,"R=%u/D=%u cannot add new messages to cancelling storage",
                          unsigned(regionId), dlvId );
            break;
        }
        MsgList::StopRollLock srg(messageList_);
        MsgIter ib = listFrom.begin(), ie = listFrom.end();
        for ( MsgIter i = ib; i != ie; ++i ) {
            Message& m = i->msg;
            m.lastTime = currentTime;
            m.timeLeft = 0;
            m.state = MSGSTATE_PROCESS;
            m.retryCount = 0;
            smsc_log_debug(log_,"new input msg R=%u/D=%u/M=%llu ",
                           unsigned(regionId), dlvId,
                           ulonglong(m.msgId));
            regionid_type serial = MessageLocker::nullSerial;
            dlv_->activityLog_->addRecord(currentTime,*region_,m,0,0,MSGSTATE_INPUT);
            dlv_->storeJournal_->journalMessage(dlvId,regionId,*i);
            i->serial = serial;
        }
        RelockMutexGuard mg(cacheMon_ MTXWHEREPOST);
        if (cancelling_) {
            smsc_log_warn(log_,"R=%u/D=%u storage was cancelled while adding new messages",
                          unsigned(regionId),dlvId);
            break;
        }
        for ( MsgIter i = ib; i != ie; ++i ) {
            newQueue_.Push(i);
        }
        srg.insert( listFrom );
        // messageList_.splice( messageList_.begin(), listFrom, iter1, iter2 );
        cacheMon_.notify();
        return;
    } while (false);
    // finalizing all messages on cancellation
    const int smppState = smsc::system::Status::CANCELFAIL;
    for ( MsgIter i = listFrom.begin(), ie = listFrom.end(); i != ie; ++i ) {
        Message& m = i->msg;
        m.lastTime = currentTime;
        m.timeLeft = 0;
        // m.retryCount = 0;
        const uint8_t prevState = m.state;
        m.state = MSGSTATE_EXPIRED;
        smsc_log_debug(log_,"message R=%u/D=%u/M=%llu is cancelled, state=%u, smpp=%u",
                       unsigned(regionId),
                       unsigned(dlvId),
                       ulonglong(m.msgId),m.state,smppState );
        dlv_->activityLog_->addRecord(currentTime,*region_,m,smppState,0,prevState);
        dlv_->storeJournal_->journalMessage(dlvId,regionId,*i);
    }
}


void RegionalStorage::resendIO( bool isInputDirection, volatile bool& stopFlag )
{
    /// this method is invoked from ResendTransferTask
    const dlvid_type dlvId = dlv_->getDlvId();
    smsc_log_info(log_,"R=%u/D=%u resend-%s task started",
                  getRegionId(), dlvId, isInputDirection ? "in" : "out");

    if ( isInputDirection ) {

        try {

            // reading just one file
            char fpath[100];
            // need to input
            ulonglong ymdTime;
            {
                smsc::core::synchronization::MutexGuard mg(cacheMon_ MTXWHEREPOST);
                if (!nextResendFile_ || cancelling_) return;
                ymdTime = msgTimeToYmd(nextResendFile_);
                dlv_->makeResendFilePath(fpath,getRegionId(),ymdTime);
            }
            FileGuard fg;
            fg.ropen( (getCS()->getStorePath() + fpath).c_str() );

            // reading the whole file
            TmpBuf<char,8192> buf;
            MessageList msgList;
            ResendReader resendReader(msgList,stopFlag);
            FileReader fileReader(fg);
            const size_t total = fileReader.readRecords(buf,resendReader);
            fg.close();
            if (stopFlag) return;
            smsc_log_info(log_,"R=%u/D=%u resend-in: file %llu has been read %u records",
                          getRegionId(), dlvId, ymdTime, unsigned(total));
            {
                // checking
                smsc::core::synchronization::MutexGuard mg(cacheMon_ MTXWHEREPOST);
                if (cancelling_) return;
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
                        smsc_log_debug(log_,"R=%u/D=%u/M=%llu duplicate: found in queue",
                                       getRegionId(), dlvId, ulonglong(i->msg.msgId));
                        i = msgList.erase(i);
                    } else {
                        // NOTE: we have to insert into resendQueue here to make sure
                        // that we don't have duplicate entries in the file.
                        resendQueue_.insert( std::make_pair(i->msg.lastTime,i));
                        // smsc_log_debug(log_,"R=%u/D=%u/M=%llu resend-in added to resend queue",
                        // getRegionId(), dlvId, ulonglong(i->msg.msgId));
                        ++i;
                    }
                }
            }
            {
                MsgList::StopRollLock srg(messageList_);
                // RelockMutexGuard mg(cacheMon_);
                // NOTE: cacheMon_ is unlocked here!

                try {
                    // storing into journal
                    for ( MsgIter i = msgList.begin(); i != msgList.end(); ++i ) {
                        i->serial = MessageLocker::nullSerial;
                        // NOTE: we may avoid locking here, as i is not in msgList_
                        if ( ! i->msg.text.isUnique() ) {
                            dlv_->dlvInfo_->getGlossary().fetchText(i->msg.text);
                        }
                        if (!i->msg.text.getText()) {
                            throw InfosmeException(EXC_LOGICERROR,"D=%u/M=%llu message text is null",
                                                   dlvId,ulonglong(i->msg.msgId));
                        }
                        dlv_->storeJournal_->journalMessage(dlvId,getRegionId(),*i);
                        // i->serial = serial;
                        if (stopFlag) {
                            throw InfosmeException(EXC_EXPIRED,"stop flagged");
                        }
                    }
                } catch ( std::exception& e ) {
                    // failed, we have to clean up the resendQueue_
                    smsc_log_warn(log_,"R=%u/D=%u resend-in failed to journal: %s",
                                  getRegionId(), dlvId, e.what());
                    smsc::core::synchronization::MutexGuard mg(cacheMon_ MTXWHEREPOST);
                    for ( MsgIter i = msgList.begin(); i != msgList.end(); ++i ) {
                        std::pair< ResendQueue::iterator, ResendQueue::iterator > ab =
                            resendQueue_.equal_range( i->msg.lastTime );
                        bool found = false;
                        for ( ResendQueue::iterator j = ab.first; j != ab.second; ++j ) {
                            if ( j->second->msg.msgId == i->msg.msgId ) {
                                resendQueue_.erase(j);
                                found = true;
                                break;
                            }
                        }
                        if (!found) {
                            smsc_log_warn(log_,"R=%u/D=%u resend-in logic failure: M=%llu is not found",
                                          getRegionId(), dlvId, ulonglong(i->msg.msgId) );
                        }
                    }
                    if (stopFlag) { return; }
                    throw;
                }

                srg.insert(msgList);

                // we have to delete the file and find the next resend file
                try {
                    FileGuard::unlink((getCS()->getStorePath() + fpath).c_str());
                } catch ( ErrnoException& e ) {
                    smsc_log_warn(log_,"R=%u/D=%u exc: %s",
                                  getRegionId(),dlvId,e.what());
                }
                const msgtime_type nexttime = dlv_->findNextResendFile(getRegionId());
                smsc_log_debug(log_,"R=%u/D=%u resend-in set nextResend=%u",
                               getRegionId(), dlvId, nexttime);
                smsc::core::synchronization::MutexGuard mg(cacheMon_ MTXWHEREPOST);
                nextResendFile_ = nexttime;
                cacheMon_.notify();
            }

        } catch ( std::exception& e ) {
            smsc_log_warn(log_,"R=%u/D=%u resend-in exc: %s",
                          getRegionId(), dlvId, e.what());
        }
        smsc_log_info(log_,"R=%u/D=%u resend-in finished",
                      getRegionId(), dlvId );
        return;

    }

    // output
    if ( resendQueue_.empty() ) {
        smsc_log_warn(log_,"R=%u/D=%u resend-out: queue is empty",
                      getRegionId(), dlvId );
        return;
    }
    const msgtime_type period = getCS()->getResendUploadPeriod();
    const msgtime_type currentTime = currentTimeSeconds();

    TmpBuf<char,8192> buf;
    msgtime_type startTime;
    {
        smsc::core::synchronization::MutexGuard mg(cacheMon_ MTXWHEREPOST);
        if (cancelling_) return;
        startTime =
            std::min(currentTime, resendQueue_.begin()->first) +
            getCS()->getResendMinTimeToUpload() + period*2 - 1;
        startTime -= startTime % period;
    }
    smsc_log_debug(log_,"R=%u/D=%u resend-out curTime=%u startTime=%+d period=%u",
                   getRegionId(), dlvId, currentTime, int(startTime-currentTime), period);

    MsgList::StopRollLock srg(messageList_);

    while (true) {
        
        msgtime_type nextTime = startTime + period;
        MessageList msgList;
        unsigned count = 0;
        {
            smsc::core::synchronization::MutexGuard mg(cacheMon_ MTXWHEREPOST);
            if (cancelling_) return;
            ResendQueue::iterator prev = resendQueue_.lower_bound(startTime);
            if ( prev == resendQueue_.end() ) { break; }
            if (stopFlag) { break; }
            ResendQueue::iterator next = resendQueue_.lower_bound(nextTime);
            if ( prev == next ) {
                startTime = nextTime;
                continue;
            }
            
            MsgLock ml(messageList_);
            for ( ResendQueue::iterator i = prev; i != next; ++i ) {
                if (ml.pop(i->second,msgList)) {
                    // successfully locked
                    // msgList.splice(msgList.end(),messageList_,i->second);
                    ++count;
                }
            }
            resendQueue_.erase(prev,next);
        }

        char fpath[100];
        dlv_->makeResendFilePath(fpath,getRegionId(),msgTimeToYmd(startTime));
        smsc_log_info(log_,"R=%u/D=%u resend-out writing %u records in '%s'",
                      getRegionId(), dlvId, count, fpath );
        FileGuard fg;
        size_t oldFilePos = 0;
        try {

            // FIXME: optimize writing by big buffer
            fg.create( (getCS()->getStorePath() + fpath).c_str(),
                       0666, true );
            oldFilePos = fg.seek(0,SEEK_END);
            static const uint8_t version = 3;
            for ( MsgIter i = msgList.begin(); i != msgList.end(); ++i ) {
                Message& msg = i->msg;
                uint8_t stateVersion = version;
                if (msg.text.isUnique()) {
                    buf.setSize(200 + strlen(msg.text.getText()) + msg.msgUserData.size());
                    stateVersion |= 0x80;
                } else {
                    buf.setSize(200 + msg.msgUserData.size());
                }
                ToBuf tb(buf.get(),buf.getSize());
                tb.skip(LENSIZE);
                tb.set8(stateVersion);
                tb.set64(msg.msgId);
                tb.set32(msg.lastTime);
                tb.set32(msg.timeLeft);
                tb.set16(msg.retryCount);
                tb.set64(msg.subscriber);
                tb.setCString(msg.msgUserData.c_str());
                tb.setHexCString(msg.flags.buf(),msg.flags.bufsize());
                if (stateVersion & 0x80) {
                    tb.setCString(msg.text.getText());
                } else {
                    tb.set32(msg.text.getTextId());
                }
                const size_t buflen = tb.getPos();
                tb.setPos(0);
                tb.set16(uint16_t(buflen-LENSIZE));
                fg.write(buf.get(),buflen);
                smsc_log_debug(log_,"R=%u/D=%u/M=%llu resend-out written to file",
                               getRegionId(), dlvId, ulonglong(msg.msgId));
                // fg.fsync();
            }

        } catch ( std::exception& e ) {
            smsc_log_warn(log_,"R=%u/D=%u resend-out writing '%s' exc: %s",
                          getRegionId(), getDlvId(), fpath, e.what());
            // cleanup the file
            if ( oldFilePos == 0 ) {
                try {
                    FileGuard::unlink((getCS()->getStorePath() + fpath).c_str());
                } catch ( std::exception& ex ) {
                    smsc_log_warn(log_,"R=%u/D=%u resend-out cannot unlink '%s'",
                                  getRegionId(), getDlvId(), fpath );
                }
            } else {
                fg.truncate(oldFilePos);
            }

            smsc::core::synchronization::MutexGuard mg(cacheMon_ MTXWHEREPOST);
            if (cancelling_) return;
            // restoring resend queue from msgList
            for ( MsgIter i = msgList.begin(); i != msgList.end(); ++i ) {
                resendQueue_.insert(std::make_pair(i->msg.lastTime,i));
            }
            srg.insert(msgList);
            throw;
        }

        fg.close(); // to fsync
        smsc::core::synchronization::MutexGuard mg(cacheMon_ MTXWHEREPOST);
        if ( nextResendFile_ == 0 || nextResendFile_ > startTime ) {
            smsc_log_debug(log_,"R=%u/D=%u resend-out set nextResend=%u",
                           getRegionId(), dlvId, startTime);
            nextResendFile_ = startTime;
        }
        startTime = nextTime;
    }
    smsc_log_info(log_,"R=%u/D=%u resend-out finished", getRegionId(), dlvId);
}


/*
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
                               getRegionId(), dlv_->getDlvId(),i->c_str());
                continue;
            }
            smsc_log_debug(log_,"R=%u/D=%u next resend file is %llu",
                           getRegionId(),dlv_->getDlvId(),ymd);
            return ymdToMsgTime(ymd);
        }
    } catch ( std::exception& e ) {
        smsc_log_debug(log_,"R=%u/D=%u next resend file is not found",
                       getRegionId(), dlv_->getDlvId());
    }
    return 0;
}
 */

}
}
