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
            BILLING_WAITING_FOR_INSTRUCTIONS BILLING::WAITING_FOR_INSTRUCTIONS("BILLING::WAITING_FOR_INSTRUCTIONS", 1);
            BILLING_MONITORING BILLING::MONITORING("BILLING::MONITORING", 2);

            void BillingState::connectSMS(BillingContext& context)
            {
                Default(context);
                return;
            }

            void BillingState::continueSMS(BillingContext& context)
            {
                Default(context);
                return;
            }

            void BillingState::initialDPSMS(BillingContext& context)
            {
                Default(context);
                return;
            }

            void BillingState::releaseSMS(BillingContext& context)
            {
                Default(context);
                return;
            }

            void BillingState::Default(BillingContext& context)
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

            void BILLING_IDLE::initialDPSMS(BillingContext& context)
            {

                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : BILLING initialDPSMS()"
                        << std::endl;
                }

                (context.getState()).Exit(context);
                context.setState(BILLING::WAITING_FOR_INSTRUCTIONS);
                (context.getState()).Entry(context);

                return;
            }

            void BILLING_WAITING_FOR_INSTRUCTIONS::connectSMS(BillingContext& context)
            {

                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : BILLING connectSMS()"
                        << std::endl;
                }

                (context.getState()).Exit(context);
                context.setState(BILLING::MONITORING);
                (context.getState()).Entry(context);

                return;
            }

            void BILLING_WAITING_FOR_INSTRUCTIONS::continueSMS(BillingContext& context)
            {

                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : BILLING continueSMS()"
                        << std::endl;
                }

                (context.getState()).Exit(context);
                context.setState(BILLING::MONITORING);
                (context.getState()).Entry(context);

                return;
            }

            void BILLING_WAITING_FOR_INSTRUCTIONS::releaseSMS(BillingContext& context)
            {

                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : BILLING releaseSMS()"
                        << std::endl;
                }

                (context.getState()).Exit(context);
                context.setState(BILLING::IDLE);
                (context.getState()).Entry(context);

                return;
            }
        }
    }
}
