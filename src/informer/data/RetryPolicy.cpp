#include "system/status.h"
#include "RetryPolicy.h"
#include "logger/Logger.h"

namespace {
smsc::logger::Logger* log_ = 0;
void getlog()
{
}
}

namespace eyeline {
namespace informer {

using namespace smsc::system::Status;

timediff_type RetryPolicy::getRetryTime( const DeliveryInfo& info,
                                         int                 smppState,
                                         uint16_t            retryCount ) const
{
    // FIXME: apply per smsc policy here
    if ( isErrorPermanent(smppState) ) {
        return -1;
    }
    switch (smppState) {
    case OK :
    case MSGQFUL :
    case THROTTLED :
    case SMENOTCONNECTED :
    case LICENSELIMITREJECT :
        return 0;
    default:
        return 100;
    }
}


}
}
