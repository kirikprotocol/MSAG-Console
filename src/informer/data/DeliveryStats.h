#ifndef _INFORMER_DELIVERYSTATS_H
#define _INFORMER_DELIVERYSTATS_H

#include "util/int.h"
#include <cstring>

namespace eyeline {
namespace informer {

struct DeliveryStats
{
public:
    /// firstMessageSent field may only be set once.
    msgtime_type firstMessageSent;
    /// all other fields may only grow.
    msgtime_type lastMessageSent;
    uint32_t     totalMessages;
    uint32_t     sentMessages;
    uint32_t     dlvdMessages;
    uint32_t     failedMessages;
    uint32_t     expiredMessages;

    void clear() {
        memset(this,0,sizeof(this));
    }
};

} // informer
} // smsc

#endif
