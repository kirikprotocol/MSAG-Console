
#include "RuleEngine.h"

namespace scag { namespace re
{
using core::synchronization::MutexGuard;

/* 
Mutex           rulesLock;
IntHash<Rule>   rules;
IntHash<bool>   rulesInUse;
*/
RuleEngine::RuleEngine(ConfigView* config)
{
    /* TODO: Implement
             1) Configure global params (if any)
             2) Scan all sub-sections & create rules for it (via RulesFactory)
    */
}
RuleEngine::~RuleEngine()
{
    // TODO: delete rules
}

void RuleEngine::addRule(int ruleId, ConfigView* config)
{
    /* TODO: Implement
             1) Check ruleId exist?
             2) Create new rule for config section (via RulesFactory)
    */
}
void RuleEngine::modifyRule(int ruleId, ConfigView* config)
{
    /* TODO: Implement
            1) Check ruleId not exist?
            2) Create new rule for config section (via RulesFactory)
            3) Wait while this rule be unlocked in process (check rulesInUse)
            4) Replace rule
    */
}
void RuleEngine::removeRule(int ruleId)
{
    /* TODO: Implement
            1) Check ruleId not exist?
            2) Wait while this rule be unlocked in process (check rulesInUse)
            3) Delete rule
    */
}

RuleStatus RuleEngine::process(int ruleId, SCAGCommand command)
{
    /* TODO: Implement
             1) Find rule by id and lock for modify (add/inc rulesInUse)
             2) Call process on rule
             3) Unlock rule & return status from rule
    */
    
    RuleStatus status;
    return status;
}


}}
