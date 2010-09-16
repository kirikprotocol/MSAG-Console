#ifndef _INFOSME_V3_DELIVERYINFO_H
#define _INFOSME_V3_DELIVERYINFO_H

#include "Typedefs.h"

namespace smsc {
namespace infosme {

class DeliveryInfo
{
public:
    DeliveryInfo( dlvid_type dlvid ) : dlvid_(dlvid) {}

    dlvid_type getDlvId() const { return dlvid_; }
    unsigned getPriority() const { return 1; }

    /// minimal number of input messages per region when request for new
    /// input messages should be issued.
    unsigned getMinInputQueueSize() const { return 5; }

    /// maximum number of messages in resend queue that prevents the
    /// request for new input messages.
    unsigned getMaxResendQueueSize() const { return 20; }

    /// number of input messages to be requested.
    unsigned getUploadCount() const { return 10; }

    /// minimal time between retries, seconds
    unsigned getMinRetryTime() const { return 60; }

private:
    dlvid_type dlvid_;
};

} // infosme
} // smsc

#endif
