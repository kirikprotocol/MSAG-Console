#ifndef SCAG_BILLING_BILL
#define SCAG_BILLING_BILL

#include <inttypes.h>

namespace scag { namespace bill
{
    struct Bill
    {
        uint8_t     machine_id;
        uint64_t    bill_id;
    };
}}

#endif // SCAG_BILLING_BILL

