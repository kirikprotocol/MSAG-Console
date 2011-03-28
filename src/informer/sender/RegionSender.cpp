#include "RegionSender.h"
#include "SmscSender.h"
#include "system/status.h"
#include "informer/data/DeliveryInfo.h"
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
    sprintf(buf,"rs.%u",r->getRegionId());
    return smsc::logger::Logger::getInstance(buf);
}

}

RegionSender::RegionSender( SmscSender& conn, const RegionPtr& r ) :
log_(makeLogger(r.get())),
ref_(0),
conn_(0),
region_(r),
taskList_(*this,2*maxScoreIncrement,log_),
speedControl_(std::max(region_->getBandwidth(),1U))
{
    smsc_log_debug(log_,"ctor S='%s' R=%u",conn.getSmscId().c_str(),unsigned(r->getRegionId()));
    assignSender(&conn);
}


void RegionSender::assignSender( SmscSender* conn )
{
    // reset speed control
    speedControl_.setSpeed(std::max(region_->getBandwidth(),1U),
                           currentTimeMicro() % flipTimePeriod );
    if ( conn_ != conn ) {
        if (conn_) conn_->detachRegionSender(*this);
        conn_ = conn;
        if (conn_) conn_->attachRegionSender(*this);
    } else {
        conn_->updateBandwidth();
    }
}


std::string RegionSender::toString() const
{
    char buf[30];
    sprintf(buf,"R=%u bw=%u",getRegionId(),getBandwidth());
    return buf;
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
        weekTime_ = region_->getLocalWeekTime(now);

        MutexGuard mg(lock_);
        // check speed control
        const usectime_type delay = 
            speedControl_.isReady( currentTime % flipTimePeriod,
                                   maxSnailDelay );
        if ( delay > 0 ) {
            return unsigned(delay);
        }
        
        const unsigned toSleep = taskList_.processOnce(0/*not used*/,tuPerSec);
        if (toSleep>0) {
            // smsc_log_debug(log_,"R=%u deliveries are not ready, sleep=%u",
            // getRegionId(),toSleep);
            speedControl_.suspend((currentTime + toSleep) % flipTimePeriod);
            return toSleep;
        } else {
            // smsc_log_debug(log_,"R=%u delivery processed",getRegionId());
            speedControl_.consumeQuant();
            return 0;
        }
    } catch ( std::exception& e ) {
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
    {
        MutexGuard mg(lock_);
        DlvMap::iterator iter = dlvList_.lower_bound(dlvId);
        if (iter != dlvList_.end() && iter->first == dlvId ) {
            // already added
            return;
        }
        dlvList_.insert(iter,std::make_pair(dlvId,RegionalStoragePtr(&ptr)));
        taskList_.add(&ptr);
        usectime_type currentTime = (currentTimeMicro() + 1000) % flipTimePeriod;
        if ( speedControl_.getNextTime() > currentTime ) {
            speedControl_.suspend( currentTime );
        }
    }
    if (conn_) conn_->wakeUp();
}


void RegionSender::removeDelivery( dlvid_type dlvId )
{
    smsc_log_debug(log_,"remove delivery D=%u",dlvId);
    {
        MutexGuard mg(lock_);
        DlvMap::iterator iter = dlvList_.lower_bound(dlvId);
        if (iter == dlvList_.end() || iter->first != dlvId ) {
            // not found
            return;
        }
        taskList_.remove(EqualById(dlvId));
        dlvList_.erase(iter);
    }
}


unsigned RegionSender::scoredObjIsReady( unsigned unused, ScoredPtrType& ptr )
{
    static const unsigned sleepTimeException = unsigned(4*tuPerSec);
    if (!ptr) return sleepTimeException;
    try {
        if ( ptr->getState() == DLVSTATE_ACTIVE ) {
            // delivery is active
            const int sleepTimeNotReady = ptr->getNextMessage(currentTime_,
                                                              weekTime_,
                                                              msg_);
            if (sleepTimeNotReady <= 0) {
                smsc_log_debug(log_,"R=%u/D=%u/M=%llu is ready to be sent",
                               unsigned(getRegionId()),
                               unsigned(ptr->getDlvId()),
                               ulonglong(msg_.msgId));
                untilActiveEnd_ = -sleepTimeNotReady;
                return 0;
            } else {
                smsc_log_debug(log_,"R=%u/D=%u: is not ready, going to sleep %u usec",
                               unsigned(getRegionId()),
                               unsigned(ptr->getDlvId()),
                               ulonglong(sleepTimeNotReady));
                return sleepTimeNotReady; // wait one second
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


int RegionSender::processScoredObj(unsigned, ScoredPtrType& ptr, unsigned& objSleep)
{
    int nchunks = 0;
    int res;
    const int inc = maxScoreIncrement / ptr->getDlvInfo().getPriority();
    assert(!ptr == false);
    try {

        assert(conn_);
        res = conn_->send(*ptr, msg_, untilActiveEnd_, nchunks);
        if ( res == smsc::system::Status::OK && nchunks > 0 ) {
            // message has been put into output queue
            smsc_log_debug(log_,"R=%u/D=%u/M=%llu sent nchunks=%d",
                           unsigned(getRegionId()),
                           unsigned(ptr->getDlvId()),
                           ulonglong(msg_.msgId), nchunks);
            objSleep = 0;
            return inc * nchunks;
        } else {
            smsc_log_warn(log_,"R=%u/D=%u/M=%llu send failed, res=%d, nchunks=%d",
                          unsigned(getRegionId()),
                          unsigned(ptr->getDlvId()),
                          ulonglong(msg_.msgId), res, nchunks);
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
                       res, nchunks);
    objSleep = 0;
    return -inc;
}

}
}
