#include "RegionSender.h"
#include "SmscSender.h"
#include "system/status.h"
#include "informer/data/CommonSettings.h"
#include "informer/data/DeliveryInfo.h"
#include "informer/data/DeliveryActivator.h"
#include "informer/data/UserInfo.h"
#include "informer/io/InfosmeException.h"

namespace {
using namespace eyeline::informer;
struct EqualById {
    inline EqualById(dlvid_type dlvId) : d(dlvId) {}
    inline bool operator () ( const RegionalStorage* ptr ) const {
        return (ptr->getDlvId() == d);
    }
    dlvid_type d;
};
}


namespace eyeline {
namespace informer {

namespace {
smsc::logger::Logger* makeLogger( const Region* r)
{
    if (!r) {
        throw InfosmeException(EXC_LOGICERROR,"NULL region passed");
    }
    char buf[15];
    sprintf(buf,"rs.%02u",r->getRegionId() % 100);
    return smsc::logger::Logger::getInstance(buf);
}

}

RegionSender::RegionSender( const SmscSenderPtr& conn, const RegionPtr& r ) :
log_(makeLogger(r.get())),
reflock_( MTXWHEREAMI ),
ref_(0),
lock_( MTXWHEREAMI ),
conn_(),
region_(r),
taskList_(*this,2*maxScoreIncrement,log_),
speedControl_(std::max(region_->getBandwidth(),1U)),
speedLimiter_(std::max(region_->getBandwidth(),1U),
              // getCS()->getRegionSpeedLimitNBins(),
              // getCS()->getRegionSpeedLimitInterval(),
              getCS()->getRegionSpeedLimitSpeedup())
              // getCS()->getRegionSpeedLimitBaseSpeed())
{
    if (!conn) {
        throw InfosmeException(EXC_LOGICERROR,"conn is null");
    }
    smsc_log_debug(log_,"ctor S='%s' R=%u @%p",conn->getSmscId().c_str(),unsigned(r->getRegionId()),this);
    assignSender(conn);
}


void RegionSender::assignSender( const SmscSenderPtr& conn )
{
    SmscSenderPtr oldconn, newconn;
    {
        smsc::core::synchronization::MutexGuard mg(lock_);
        oldconn = conn_;
        // reset speed control
        speedControl_.setSpeed(std::max(region_->getBandwidth(),1U),
                               currentTimeMicro() % flipTimePeriod );
        speedLimiter_.setSpeed(std::max(region_->getBandwidth(),1U));
        newconn = conn_ = conn;
    }
    if ( oldconn != conn ) {
        if (oldconn.get()) { oldconn->detachRegionSender(*this); }
        if (newconn.get()) { newconn->attachRegionSender(*this); }
    } else if (oldconn.get()) {
        oldconn->updateBandwidth();
    }
}


std::string RegionSender::toString() const
{
    char buf[30];
    sprintf(buf,"R=%u bw=%u",getRegionId(),getBandwidth());
    return buf;
}


void RegionSender::resetSpeedControl( usectime_type currentTime )
{
    speedControl_.suspend(currentTime);
}


unsigned RegionSender::processRegion( usectime_type currentTime )
{
    try {
        // smsc_log_debug(log_,"R=%u processing at %llu",getRegionId(),currentTime);
        currentTime_ = currentTime;
        const msgtime_type now = msgtime_type(currentTime_ / tuPerSec);

        /*
        struct tm tmnow;
        {
            const time_t tmp = time_t(now);
            if ( !gmtime_r(&tmp,&tmnow) ) {
                throw InfosmeException(EXC_SYSTEM,"R=%u gmtime_r()",getRegionId());
            }
        }
        static const int daynight = 24*3600;
        static const int aweek = 7*daynight;
        // monday is 0..daynight-1, tue is daynight..daynight*2-1, etc.
        weekTime_ = int( ((tmnow.tm_wday+6)*daynight +
                          (now % daynight) + 
                          region_->getTimezone()) % aweek );
         */

        smsc::core::synchronization::MutexGuard mg(lock_);
        weekTime_ = region_->getLocalWeekTime(now,&localTime_);

        // check speed control
        {
            const usectime_type delay = 
                speedControl_.isReady( currentTime % flipTimePeriod,
                                       maxSnailDelay );
            if ( delay > 0 ) {
                return unsigned(delay);
            }
        }
        {
            const usectime_type delay = speedLimiter_.isReady( currentTime );
            if ( delay > 0 ) return unsigned(delay);
        }
        
        nchunks_ = 0;
        // smsc_log_debug(log_,"R=%u processing weekTime=%u curTime=%llu locTime=%+d",
        // getRegionId(),weekTime_,
        // msgTimeToYmd(msgtime_type(currentTime_/tuPerSec)),
        // int(localTime_ - msgtime_type(currentTime_/tuPerSec)));
        const unsigned toSleep = taskList_.processOnce(0/*not used*/,tuPerSec);
        if (toSleep>0) {
            // smsc_log_debug(log_,"R=%u deliveries are not ready, sleep=%u",
            // getRegionId(),toSleep);
            speedControl_.suspend((currentTime + toSleep) % flipTimePeriod);
            return toSleep;
        } else {
            // smsc_log_debug(log_,"R=%u delivery processed",getRegionId());
            speedControl_.consumeQuant(nchunks_);
            speedLimiter_.consumeQuant(currentTime,nchunks_);
            return 0;
        }
    } catch ( std::exception& e ) {
        smsc::core::synchronization::MutexGuard mg(lock_);
        smsc_log_debug(log_,"R=%u send exc: %s",getRegionId(),e.what());
        speedControl_.suspend( (currentTime + tuPerSec) % flipTimePeriod);
    }
    // wait one second
    return unsigned(tuPerSec);
}


void RegionSender::addDelivery( RegionalStorage& ptr )
{
    const dlvid_type dlvId = ptr.getDlvId();
    smsc_log_debug(log_,"add delivery D=%u",dlvId);
    SmscSenderPtr conn;
    {
        smsc::core::synchronization::MutexGuard mg(lock_);
        RegionalStoragePtr* iter = dlvList_.GetPtr(dlvId);
        if (iter) {
            // already added
            return;
        }
        dlvList_.Insert(dlvId,RegionalStoragePtr(&ptr));
        taskList_.add(&ptr);
        usectime_type currentTime = (currentTimeMicro() + 1000) % flipTimePeriod;
        if ( speedControl_.getNextTime() > currentTime ) {
            speedControl_.suspend( currentTime );
        }
        conn = conn_;
    }
    if (conn.get()) { conn->wakeUp(); }
}


void RegionSender::removeDelivery( dlvid_type dlvId )
{
    smsc_log_debug(log_,"remove delivery D=%u",dlvId);
    {
        RegionalStoragePtr iter;
        smsc::core::synchronization::MutexGuard mg(lock_);
        if (!dlvList_.Pop(dlvId,iter)) {
            // not found
            return;
        }
        taskList_.remove(EqualById(dlvId));
    }
}


unsigned RegionSender::scoredObjIsReady( unsigned unused, ScoredPtrType ptr )
{
    static const unsigned sleepTimeException = unsigned(4*tuPerSec);
    if (!ptr) return sleepTimeException;
    try {
        if ( ptr->getState() == DLVSTATE_ACTIVE ) {
            // delivery is active
            const DeliveryInfo& info = ptr->getDlvInfo();
            const msgtime_type activeTime = 
                info.isBoundToLocalTime() ? localTime_ :
                msgtime_type(currentTime_ / tuPerSec);
            // compare start/end date
            // smsc_log_debug(log_,"R=%u/D=%u actTime=%llu startDate=%llu endDate=%llu regTime=%d",
            // getRegionId(), info.getDlvId(),
            // msgTimeToYmd(activeTime),
            // msgTimeToYmd(info.getStartDate()),
            // msgTimeToYmd(info.getEndDate()),
            // info.isBoundToLocalTime());
            if ( info.getStartDate() && activeTime < info.getStartDate() ) {
                // too early
                // smsc_log_debug(log_,"R=%u/D=%u early than startDate=%llu actTime=%llu",
                // getRegionId(), info.getDlvId(),
                // msgTimeToYmd(info.getStartDate()),
                // msgTimeToYmd(activeTime));
                return unsigned(std::min(info.getStartDate()-activeTime,4U)*tuPerSec);
            } else if ( info.getEndDate() && activeTime > info.getEndDate() ) {
                // too late, request detach
                // smsc_log_debug(log_,"R=%u/D=%u later than endDate=%llu actTime=%llu",
                // getRegionId(), info.getDlvId(),
                // msgTimeToYmd(info.getEndDate()),
                // msgTimeToYmd(activeTime));
                std::vector<regionid_type> regs(1,getRegionId());
                DeliveryActivator& da = info.getUserInfo().getDA();
                da.startCancelThread(info.getDlvId(),getRegionId());
                da.deliveryRegions(info.getDlvId(),regs,false);
                return sleepTimeException;
            }

            const int sleepTimeNotReady = ptr->getNextMessage(currentTime_,
                                                              weekTime_,
                                                              msg_);
            if (sleepTimeNotReady <= 0) {
                // smsc_log_debug(log_,"R=%u/D=%u/M=%llu is ready to be sent",
                // unsigned(getRegionId()),
                // unsigned(ptr->getDlvId()),
                // ulonglong(msg_.msgId));
                untilActiveEnd_ = -sleepTimeNotReady;
                return 0;
            } else {
                // smsc_log_debug(log_,"R=%u/D=%u: is not ready, going to sleep %u usec",
                // unsigned(getRegionId()),
                // unsigned(ptr->getDlvId()),
                // ulonglong(sleepTimeNotReady));
                return sleepTimeNotReady; // wait
            }
        } else {
            smsc_log_debug(log_,"R=%u/D=%u: is not active",
                           getRegionId(),
                           ptr->getDlvId());
        }
    } catch ( std::exception& e ) {
        smsc_log_warn(log_,"R=%u/D=%u isReady exc: %s",
                      unsigned(getRegionId()),
                      unsigned(ptr->getDlvId()), e.what());
    }
    return sleepTimeException;  // wait 5 seconds
}


int RegionSender::processScoredObj(unsigned, ScoredPtrType ptr, unsigned& objSleep)
{
    nchunks_ = 0;
    int res;
    const int inc = maxScoreIncrement / ptr->getDlvInfo().getPriority();
    assert(!ptr == false);
    // we may use conn_ here as lock_ is locked in processRegion()
    try {

        assert(conn_.get());
        res = conn_->send(*ptr, msg_, untilActiveEnd_, nchunks_);
        if ( res == smsc::system::Status::OK && nchunks_ > 0 ) {
            // message has been put into output queue
            smsc_log_debug(log_,"R=%u/D=%u/M=%llu sent nchunks=%d",
                           unsigned(getRegionId()),
                           unsigned(ptr->getDlvId()),
                           ulonglong(msg_.msgId), nchunks_);
            objSleep = 0;
            return inc * nchunks_;
        } else {
            smsc_log_warn(log_,"R=%u/D=%u/M=%llu send failed, res=%d, nchunks=%d",
                          unsigned(getRegionId()),
                          unsigned(ptr->getDlvId()),
                          ulonglong(msg_.msgId), res, nchunks_);
        }

    } catch ( std::exception& e ) {
        smsc_log_warn(log_,"R=%u/D=%u/M=%llu send failed, exc: %s",
                      unsigned(getRegionId()),
                      unsigned(ptr->getDlvId()),
                      ulonglong(msg_.msgId), e.what());
        res = smsc::system::Status::UNKNOWNERR;
    }
    ptr->retryMessage( msg_.msgId, conn_->getRetryPolicy(),
                       msgtime_type(currentTime_/tuPerSec),
                       res, nchunks_);
    objSleep = 0;
    return -inc;
}

}
}
