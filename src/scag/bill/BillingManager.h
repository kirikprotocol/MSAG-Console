#ifndef SCAG_BILLING_MANAGER
#define SCAG_BILLING_MANAGER

#include <inttypes.h>
#include "Bill.h"
#include <string>

namespace scag { namespace bill
{
    class BillingManager
    {
    protected:

        BillingManager() {};
        virtual ~BillingManager() = 0;
   
    private:

        BillingManager(const BillingManager& bm);
        BillingManager& operator=(const BillingManager& bm);

    public:

        static void Init(const std::string& cfg_dir, const std::string& so_dir);
        static BillingManager& Instance();

        virtual void rollback(const Bill& bill) = 0;
    };
}}

#endif // SCAG_BILLING_MANAGER

