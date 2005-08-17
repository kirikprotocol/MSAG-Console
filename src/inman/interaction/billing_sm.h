#ifndef _H_BILLING_SM
#define _H_BILLING_SM

#define SMC_USES_IOSTREAMS

#include "inman/common/statemap.h"

namespace smsc
{
    namespace inman
    {
        namespace interaction
        {
            // Forward declarations.
            class BILLING;
            class BILLING_IDLE;
            class BILLING_WAIT_FOR_INSTRUCTIONS;
            class BILLING_MONITORING;
            class BILLING_Default;
            class BillingInteractionState;
            class BillingInteractionContext;
            class BillingInteraction;

            class BillingInteractionState :
                public statemap::State
            {
            public:

                BillingInteractionState(const char *name, int stateId)
                : statemap::State(name, stateId)
                {};

                virtual void Entry(BillingInteractionContext&) {};
                virtual void Exit(BillingInteractionContext&) {};


            protected:

                virtual void Default(BillingInteractionContext& context);
            };

            class BILLING
            {
            public:

                static BILLING_IDLE IDLE;
                static BILLING_WAIT_FOR_INSTRUCTIONS WAIT_FOR_INSTRUCTIONS;
                static BILLING_MONITORING MONITORING;
            };

            class BILLING_Default :
                public BillingInteractionState
            {
            public:

                BILLING_Default(const char *name, int stateId)
                : BillingInteractionState(name, stateId)
                {};

            };

            class BILLING_IDLE :
                public BILLING_Default
            {
            public:
                BILLING_IDLE(const char *name, int stateId)
                : BILLING_Default(name, stateId)
                {};

            };

            class BILLING_WAIT_FOR_INSTRUCTIONS :
                public BILLING_Default
            {
            public:
                BILLING_WAIT_FOR_INSTRUCTIONS(const char *name, int stateId)
                : BILLING_Default(name, stateId)
                {};

            };

            class BILLING_MONITORING :
                public BILLING_Default
            {
            public:
                BILLING_MONITORING(const char *name, int stateId)
                : BILLING_Default(name, stateId)
                {};

            };

            class BillingInteractionContext :
                public statemap::FSMContext
            {
            public:

                BillingInteractionContext(BillingInteraction& owner)
                : _owner(owner)
                {
                    setState(BILLING::IDLE);
                    BILLING::IDLE.Entry(*this);
                };

                BillingInteraction& getOwner() const
                {
                    return (_owner);
                };

                BillingInteractionState& getState() const
                {
                    if (_state == NULL)
                    {
                        throw statemap::StateUndefinedException();
                    }

                    return (dynamic_cast<BillingInteractionState&>(*_state));
                };

            private:

                BillingInteraction& _owner;
            };
        };

    };

};

#endif
