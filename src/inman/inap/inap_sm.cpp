#include "inap.hpp"
#include "inap_sm.h"

using namespace statemap;

namespace smsc
{
    namespace inman
    {
        namespace inap
        {
            // Static class declarations.
            INAP_FSM_Idle INAP_FSM::Idle("INAP_FSM::Idle", 0);
            INAP_FSM_InittialDpInvoked INAP_FSM::InittialDpInvoked("INAP_FSM::InittialDpInvoked", 1);
            INAP_FSM_WaitForInstructions INAP_FSM::WaitForInstructions("INAP_FSM::WaitForInstructions", 2);
            INAP_FSM_EventReportInvoked INAP_FSM::EventReportInvoked("INAP_FSM::EventReportInvoked", 3);

            void INAPState::continueSMS(INAPContext& context)
            {
                Default(context);
                return;
            }

            void INAPState::failed(INAPContext& context, const TcapOperation& error)
            {
                Default(context);
                return;
            }

            void INAPState::releaseSMS(INAPContext& context)
            {
                Default(context);
                return;
            }

            void INAPState::requestReportEvent(INAPContext& context)
            {
                Default(context);
                return;
            }

            void INAPState::start(INAPContext& context)
            {
                Default(context);
                return;
            }

            void INAPState::successed(INAPContext& context, const TcapOperation& result)
            {
                Default(context);
                return;
            }

            void INAPState::Default(INAPContext& context)
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

            void INAP_FSM_Idle::start(INAPContext& context)
            {
                INAP& ctxt = context.getOwner();

                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : INAP_FSM start()"
                        << std::endl;
                }

                (context.getState()).Exit(context);
                context.clearState();
                try
                {
                    ctxt.sendInitialDP();
                    context.setState(INAP_FSM::InittialDpInvoked);
                }
                catch (...)
                {
                    context.setState(INAP_FSM::InittialDpInvoked);
                    throw;
                }
                (context.getState()).Entry(context);

                return;
            }

            void INAP_FSM_InittialDpInvoked::failed(INAPContext& context, const TcapOperation& error)
            {

                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : INAP_FSM failed(const TcapOperation& error)"
                        << std::endl;
                }

                (context.getState()).Exit(context);
                context.setState(INAP_FSM::Idle);
                (context.getState()).Entry(context);

                return;
            }

            void INAP_FSM_InittialDpInvoked::successed(INAPContext& context, const TcapOperation& result)
            {

                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : INAP_FSM successed(const TcapOperation& result)"
                        << std::endl;
                }

                (context.getState()).Exit(context);
                context.setState(INAP_FSM::WaitForInstructions);
                (context.getState()).Entry(context);

                return;
            }

            void INAP_FSM_WaitForInstructions::continueSMS(INAPContext& context)
            {
                INAP& ctxt = context.getOwner();

                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : INAP_FSM continueSMS()"
                        << std::endl;
                }

                (context.getState()).Exit(context);
                context.clearState();
                try
                {
                    ctxt.sendEventReport();
                    context.setState(INAP_FSM::EventReportInvoked);
                }
                catch (...)
                {
                    context.setState(INAP_FSM::EventReportInvoked);
                    throw;
                }
                (context.getState()).Entry(context);

                return;
            }

            void INAP_FSM_WaitForInstructions::releaseSMS(INAPContext& context)
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

            void INAP_FSM_WaitForInstructions::requestReportEvent(INAPContext& context)
            {

                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : INAP_FSM requestReportEvent()"
                        << std::endl;
                }


                return;
            }

            void INAP_FSM_EventReportInvoked::failed(INAPContext& context, const TcapOperation& error)
            {

                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : INAP_FSM failed(const TcapOperation& error)"
                        << std::endl;
                }

                (context.getState()).Exit(context);
                context.setState(INAP_FSM::Idle);
                (context.getState()).Entry(context);

                return;
            }

            void INAP_FSM_EventReportInvoked::successed(INAPContext& context, const TcapOperation& result)
            {

                if (context.getDebugFlag() == true)
                {
                    std::ostream& str = context.getDebugStream();

                    str << "TRANSITION   : INAP_FSM successed(const TcapOperation& result)"
                        << std::endl;
                }

                (context.getState()).Exit(context);
                context.setState(INAP_FSM::Idle);
                (context.getState()).Entry(context);

                return;
            }
        }
    }
}
