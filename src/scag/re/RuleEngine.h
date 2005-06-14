#ifndef SCAG_RULE_ENGINE
#define SCAG_RULE_ENGINE

#include <core/synchronization/Mutex.hpp>
#include <core/buffers/IntHash.hpp>

#include "Rules.h"

namespace scag { namespace re 
{
    using core::synchronization::Mutex;
    using core::buffers::IntHash;
    
    /**
     * Should be instantiated from SCAG Core on startup
     * Allows to manage rules in runtime (add/remove/modify)
     */
    class RuleEngine
    {
    private:

        Mutex           rulesLock;
        IntHash<Rule*>  rules;
        IntHash<int>    rulesInUse;

    public:

        //TODO: Implement

        /**
         * Creates RuleEngine by config subsection
         * @param   config      RuleEngine config subsection
         */
        RuleEngine(ConfigView* config);
        ~RuleEngine();

        /**
         * Processes specified command on rule difined by id (obtained in RouteInfo).
         * @param   ruleId      rule to run
         * @param   command     command to process
         * @return  status      command processing status
         */
        RuleStatus process(int ruleId, SCAGCommand command);

        void addRule   (int ruleId, ConfigView* config);
        void modifyRule(int ruleId, ConfigView* config);
        void removeRule(int ruleId);
    };

}}}
#endif // SCAG_RULE_ENGINE

