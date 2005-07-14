#ifndef SCAG_RULE_ENGINE
#define SCAG_RULE_ENGINE

#include <core/synchronization/Mutex.hpp>
#include <core/buffers/IntHash.hpp>

#include <util/config/ConfigView.h>
//#include <scag/admin/SCAGCommand.h>

#include "scag/re/actions/Action.h"
#include "scag/re/actions/ActionChoose.h"
#include "scag/re/Rule.h"

namespace scag { namespace re 
{
    using smsc::core::synchronization::Mutex;
    using smsc::core::buffers::IntHash;
    using namespace scag::re::actions;
    using smsc::util::config::ConfigView;
//    using smsc::scag::admin::SCAGCommand;

/*
class ConfigView 
{
}; */
        
class SCAGCommand
{
};    

    /**
     * Should be instantiated from SCAG Core on startup
     * Allows to manage rules in runtime (add/remove/modify)
     */

struct RulesReference;
struct Rules;




class RuleEngine
{


private:
    ActionFactory factory;

    friend struct RulesReference; 
    struct Rules
    {
        Mutex           rulesLock;
        IntHash<Rule*>  rules;
        int             useCounter;

        Rules() : useCounter(1) {}
        ~Rules() 
         {
             IntHash<Rule*>::Iterator it = rules.First();
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

        RulesReference(Rules* _rules) : rules(_rules) 
        {
            __require__(rules);
            rules->ref();
        };

        RulesReference(const RulesReference& rr) : rules(rr.rules) 
        {
            __require__(rules);
            rules->ref();
        }

        ~RulesReference() 
        {
            __require__(rules);
            rules->unref();
        }

        IntHash<Rule*>& operator->() 
        {
            __require__(rules);
            return rules->rules;
        }
    };
        
    Mutex  rulesLock;
    Rules* rules;

    RulesReference getRules() 
    {
	MutexGuard mg(rulesLock);
	return RulesReference(rules);
    }

    void changeRules(Rules* _rules) 
    {
        MutexGuard mg(rulesLock);
        __require__(_rules);
        rules->unref();
        rules = _rules;
    }

    Mutex   changeLock; 

    Rules* copyReference()
    {
        Rules* newRules = new Rules();
        IntHash<Rule*>::Iterator it = rules->rules.First();
        int oldRuleId; Rule* rule = 0;
        while (it.Next(oldRuleId, rule))
        {
            if (!rule) continue;
            rule->ref();
            newRules->rules.Insert(oldRuleId, rule);
        }
             
        return newRules;
    }

    int ParseFile(const std::string& xmlFile);


public:

    RuleEngine(const std::string& dir); 
    ~RuleEngine();

        /**
         * Processes specified command on rule difined by id (obtained in RouteInfo).
         * @param   ruleId      rule to run
         * @param   command     command to process
         * @return  status      command processing status
         */
    RuleStatus process(int ruleId, SCAGCommand command)
    {
        RulesReference rulesRef = getRules();
        if (rulesRef.rules->rules.Exist(ruleId)) 
        {
                // TODO: Implement ...
        }
	RuleStatus rs;
	return rs;
    }

    void updateRule(int ruleId) // add or modify
    {
        MutexGuard mg(changeLock);

        Rule* newRule = 0; // to create

        Rules *newRules = copyReference();
        Rule** rulePtr = newRules->rules.GetPtr(ruleId);
        if (rulePtr) 
        {
            (*rulePtr)->unref();
            newRules->rules.Delete(ruleId);
        }
        newRules->rules.Insert(ruleId, newRule);
        changeRules(newRules);
    }

    bool removeRule(int ruleId)
    {
        MutexGuard mg(changeLock);

        Rule** rulePtr = rules->rules.GetPtr(ruleId);  // Can we do such direct access? TODO: Ensure
        if (!rulePtr) return false;
            
        Rules *newRules = copyReference();
        rulePtr = newRules->rules.GetPtr(ruleId);
        (*rulePtr)->unref();
        newRules->rules.Delete(ruleId);
        changeRules(newRules);
        return true;
    }
};

}}
#endif // SCAG_RULE_ENGINE

