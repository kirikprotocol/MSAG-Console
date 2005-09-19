#ifndef SCAG_BILLING_MANAGER
#define SCAG_BILLING_MANAGER

#include <inttypes.h>
#include "Bill.h"
#include <string>

namespace scag { namespace re { namespace actions {
     class ActionFactory;
}}}


namespace scag { namespace bill
{

    struct BillingManagerConfig
    {
        std::string cfg_dir;
        std::string so_dir;
        scag::re::actions::ActionFactory * mainActionFactory;

        BillingManagerConfig() : mainActionFactory(0) {}
    };

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

        virtual void rollback(const Bill& bill) = 0;

    };
}}

#endif // SCAG_BILLING_MANAGER

