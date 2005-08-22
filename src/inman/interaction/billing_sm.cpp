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

            void BillingState::furnishChargingInformationSMS(BillingContext& context)
            {
                Default(context);
                return;
            }

            void BillingState::releaseSMS(BillingContext& context)
            {
                Default(context);
                return;
            }

            void BillingState::requestReportSMSEvent(BillingContext& context, RequestReportSMSEventArg* arg)
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
                Billing& ctxt = context.getOwner();

                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : BILLING releaseSMS()"
                        << std::endl;
                }

                (context.getState()).Exit(context);
                context.clearState();
                try
                {
                    ctxt.notifyFailure();
                    context.setState(BILLING::IDLE);
                }
                catch (...)
                {
                    context.setState(BILLING::IDLE);
                    throw;
                }
                (context.getState()).Entry(context);

                return;
            }

            void BILLING_WAITING_FOR_INSTRUCTIONS::requestReportSMSEvent(BillingContext& context, RequestReportSMSEventArg* arg)
            {
                Billing& ctxt = context.getOwner();

                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : BILLING requestReportSMSEvent(RequestReportSMSEventArg* arg)"
                        << std::endl;
                }

                BillingState& EndStateName = context.getState();

                context.clearState();
                try
                {
                    ctxt.armDetectionPoints(arg);
                    context.setState(EndStateName);
                }
                catch (...)
                {
                    context.setState(EndStateName);
                    throw;
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
                Billing& ctxt = context.getOwner();

                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : BILLING smsRejected()"
                        << std::endl;
                }

                (context.getState()).Exit(context);
                context.clearState();
                try
                {
                    ctxt.sendReportSMSEvent();
                    context.setState(BILLING::IDLE);
                }
                catch (...)
                {
                    context.setState(BILLING::IDLE);
                    throw;
                }
                (context.getState()).Entry(context);

                return;
            }

            void BILLING_MONITORING::smsSent(BillingContext& context)
            {
                Billing& ctxt = context.getOwner();

                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : BILLING smsSent()"
                        << std::endl;
                }

                (context.getState()).Exit(context);
                context.clearState();
                try
                {
                    ctxt.sendReportSMSEvent();
                    context.setState(BILLING::IDLE);
                }
                catch (...)
                {
                    context.setState(BILLING::IDLE);
                    throw;
                }
                (context.getState()).Entry(context);

                return;
            }
        }
    }
}
