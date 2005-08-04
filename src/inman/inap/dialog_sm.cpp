#include "dialog.hpp"
#include "dialog_sm.h"

using namespace statemap;

namespace smsc
{
    namespace inman
    {
        namespace inap
        {
            // Static class declarations.
            TCAP_FSM_Idle TCAP_FSM::Idle("TCAP_FSM::Idle", 0);
            TCAP_FSM_Error TCAP_FSM::Error("TCAP_FSM::Error", 1);
            INAP_FSM_Idle INAP_FSM::Idle("INAP_FSM::Idle", 2);
            INAP_FSM_WaitingForInstructions INAP_FSM::WaitingForInstructions("INAP_FSM::WaitingForInstructions", 3);
            INAP_FSM_Monitoring INAP_FSM::Monitoring("INAP_FSM::Monitoring", 4);

            void DialogState::collectInfo(DialogContext& context)
            {
                Default(context);
                return;
            }

            void DialogState::connectSMS(DialogContext& context)
            {
                Default(context);
                return;
            }

            void DialogState::continueSMS(DialogContext& context)
            {
                Default(context);
                return;
            }

            void DialogState::deliveryRequest(DialogContext& context)
            {
                Default(context);
                return;
            }

            void DialogState::done(DialogContext& context)
            {
                Default(context);
                return;
            }

            void DialogState::error(DialogContext& context)
            {
                Default(context);
                return;
            }

            void DialogState::failure(DialogContext& context)
            {
                Default(context);
                return;
            }

            void DialogState::furnishChargingInformation(DialogContext& context)
            {
                Default(context);
                return;
            }

            void DialogState::releaseSMS(DialogContext& context)
            {
                Default(context);
                return;
            }

            void DialogState::requestReportSMSEvent(DialogContext& context)
            {
                Default(context);
                return;
            }

            void DialogState::resetTimer(DialogContext& context)
            {
                Default(context);
                return;
            }

            void DialogState::timerExpired(DialogContext& context)
            {
                Default(context);
                return;
            }

            void DialogState::Default(DialogContext& context)
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

            void TCAP_FSM_Idle::error(DialogContext& context)
            {

                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : TCAP_FSM error()"
                        << std::endl;
                }

                (context.getState()).Exit(context);
                context.setState(TCAP_FSM::Error);
                (context.getState()).Entry(context);

                return;
            }

            void TCAP_FSM_Error::error(DialogContext& context)
            {

                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : TCAP_FSM error()"
                        << std::endl;
                }


                return;
            }

            void INAP_FSM_Idle::collectInfo(DialogContext& context)
            {
                Dialog& ctxt = context.getOwner();

                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : INAP_FSM collectInfo()"
                        << std::endl;
                }

                (context.getState()).Exit(context);
                context.clearState();
                try
                {
                    ctxt.initialDPSMS();
                    context.setState(INAP_FSM::WaitingForInstructions);
                }
                catch (...)
                {
                    context.setState(INAP_FSM::WaitingForInstructions);
                    throw;
                }
                (context.getState()).Entry(context);

                return;
            }

            void INAP_FSM_Idle::deliveryRequest(DialogContext& context)
            {

                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : INAP_FSM deliveryRequest()"
                        << std::endl;
                }


                return;
            }

            void INAP_FSM_Idle::failure(DialogContext& context)
            {

                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : INAP_FSM failure()"
                        << std::endl;
                }


                return;
            }

            void INAP_FSM_WaitingForInstructions::connectSMS(DialogContext& context)
            {
                Dialog& ctxt = context.getOwner();

                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : INAP_FSM connectSMS()"
                        << std::endl;
                }

                (context.getState()).Exit(context);
                context.clearState();
                try
                {
                    ctxt.eventReportSMS();
                    context.setState(INAP_FSM::Monitoring);
                }
                catch (...)
                {
                    context.setState(INAP_FSM::Monitoring);
                    throw;
                }
                (context.getState()).Entry(context);

                return;
            }

            void INAP_FSM_WaitingForInstructions::continueSMS(DialogContext& context)
            {

                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : INAP_FSM continueSMS()"
                        << std::endl;
                }

                (context.getState()).Exit(context);
                context.setState(INAP_FSM::Monitoring);
                (context.getState()).Entry(context);

                return;
            }

            void INAP_FSM_WaitingForInstructions::failure(DialogContext& context)
            {

                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : INAP_FSM failure()"
                        << std::endl;
                }

                (context.getState()).Exit(context);
                context.setState(INAP_FSM::Idle);
                (context.getState()).Entry(context);

                return;
            }

            void INAP_FSM_WaitingForInstructions::furnishChargingInformation(DialogContext& context)
            {

                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : INAP_FSM furnishChargingInformation()"
                        << std::endl;
                }


                return;
            }

            void INAP_FSM_WaitingForInstructions::releaseSMS(DialogContext& context)
            {

                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : INAP_FSM releaseSMS()"
                        << std::endl;
                }

                (context.getState()).Exit(context);
                context.setState(INAP_FSM::Idle);
                (context.getState()).Entry(context);

                return;
            }

            void INAP_FSM_WaitingForInstructions::requestReportSMSEvent(DialogContext& context)
            {

                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : INAP_FSM requestReportSMSEvent()"
                        << std::endl;
                }


                return;
            }

            void INAP_FSM_WaitingForInstructions::resetTimer(DialogContext& context)
            {

                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : INAP_FSM resetTimer()"
                        << std::endl;
                }


                return;
            }

            void INAP_FSM_WaitingForInstructions::timerExpired(DialogContext& context)
            {

                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : INAP_FSM timerExpired()"
                        << std::endl;
                }

                (context.getState()).Exit(context);
                context.setState(INAP_FSM::Idle);
                (context.getState()).Entry(context);

                return;
            }

            void INAP_FSM_Monitoring::done(DialogContext& context)
            {
                Dialog& ctxt = context.getOwner();

                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : INAP_FSM done()"
                        << std::endl;
                }

                (context.getState()).Exit(context);
                context.clearState();
                try
                {
                    ctxt.eventReportSMS();
                    context.setState(INAP_FSM::Idle);
                }
                catch (...)
                {
                    context.setState(INAP_FSM::Idle);
                    throw;
                }
                (context.getState()).Entry(context);

                return;
            }
        }
    }
}
