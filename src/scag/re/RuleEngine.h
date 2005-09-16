#ifndef SCAG_RULE_ENGINE
#define SCAG_RULE_ENGINE

#include <core/synchronization/Mutex.hpp>
#include <core/buffers/IntHash.hpp>

#include <util/debug.h>


#include "scag/re/Rule.h"
#include "scag/re/actions/MainActionFactory.h"

#include "scag/SAX2Print.hpp"

namespace scag { namespace re 
{
    using smsc::core::synchronization::Mutex;
    using smsc::core::buffers::IntHash;
    using namespace scag::re::actions;

struct RulesReference;
struct Rules;




class RuleEngine
{


private:
    MainActionFactory factory;
    std::string RulesDir;
    int GetRuleId(SCAGCommand& command);

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

    Rule * ParseFile(const std::string& xmlFile);
    bool isValidFileName(std::string fname,int& ruleId);
    std::string CreateRuleFileName(const std::string& dir,const int ruleId) const;

public:

    RuleEngine(const std::string& dir); 
    ~RuleEngine();

        /**
         * Processes specified command on rule difined by id (obtained in RouteInfo).
         * @param   ruleId      rule to run
         * @param   command     command to process
         * @return  status      command processing status
         */
    RuleStatus process(SCAGCommand& command,Session& session);
    void updateRule(int ruleId);
    bool removeRule(int ruleId);
    const ActionFactory& getActionFactory() {return factory;}

};

}}
#endif // SCAG_RULE_ENGINE

