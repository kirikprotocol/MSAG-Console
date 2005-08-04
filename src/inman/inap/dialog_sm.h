#ifndef _H_DIALOG_SM
#define _H_DIALOG_SM

#define SMC_USES_IOSTREAMS

#include "statemap.h"

namespace smsc
{
    namespace inman
    {
        namespace inap
        {
            // Forward declarations.
            class TCAP_FSM;
            class TCAP_FSM_Idle;
            class TCAP_FSM_Error;
            class TCAP_FSM_Default;
            class INAP_FSM;
            class INAP_FSM_Idle;
            class INAP_FSM_WaitingForInstructions;
            class INAP_FSM_Monitoring;
            class INAP_FSM_Default;
            class DialogState;
            class DialogContext;
            class Dialog;

            class DialogState :
                public statemap::State
            {
            public:

                DialogState(const char *name, int stateId)
                : statemap::State(name, stateId)
                {};

                virtual void Entry(DialogContext&) {};
                virtual void Exit(DialogContext&) {};

                virtual void collectInfo(DialogContext& context);
                virtual void connectSMS(DialogContext& context);
                virtual void continueSMS(DialogContext& context);
                virtual void deliveryRequest(DialogContext& context);
                virtual void done(DialogContext& context);
                virtual void error(DialogContext& context);
                virtual void failure(DialogContext& context);
                virtual void furnishChargingInformation(DialogContext& context);
                virtual void releaseSMS(DialogContext& context);
                virtual void requestReportSMSEvent(DialogContext& context);
                virtual void resetTimer(DialogContext& context);
                virtual void timerExpired(DialogContext& context);

            protected:

                virtual void Default(DialogContext& context);
            };

            class TCAP_FSM
            {
            public:

                static TCAP_FSM_Idle Idle;
                static TCAP_FSM_Error Error;
            };

            class TCAP_FSM_Default :
                public DialogState
            {
            public:

                TCAP_FSM_Default(const char *name, int stateId)
                : DialogState(name, stateId)
                {};

            };

            class TCAP_FSM_Idle :
                public TCAP_FSM_Default
            {
            public:
                TCAP_FSM_Idle(const char *name, int stateId)
                : TCAP_FSM_Default(name, stateId)
                {};

                void error(DialogContext& context);
            };

            class TCAP_FSM_Error :
                public TCAP_FSM_Default
            {
            public:
                TCAP_FSM_Error(const char *name, int stateId)
                : TCAP_FSM_Default(name, stateId)
                {};

                void error(DialogContext& context);
            };

            class INAP_FSM
            {
            public:

                static INAP_FSM_Idle Idle;
                static INAP_FSM_WaitingForInstructions WaitingForInstructions;
                static INAP_FSM_Monitoring Monitoring;
            };

            class INAP_FSM_Default :
                public DialogState
            {
            public:

                INAP_FSM_Default(const char *name, int stateId)
                : DialogState(name, stateId)
                {};

            };

            class INAP_FSM_Idle :
                public INAP_FSM_Default
            {
            public:
                INAP_FSM_Idle(const char *name, int stateId)
                : INAP_FSM_Default(name, stateId)
                {};

                void collectInfo(DialogContext& context);
                void deliveryRequest(DialogContext& context);
                void failure(DialogContext& context);
            };

            class INAP_FSM_WaitingForInstructions :
                public INAP_FSM_Default
            {
            public:
                INAP_FSM_WaitingForInstructions(const char *name, int stateId)
                : INAP_FSM_Default(name, stateId)
                {};

                void connectSMS(DialogContext& context);
                void continueSMS(DialogContext& context);
                void failure(DialogContext& context);
                void furnishChargingInformation(DialogContext& context);
                void releaseSMS(DialogContext& context);
                void requestReportSMSEvent(DialogContext& context);
                void resetTimer(DialogContext& context);
                void timerExpired(DialogContext& context);
            };

            class INAP_FSM_Monitoring :
                public INAP_FSM_Default
            {
            public:
                INAP_FSM_Monitoring(const char *name, int stateId)
                : INAP_FSM_Default(name, stateId)
                {};

                void done(DialogContext& context);
            };

            class DialogContext :
                public statemap::FSMContext
            {
            public:

                DialogContext(Dialog& owner)
                : _owner(owner)
                {
                    setState(TCAP_FSM::Idle);
                    TCAP_FSM::Idle.Entry(*this);
                };

                Dialog& getOwner() const
                {
                    return (_owner);
                };

                DialogState& getState() const
                {
                    if (_state == NULL)
                    {
                        throw statemap::StateUndefinedException();
                    }

                    return (dynamic_cast<DialogState&>(*_state));
                };

                void collectInfo()
                {
                    setTransition("collectInfo");
                    (getState()).collectInfo(*this);
                    setTransition(NULL);
                };

                void connectSMS()
                {
                    setTransition("connectSMS");
                    (getState()).connectSMS(*this);
                    setTransition(NULL);
                };

                void continueSMS()
                {
                    setTransition("continueSMS");
                    (getState()).continueSMS(*this);
                    setTransition(NULL);
                };

                void deliveryRequest()
                {
                    setTransition("deliveryRequest");
                    (getState()).deliveryRequest(*this);
                    setTransition(NULL);
                };

                void done()
                {
                    setTransition("done");
                    (getState()).done(*this);
                    setTransition(NULL);
                };

                void error()
                {
                    setTransition("error");
                    (getState()).error(*this);
                    setTransition(NULL);
                };

                void failure()
                {
                    setTransition("failure");
                    (getState()).failure(*this);
                    setTransition(NULL);
                };

                void furnishChargingInformation()
                {
                    setTransition("furnishChargingInformation");
                    (getState()).furnishChargingInformation(*this);
                    setTransition(NULL);
                };

                void releaseSMS()
                {
                    setTransition("releaseSMS");
                    (getState()).releaseSMS(*this);
                    setTransition(NULL);
                };

                void requestReportSMSEvent()
                {
                    setTransition("requestReportSMSEvent");
                    (getState()).requestReportSMSEvent(*this);
                    setTransition(NULL);
                };

                void resetTimer()
                {
                    setTransition("resetTimer");
                    (getState()).resetTimer(*this);
                    setTransition(NULL);
                };

                void timerExpired()
                {
                    setTransition("timerExpired");
                    (getState()).timerExpired(*this);
                    setTransition(NULL);
                };

            private:

                Dialog& _owner;
            };
        };

    };

};

#endif
