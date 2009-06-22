#ifndef SCAG_BILL_EWALLET_STATUS_H
#define SCAG_BILL_EWALLET_STATUS_H

#include "util/int.h"

namespace scag2 {
namespace bill {
namespace ewallet {

class Status
{
public:
    enum {
        OK             = 0x00,
        // info

        // warning (current request cannot be performed)
        BAD_REQUEST    = 0x10,
        BAD_RESPONSE   = 0x11,
        TIMEOUT        = 0x12,

        // errors
        IO_ERROR       = 0x20,
        NOT_SUPPORTED  = 0x21,
        NOT_CONNECTED  = 0x22,
        CLIENT_BUSY    = 0x23,
        CONFIG_INVALID = 0x24,

        UNKNOWN      = 0xff
    };
};

} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_STATUS_H */
