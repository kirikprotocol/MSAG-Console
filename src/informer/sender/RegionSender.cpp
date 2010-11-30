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

RegionSender::RegionSender( SmscSender& conn, const RegionPtr& r ) :
log_(smsc::logger::Logger::getInstance("regsend")),
ref_(0),
conn_(0),
region_(r),
taskList_(*this,2*maxScoreIncrement,
          smsc::logger::Logger::getInstance("dlvlist")),
speedControl_(region_->getBandwidth())
{
    smsc_log_debug(log_,"ctor S='%s' R=%u",conn.getSmscId().c_str(),unsigned(r->getRegionId()));
    assignSender(&conn);
}


void RegionSender::assignSender( SmscSender* conn )
{
    if ( conn_ != conn ) {
        if (conn_) conn_->detachRegionSender(*this);
        conn_ = conn;
        if (conn_) conn_->attachRegionSender(*this);
    }
}


std::string RegionSender::toString() const
{
    char buf[30];
    sprintf(buf,"R=%u bw=%u",getRegionId(),getBandwidth());
    return buf;
}


bool RegionSender::processRegion( usectime_type currentTime )
{
    try {
        smsc_log_debug(log_,"R=%u processing at %llu",getRegionId(),currentTime);
        currentTime_ = currentTime;
        const msgtime_type now = msgtime_type(currentTime_ / tuPerSec);
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
        MutexGuard mg(lock_);
        // check speed control
        if ( speedControl_.isReady( currentTime % flipTimePeriod,
                                    maxSnailDelay ) > 0 ) {
            return false;
        }
        
        const unsigned toSleep = taskList_.processOnce(0/*not used*/,tuPerSec);
        if (toSleep>0) {
            smsc_log_debug(log_,"R=%u deliveries are not ready, sleep=%u",
                           getRegionId(),toSleep);
            speedControl_.suspend((currentTime + toSleep) % flipTimePeriod);
            return false;
        } else {
            smsc_log_debug(log_,"R=%u delivery processed",getRegionId());
            speedControl_.consumeQuant();
            return true;
        }
    } catch ( std::exception& e ) {
        smsc_log_debug(log_,"R=%u send exc: %s",getRegionId(),e.what());
        speedControl_.suspend( (currentTime + tuPerSec) % flipTimePeriod);
    }
    return false;
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
    }
    /*
    if (log_->isDebugEnabled()) {
        std::string dumpstring;
        dumpstring.reserve(200);
        taskList_.dump(dumpstring);
        smsc_log_debug(log_,"list after remove: %s",dumpstring.c_str());
    }
     */
}


unsigned RegionSender::scoredObjIsReady( unsigned unused, ScoredPtrType& ptr )
{
    static const unsigned sleepTimeException = unsigned(4*tuPerSec);
    if (!ptr) return sleepTimeException;
    try {
        if ( ptr->getState() == DLVSTATE_ACTIVE ) {
            // delivery is active
            const unsigned sleepTimeNotReady = ptr->getNextMessage(currentTime_,
                                                                   weekTime_,
                                                                   msg_);
            if (sleepTimeNotReady == 0) {
                smsc_log_debug(log_,"R=%u/D=%u/M=%llu is ready to be sent",
                               unsigned(getRegionId()),
                               unsigned(ptr->getDlvId()),
                               ulonglong(msg_.msgId));
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


int RegionSender::processScoredObj(unsigned, ScoredPtrType& ptr)
{
    int nchunks = 0;
    int res;
    const int inc = maxScoreIncrement / ptr->getDlvInfo().getPriority();
    assert(!ptr == false);
    try {

        assert(conn_);
        res = conn_->send(*ptr, msg_, nchunks);
        if ( res == smsc::system::Status::OK && nchunks > 0 ) {
            // message has been put into output queue
            smsc_log_debug(log_,"R=%u/D=%u/M=%llu sent nchunks=%d",
                           unsigned(getRegionId()),
                           unsigned(ptr->getDlvId()),
                           ulonglong(msg_.msgId), nchunks);
            // message is considered to be sent only on response
            // ptr.messageSent(msg_.msgId, msgtime_type(currentTime_/tuPerSec));
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
    return -inc;
}

}
}
