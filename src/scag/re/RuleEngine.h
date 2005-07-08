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

        friend struct RuleEngine::RulesReference; 
        struct Rules
        {
            Mutex           rulesLock;
            IntHash<Rule*>  rules;
            int             useCounter;

            Rules() : useCounter(1) {}
            ~Rules() 
            {
                IntHash<Rules*>::Iterator it = rules->First();
                int ruleId; Rule* rule = 0;
                while (it.Next(ruleId, rule))
                {
                    if (!rule) continue;
                    rule->unref();
                }
            }

            void ref() 
            {
                MutexGuard mg(rulesLock);
                useCounter++;
            }
            void unref() 
            {
                bool del=false;
                {
                    MutexGuard mg(rulesLock);
                    del = (--useCounter == 0);
                }
                if (del) delete this;
            }
        };

        struct RulesReference
        {
            Rules*  rules;

            RulesReference(Rules* _rules) : rules(_rules) {
                __require__(rules);
                rules->ref();
            };
            RulesReference(const RulesReference& rr) : rules(rr.rules) {
                __require__(rules);
                rules->ref();
            }
            ~RulesReference() {
                __require__(rules);
                rules->unref();
            }

            IntHash<Rule*>& operator->() {
                __require__(rules);
                return rules->rules;
            }
        };
        
        Mutex  rulesLock;
        Rules* rules;

        RulesReference getRules() {
            MutexGuard mg(rulesLock);
            return RulesReference(rules);
        }

        void changeRules(Rules* _rules) {
            MutexGuard mg(rulesLock);
            __require__(_rules);
            rules->unref();
            rules = _rules;
        }

        Mutex   changeLock; 

        Rules* copyReference()
        {
            Rules* newRules = new Rules();
            IntHash<Rules*>::Iterator it = rules->First();
            int oldRuleId; Rule* rule = 0;
            while (it.Next(oldRuleId, rule))
            {
                if (!rule) continue;
                rule->ref();
                newRules->rules.Insert(oldRuleId, rule);
            }
             
            return newRules;
        }

    public:

        //TODO: Implement

        /**
         * Creates RuleEngine by config subsection
         * @param   config      RuleEngine config subsection
         */
        RuleEngine(ConfigView* config); // TODO: new Rules();
        ~RuleEngine()
	{
	    if (rules) rules->unref();
	}

        /**
         * Processes specified command on rule difined by id (obtained in RouteInfo).
         * @param   ruleId      rule to run
         * @param   command     command to process
         * @return  status      command processing status
         */
        RuleStatus process(int ruleId, SCAGCommand command)
        {
            RulesReference rulesRef = getRules();
            if (rulesRef->Exists(ruleId)) {
                // TODO: Implement ...
            }
        }

        void updateRule(int ruleId, ConfigView* config) // add or modify
        {
            MutexGuard mg(changeLock);

            Rule* newRule = 0; // to create

            Rules *newRules = copyReference();
            Rule** rulePtr = newRules->GetPtr(ruleId);
            if (rulePtr) {
                (*rulePtr)->unref();
                newRules->Delete(ruleId);
            }
            newRules->Insert(ruleId, newRule);
            changeRules(newRules);
        }
        bool removeRule(int ruleId)
        {
            MutexGuard mg(changeLock);

            Rule** rulePtr = rules->GetPtr(ruleId);  // Can we do such direct access? TODO: Ensure
            if (!rulePtr) return false;
            
            Rules *newRules = copyReference();
            rulePtr = newRules->GetPtr(ruleId);
            (*rulePtr)->unref();
            newRules->Delete(ruleId);
            changeRules(newRules);
            return true;
        }
    };

}}}
#endif // SCAG_RULE_ENGINE

