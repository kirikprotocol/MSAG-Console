#ifndef _INFORMER_DELIVERYSTATS_H
#define _INFORMER_DELIVERYSTATS_H

#include "util/int.h"
#include "informer/io/InfosmeException.h"
#include <cstring>

namespace eyeline {
namespace informer {

struct DeliveryStats
{
public:
    uint32_t     totalMessages;
    uint32_t     procMessages;    // a number of messages in opstore proc
    uint32_t     sentMessages;    // a number of messages in sent state
    uint32_t     retryMessages;   // the length of resend queue
    uint32_t     dlvdMessages;
    uint32_t     failedMessages;
    uint32_t     expiredMessages;

    void clear() {
        memset(this,0,sizeof(*this));
    }

    bool isEmpty() const {
        return 
            totalMessages == 0 &&
            procMessages == 0 &&
            sentMessages == 0 &&
            retryMessages == 0 &&
            dlvdMessages == 0 &&
            failedMessages == 0 &&
            expiredMessages == 0;
    }

    bool operator != ( const DeliveryStats& ds ) const {
        return 
            ( totalMessages   != ds.totalMessages   ) ||
            ( procMessages    != ds.procMessages    ) ||
            ( sentMessages    != ds.sentMessages    ) ||
            ( retryMessages   != ds.retryMessages   ) ||
            ( dlvdMessages    != ds.dlvdMessages    ) ||
            ( failedMessages  != ds.failedMessages  ) ||
            ( expiredMessages != ds.expiredMessages );
    }

    void incStat( uint8_t state, int value ) {
        if (!value) return;
        switch (MsgState(state)) {
        case MSGSTATE_PROCESS  : procMessages += value;  break;
        // case MSGSTATE_TAKEN    : procMessages += value;  break;
        case MSGSTATE_SENT     : sentMessages += value;  break;
        case MSGSTATE_RETRY    : retryMessages += value; break;
        case MSGSTATE_INPUT    : if (value>0) { totalMessages += value; } break;
        default :
            if (value<0) {
                throw InfosmeException("cannot decrement final state %d",state);
            }
            switch (MsgState(state)) {
            case MSGSTATE_DELIVERED: dlvdMessages += value; break;
            case MSGSTATE_EXPIRED  : expiredMessages += value; break;
            case MSGSTATE_FAILED   : failedMessages += value; break;
            default:
                throw InfosmeException("unknown state %d",state);
            }
        }
    }
};

} // informer
} // smsc

#endif
