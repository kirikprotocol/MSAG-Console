#ifndef SCAG_BILLING_BILL
#define SCAG_BILLING_BILL

#include <inttypes.h>
#include <string>

namespace scag { namespace bill
{
    struct Bill
    {
        uint8_t     machine_id;
        uint64_t    bill_id;

        bool operator == (const Bill& bill)
        {
            return ((bill.bill_id == this->bill_id) && (bill.machine_id == this->machine_id));
        }

    };

    struct BillKey
    {
        std::string OA;
        std::string DA;
        int key;

        bool operator == (const BillKey& billKey)
        {
            return (((billKey.DA == this->DA)&&(billKey.OA == this->OA)&&(billKey.key == this->key))||
                    ((billKey.DA == this->OA)&&(billKey.OA == this->DA)&&(billKey.key == this->key)));
        }
    };
}}

#endif // SCAG_BILLING_BILL

