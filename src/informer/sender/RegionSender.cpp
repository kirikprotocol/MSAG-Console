#include "RegionSender.h"
#include "SmscSender.h"

namespace eyeline {
namespace informer {

RegionSender::RegionSender( SmscSender& conn, const RegionPtr& r ) :
log_(smsc::logger::Logger::getInstance("regsend")),
conn_(0), region_(0),
taskList_(*this,2*maxScoreIncrement,log_)
{
    smsc_log_debug(log_,"ctor S='%s' R=%u",conn.getSmscId().c_str(),unsigned(r->getRegionId()));
    assignSender(conn,r);
}


void RegionSender::assignSender( SmscSender& conn, const RegionPtr& r )
{
    if ( conn_ != &conn ) {
        if (conn_) conn_->detachRegionSender(*this);
        conn_ = &conn;
        region_ = r;
        conn_->attachRegionSender(*this);
    } else {
        region_ = r;
    }
}


std::string RegionSender::toString() const
{
    char buf[30];
    sprintf(buf,"R=%u",getRegionId());
    return buf;
}


unsigned RegionSender::scoredObjIsReady( unsigned unused, ScoredObjType& ptr )
{
    static const unsigned sleepTimeNotReady = unsigned(1*tuPerSec);
    static const unsigned sleepTimeException = unsigned(5*tuPerSec);
    try {
        if ( ptr->getDlvInfo().isActive() ) {
            // delivery is active
            if ( ptr->getNextMessage(msgtime_type(currentTime_/tuPerSec),msg_) ) {
                smsc_log_debug(log_,"R=%u/D=%u/M=%u is ready to be sent",
                               getRegionId(), ptr->getDlvId(), msg_.msgId);
                return 0;
            } else {
                smsc_log_debug(log_,"R=%u/D=%u: is not ready, will sleep %u usec",
                               getRegionId(), ptr->getDlvId(), sleepTimeNotReady);
                return sleepTimeNotReady; // wait one second
            }
        }
    } catch ( std::exception& e ) {
        smsc_log_warn(log_,"R=%u/D=%u exc in isReady: %s",
                      getRegionId(), ptr->getDlvId());
    }
    return sleepTimeException;  // wait 5 seconds
}


int RegionSender::processScoredObj(unsigned, ScoredObjType& ptr)
{
    try {

        const int nchunks = conn_->send(ptr, msg_);
        if ( nchunks > 0 ) {
            // message has been put into output queue
            smsc_log_debug(log_,"R=%u/D=%u/M=%u sent nchunks=%d",
                           getRegionId(), ptr->getDlvId(), msg_.msgId, nchunks);
            ptr->messageSent(msg_.msgId, msgtime_type(currentTime_/tuPerSec));
            return maxScoreIncrement / nchunks / ptr->getDlvInfo().getPriority();
        } else {
            smsc_log_warn(log_,"R=%u/D=%u/M=%u send failed nchunks=%d",
                          getRegionId(), ptr->getDlvId(), msg_.msgId, nchunks);
        }

    } catch ( std::exception& e ) {
        smsc_log_warn(log_,"R=%u/D=%u/M=%u send failed, exc: %s",
                      getRegionId(), ptr->getDlvId(), msg_.msgId, e.what());
    }
    smsc_log_debug(log_,"FIXME: message could not be sent, mark it as failed?");
    ptr->retryMessage(msg_.msgId, msgtime_type(currentTime_/tuPerSec), 60, 8 );
    return -maxScoreIncrement;
}

}
}
