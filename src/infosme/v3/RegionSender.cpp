#include "RegionSender.h"
#include "SmscSender.h"

namespace smsc {
namespace infosme {

unsigned RegionSender::scoredObjIsReady( unsigned deltaus, Delivery& dlv )
{
    try {
        if ( dlv.isActive() ) {
            ptr_ = dlv.getRegionalStorage( getRegionId() );
            if ( ! ptr_.get() ) {
                smsc_log_debug(log_,"rgn=%u, dlv=%u, region storage is not found",
                               getRegionId(), dlv.getDlvId() );
            } else if ( ptr_->getNextMessage(currentTime_,msg_) ) {
                return 0;
            } else {
                smsc_log_debug(log_,"rgn=%u, dlv=%u, no msg is ready",
                               getRegionId(), dlv.getDlvId() );
            }
        } else {
            smsc_log_debug(log_,"rgn=%u, dlv=%u is not ready",
                           getRegionId(), dlv.getDlvId() );
        }
    } catch ( std::exception& e ) {
        smsc_log_warn(log_,"rgn=%u, exc in objIsReady dlv=%u: %s",
                      getRegionId(), dlv.getDlvId(), e.what());
    }
    return 3000000U;
}


int RegionSender::processScoredObj( unsigned deltaus, Delivery& dlv )
{
    try {

        const int nchunks = conn_->send( currentTime_, dlv, getRegionId(), msg_ );
        if ( nchunks > 0 ) {
            // message has been put into output queue
            ptr_->messageSent( msg_.msgId, currentTime_ );
            return BASEINCREMENT/nchunks/dlv.getPriority();
        } else {
            smsc_log_warn(log_,"rgn=%u, dlv=%u, procObj nchunks=%d",
                          getRegionId(), dlv.getDlvId(), nchunks);
        }

    } catch ( std::exception& e ) {
        smsc_log_warn(log_,"rgn=%u, dlv=%u, procObj exc: %s",
                      getRegionId(), dlv.getDlvId(), e.what());
    }
    // FIXME: message could not be sent, mark it as failed?
    ptr_->retryMessage( msg_.msgId, currentTime_, 60, 8 );
    return -BASEINCREMENT;
}

}
}
