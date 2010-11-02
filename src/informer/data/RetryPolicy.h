#ifndef _INFORMER_RETRYPOLICY_H
#define _INFORMER_RETRYPOLICY_H

#include "informer/io/Typedefs.h"

namespace eyeline {
namespace informer {

class DeliveryInfo;

/// the instance of this class is attached to each center.
class RetryPolicy
{
public:
    /// calculate the retry time for given delivery
    /// -1 - no retry -- permanent error
    ///  0 - immediate retry
    /// >0 - delayed retry
    timediff_type getRetryTime( const DeliveryInfo& info,
                                int                 smppState,
                                uint16_t            retryCount ) const;

    /// minimal number of message TTL, in seconds, to be attempted to retry.
    timediff_type getMinRetryTime() const {
        return 30;
    }
};

} // informer
} // smsc

#endif
