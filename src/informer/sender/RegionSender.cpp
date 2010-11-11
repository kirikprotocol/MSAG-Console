#include "RegionSender.h"
#include "SmscSender.h"
#include "system/status.h"
#include "informer/data/DeliveryInfo.h"

namespace {
using namespace eyeline::informer;
struct EqualById {
    inline EqualById(dlvid_type dlvId) : d(dlvId) {}
    inline bool operator () ( const RegionalStoragePtr& ptr ) const {
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
          smsc::logger::Logger::getInstance("dlvlist"))
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
    sprintf(buf,"R=%u",getRegionId());
    return buf;
}


unsigned RegionSender::processRegion( usectime_type currentTime )
{
    smsc_log_debug(log_,"R=%u processing at %llu",getRegionId(),currentTime);
    static const unsigned sleepTime = unsigned(1*tuPerSec);
    currentTime_ = currentTime;
    MutexGuard mg(lock_);
    return taskList_.processOnce(0/*not used*/,sleepTime);
}


void RegionSender::addDelivery( RegionalStorage& ptr )
{
    smsc_log_debug(log_,"add delivery D=%u",ptr.getDlvId());
    {
        MutexGuard mg(lock_);
        taskList_.add(RegionalStoragePtr(&ptr));
    }
    // FIXME: reset speed control
    if (conn_) conn_->wakeUp();
}


void RegionSender::removeDelivery( dlvid_type dlvId )
{
    smsc_log_debug(log_,"remove delivery D=%u",dlvId);
    {
        MutexGuard mg(lock_);
        taskList_.remove(EqualById(dlvId));
    }
    if (log_->isDebugEnabled()) {
        std::string dumpstring;
        dumpstring.reserve(200);
        taskList_.dump(dumpstring);
        smsc_log_debug(log_,"list after remove: %s",dumpstring.c_str());
    }
}


unsigned RegionSender::scoredObjIsReady( unsigned unused, ScoredPtrType& ptr )
{
    static const unsigned sleepTimeNotReady = unsigned(1*tuPerSec);
    static const unsigned sleepTimeException = unsigned(5*tuPerSec);
    if (!ptr) return sleepTimeException;
    try {
        if ( ptr->getState() == DLVSTATE_ACTIVE ) {
            // delivery is active
            if ( ptr->getNextMessage(msgtime_type(currentTime_/tuPerSec),msg_) ) {
                smsc_log_debug(log_,"R=%u/D=%u/M=%llu is ready to be sent",
                               unsigned(getRegionId()),
                               unsigned(ptr->getDlvId()),
                               ulonglong(msg_.msgId));
                return 0;
            } else {
                smsc_log_debug(log_,"R=%u/D=%u: is not ready, going to sleep %llu usec",
                               unsigned(getRegionId()),
                               unsigned(ptr->getDlvId()),
                               ulonglong(sleepTimeNotReady));
                return sleepTimeNotReady; // wait one second
            }
        }
    } catch ( std::exception& e ) {
        smsc_log_warn(log_,"R=%u/D=%u exc in isReady: %s",
                      unsigned(getRegionId()),
                      unsigned(ptr->getDlvId()));
    }
    return sleepTimeException;  // wait 5 seconds
}


int RegionSender::processScoredObj(unsigned, ScoredPtrType& ptr)
{
    int nchunks = 0;
    int res;
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
            return maxScoreIncrement / nchunks / ptr->getDlvInfo().getPriority();
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
                       msgtime_type(currentTime_/tuPerSec), res, nchunks);
    return -maxScoreIncrement;
}

}
}
