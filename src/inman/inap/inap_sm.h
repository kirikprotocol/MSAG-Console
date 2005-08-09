#ifndef _H_INAP_SM
#define _H_INAP_SM

#define SMC_USES_IOSTREAMS

#include "statemap.h"

namespace smsc
{
    namespace inman
    {
        namespace inap
        {
            // Forward declarations.
            class INAP_FSM;
            class INAP_FSM_Idle;
            class INAP_FSM_InittialDpInvoked;
            class INAP_FSM_WaitForInstructions;
            class INAP_FSM_EventReportInvoked;
            class INAP_FSM_Default;
            class INAPState;
            class INAPContext;
            class INAP;

            class INAPState :
                public statemap::State
            {
            public:

                INAPState(const char *name, int stateId)
                : statemap::State(name, stateId)
                {};

                virtual void Entry(INAPContext&) {};
                virtual void Exit(INAPContext&) {};

                virtual void continueSMS(INAPContext& context);
                virtual void failed(INAPContext& context, const TcapOperation& error);
                virtual void releaseSMS(INAPContext& context);
                virtual void requestReportEvent(INAPContext& context);
                virtual void start(INAPContext& context);
                virtual void successed(INAPContext& context, const TcapOperation& result);

            protected:

                virtual void Default(INAPContext& context);
            };

            class INAP_FSM
            {
            public:

                static INAP_FSM_Idle Idle;
                static INAP_FSM_InittialDpInvoked InittialDpInvoked;
                static INAP_FSM_WaitForInstructions WaitForInstructions;
                static INAP_FSM_EventReportInvoked EventReportInvoked;
            };

            class INAP_FSM_Default :
                public INAPState
            {
            public:

                INAP_FSM_Default(const char *name, int stateId)
                : INAPState(name, stateId)
                {};

            };

            class INAP_FSM_Idle :
                public INAP_FSM_Default
            {
            public:
                INAP_FSM_Idle(const char *name, int stateId)
                : INAP_FSM_Default(name, stateId)
                {};

                void start(INAPContext& context);
            };

            class INAP_FSM_InittialDpInvoked :
                public INAP_FSM_Default
            {
            public:
                INAP_FSM_InittialDpInvoked(const char *name, int stateId)
                : INAP_FSM_Default(name, stateId)
                {};

                void failed(INAPContext& context, const TcapOperation& error);
                void successed(INAPContext& context, const TcapOperation& result);
            };

            class INAP_FSM_WaitForInstructions :
                public INAP_FSM_Default
            {
            public:
                INAP_FSM_WaitForInstructions(const char *name, int stateId)
                : INAP_FSM_Default(name, stateId)
                {};

                void continueSMS(INAPContext& context);
                void releaseSMS(INAPContext& context);
                void requestReportEvent(INAPContext& context);
            };

            class INAP_FSM_EventReportInvoked :
                public INAP_FSM_Default
            {
            public:
                INAP_FSM_EventReportInvoked(const char *name, int stateId)
                : INAP_FSM_Default(name, stateId)
                {};

                void failed(INAPContext& context, const TcapOperation& error);
                void successed(INAPContext& context, const TcapOperation& result);
            };

            class INAPContext :
                public statemap::FSMContext
            {
            public:

                INAPContext(INAP& owner)
                : _owner(owner)
                {
                    setState(INAP_FSM::Idle);
                    INAP_FSM::Idle.Entry(*this);
                };

                INAP& getOwner() const
                {
                    return (_owner);
                };

                INAPState& getState() const
                {
                    if (_state == NULL)
                    {
                        throw statemap::StateUndefinedException();
                    }

                    return (dynamic_cast<INAPState&>(*_state));
                };

                void continueSMS()
                {
                    setTransition("continueSMS");
                    (getState()).continueSMS(*this);
                    setTransition(NULL);
                };

                void failed(const TcapOperation& error)
                {
                    setTransition("failed");
                    (getState()).failed(*this, error);
                    setTransition(NULL);
                };

                void releaseSMS()
                {
                    setTransition("releaseSMS");
                    (getState()).releaseSMS(*this);
                    setTransition(NULL);
                };

                void requestReportEvent()
                {
                    setTransition("requestReportEvent");
                    (getState()).requestReportEvent(*this);
                    setTransition(NULL);
                };

                void start()
                {
                    setTransition("start");
                    (getState()).start(*this);
                    setTransition(NULL);
                };

                void successed(const TcapOperation& result)
                {
                    setTransition("successed");
                    (getState()).successed(*this, result);
                    setTransition(NULL);
                };

            private:

                INAP& _owner;
            };
        };

    };

};

#endif
