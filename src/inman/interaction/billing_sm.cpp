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

            void BillingState::eventReportSMS(BillingContext& context)
            {
                Default(context);
                return;
            }

            void BillingState::furnishChargingInformationSMS(BillingContext& context)
            {
                Default(context);
                return;
            }

            void BillingState::nil(BillingContext& context)
            {
                Default(context);
                return;
            }

            void BillingState::releaseSMS(BillingContext& context)
            {
                Default(context);
                return;
            }

            void BillingState::requestReportSMSEvent(BillingContext& context)
            {
                Default(context);
                return;
            }

            void BillingState::resetTimerSMS(BillingContext& context)
            {
                Default(context);
                return;
            }

            void BillingState::smsRejected(BillingContext& context)
            {
                Default(context);
                return;
            }

            void BillingState::smsSent(BillingContext& context)
            {
                Default(context);
                return;
            }

            void BillingState::start(BillingContext& context)
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

            void BILLING_IDLE::nil(BillingContext& context)
            {

                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : BILLING nil()"
                        << std::endl;
                }


                return;
            }

            void BILLING_IDLE::start(BillingContext& context)
            {

                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : BILLING start()"
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


                return;
            }

            void BILLING_WAITING_FOR_INSTRUCTIONS::eventReportSMS(BillingContext& context)
            {

                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : BILLING eventReportSMS()"
                        << std::endl;
                }


                return;
            }

            void BILLING_WAITING_FOR_INSTRUCTIONS::furnishChargingInformationSMS(BillingContext& context)
            {

                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : BILLING furnishChargingInformationSMS()"
                        << std::endl;
                }


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


                return;
            }

            void BILLING_WAITING_FOR_INSTRUCTIONS::requestReportSMSEvent(BillingContext& context)
            {

                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : BILLING requestReportSMSEvent()"
                        << std::endl;
                }


                return;
            }

            void BILLING_WAITING_FOR_INSTRUCTIONS::resetTimerSMS(BillingContext& context)
            {

                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : BILLING resetTimerSMS()"
                        << std::endl;
                }


                return;
            }

            void BILLING_MONITORING::smsRejected(BillingContext& context)
            {

                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : BILLING smsRejected()"
                        << std::endl;
                }

                (context.getState()).Exit(context);
                context.setState(BILLING::IDLE);
                (context.getState()).Entry(context);

                return;
            }

            void BILLING_MONITORING::smsSent(BillingContext& context)
            {

                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : BILLING smsSent()"
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
