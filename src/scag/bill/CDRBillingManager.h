#ifndef SCAG_BILLING_MANAGER
#define SCAG_BILLING_MANAGER

#include <inttypes.h>
#include "Bill.h"
#include <string>
#include "scag/config/bill/BillingManagerConfig.h"

  


namespace scag { namespace bill 
{

    using scag::config::BillingManagerConfig;


    class BillingManager
    {
        BillingManager(const BillingManager& bm);
        BillingManager& operator=(const BillingManager& bm);

    protected:
        BillingManager() {};
        virtual ~BillingManager() {};
    public:

        static void Init(BillingManagerConfig& config);
        static BillingManager& Instance();

        virtual void rollback(int BillId) = 0;

    };
}}

#endif // SCAG_BILLING_MANAGER

