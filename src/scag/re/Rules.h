#ifndef SCAG_RULES
#define SCAG_RULES

#include <string>

#include <core/synchronization/Mutex.hpp>
#include <core/buffers/IntHash.hpp>
#include <core/buffers/Array.hpp>

#include <util/config/ConfigView.h>
#include <util/config/ConfigException.h>

//#include "SCAGCommand.h" TODO: where ???

#include "actions/Action.h"

namespace scag { namespace re 
{
    using core::buffers::Array;
    using core::buffers::IntHash;
    using core::synchronization::Mutex;

    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;

    using scag::re::actions::Action;
    
    struct RuleStatus
    {
        int status; // TODO: to be extended
    };

    /**
     * Abstract class, should be implemented for each transport
     */
    class EventHandler
    {
    protected:

        Array<Action*> actions;

    public:
        
        EventHandler() : action(0) {};
        virtual ~EventHandler() {
            if (action) delete action;
        };
        
        /**
         * Creates & configure action(s) from sub-section (via ActionsFactory)
         * @param   config      config sub-section for handler
         */
        virtual void init(ConfigView* config) = 0;

        /** 
         * Processes action (or actions set).
         * Creates ActionContext with transport specific CommandAdapter.
         * Returns RuleStatus from context after action(s) execution.
         * @param   command     command to process
         * @return  status      action(s) execution status
         */
        virtual RuleStatus process(SCAGCommand command) = 0;
    };

    /**
     * Abstract class, should be implemented for each transport
     */
    class Rule
    {
    private:

        Mutex ruleLock;
        int useCounter;

    protected:

        // TODO: add more rule attributes (traffic control etc.)

        std::string billing_id;
        IntHash<EventHandler> handlers;
        
    public:

        Rule() : useCounter(1) {};
        virtual ~Rule() {};

        void ref() {
            MutexGuard mg(ruleLock);
            useCounter++;
        }
        void unref() 
        {
            bool del = false;
            {
                MutexGuard mg(ruleLock);
                del = (--useCounter == 0);
            }
            if (del) delete this;
        }
        
        /**
         * Creates & configure rule attributes and
         * handlers from sub-section (via HandlersFactory ?)
         * @param   config      config sub-section for rule
         */
        virtual void init(ConfigView* config) = 0;


        /** 
         * Processes command via one of rules's handler.
         * Searches handler by command id/type and
         * returns RuleStatus after handler execution.
         * @param   command     command to process
         * @return  status      rule's handler execution status
         */
        virtual RuleStatus process(SCAGCommand command) = 0;
    };
    
}}}
#endif // SCAG_RULES

