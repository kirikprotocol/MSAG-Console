#include "billing.hpp"
#include "billing_sm.h"

using namespace statemap;

namespace smsc
{
    namespace inman
    {
        namespace interaction
        {
            // Static class declarations.
            BILLING_IDLE BILLING::IDLE("BILLING::IDLE", 0);
            BILLING_WAIT_FOR_INSTRUCTIONS BILLING::WAIT_FOR_INSTRUCTIONS("BILLING::WAIT_FOR_INSTRUCTIONS", 1);
            BILLING_MONITORING BILLING::MONITORING("BILLING::MONITORING", 2);

            void BillingInteractionState::Default(BillingInteractionContext& context)
            {
                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : Default"
                        << std::endl;
                }

                throw (
                    TransitionUndefinedException(
                        context.getState().getName(),
                        context.getTransition()));

                return;
            }
        }
    }
}
