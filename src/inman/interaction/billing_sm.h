#ifndef _H_BILLING_SM
#define _H_BILLING_SM

#define SMC_USES_IOSTREAMS

#include "statemap.hpp"

namespace smsc
{
    namespace inman
    {
        namespace interaction
        {
            // Forward declarations.
            class BILLING;
            class BILLING_IDLE;
            class BILLING_WAITING_FOR_INSTRUCTIONS;
            class BILLING_MONITORING;
            class BILLING_Default;
            class BillingState;
            class BillingContext;
            class Billing;

            class BillingState :
                public statemap::State
            {
            public:

                BillingState(const char *name, int stateId)
                : statemap::State(name, stateId)
                {};

                virtual void Entry(BillingContext&) {};
                virtual void Exit(BillingContext&) {};

                virtual void connectSMS(BillingContext& context);
                virtual void continueSMS(BillingContext& context);
                virtual void furnishChargingInformationSMS(BillingContext& context);
                virtual void releaseSMS(BillingContext& context);
                virtual void requestReportSMSEvent(BillingContext& context, RequestReportSMSEventArg* arg);
                virtual void resetTimerSMS(BillingContext& context);
                virtual void smsRejected(BillingContext& context);
                virtual void smsSent(BillingContext& context);
                virtual void start(BillingContext& context);

            protected:

                virtual void Default(BillingContext& context);
            };

            class BILLING
            {
            public:

                static BILLING_IDLE IDLE;
                static BILLING_WAITING_FOR_INSTRUCTIONS WAITING_FOR_INSTRUCTIONS;
                static BILLING_MONITORING MONITORING;
            };

            class BILLING_Default :
                public BillingState
            {
            public:

                BILLING_Default(const char *name, int stateId)
                : BillingState(name, stateId)
                {};

            };

            class BILLING_IDLE :
                public BILLING_Default
            {
            public:
                BILLING_IDLE(const char *name, int stateId)
                : BILLING_Default(name, stateId)
                {};

                void start(BillingContext& context);
            };

            class BILLING_WAITING_FOR_INSTRUCTIONS :
                public BILLING_Default
            {
            public:
                BILLING_WAITING_FOR_INSTRUCTIONS(const char *name, int stateId)
                : BILLING_Default(name, stateId)
                {};

                void connectSMS(BillingContext& context);
                void continueSMS(BillingContext& context);
                void furnishChargingInformationSMS(BillingContext& context);
                void releaseSMS(BillingContext& context);
                void requestReportSMSEvent(BillingContext& context, RequestReportSMSEventArg* arg);
                void resetTimerSMS(BillingContext& context);
            };

            class BILLING_MONITORING :
                public BILLING_Default
            {
            public:
                BILLING_MONITORING(const char *name, int stateId)
                : BILLING_Default(name, stateId)
                {};

                void smsRejected(BillingContext& context);
                void smsSent(BillingContext& context);
            };

            class BillingContext :
                public statemap::FSMContext
            {
            public:

                BillingContext(Billing& owner)
                : _owner(owner)
                {
                    setState(BILLING::IDLE);
                    BILLING::IDLE.Entry(*this);
                };

                Billing& getOwner() const
                {
                    return (_owner);
                };

                BillingState& getState() const
                {
                    if (_state == NULL)
                    {
                        throw statemap::StateUndefinedException();
                    }

                    return (dynamic_cast<BillingState&>(*_state));
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

                void furnishChargingInformationSMS()
                {
                    setTransition("furnishChargingInformationSMS");
                    (getState()).furnishChargingInformationSMS(*this);
                    setTransition(NULL);
                };

                void releaseSMS()
                {
                    setTransition("releaseSMS");
                    (getState()).releaseSMS(*this);
                    setTransition(NULL);
                };

                void requestReportSMSEvent(RequestReportSMSEventArg* arg)
                {
                    setTransition("requestReportSMSEvent");
                    (getState()).requestReportSMSEvent(*this, arg);
                    setTransition(NULL);
                };

                void resetTimerSMS()
                {
                    setTransition("resetTimerSMS");
                    (getState()).resetTimerSMS(*this);
                    setTransition(NULL);
                };

                void smsRejected()
                {
                    setTransition("smsRejected");
                    (getState()).smsRejected(*this);
                    setTransition(NULL);
                };

                void smsSent()
                {
                    setTransition("smsSent");
                    (getState()).smsSent(*this);
                    setTransition(NULL);
                };

                void start()
                {
                    setTransition("start");
                    (getState()).start(*this);
                    setTransition(NULL);
                };

            private:

                Billing& _owner;
            };
        };

    };

};

#endif
