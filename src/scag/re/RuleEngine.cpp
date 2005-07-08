
#include "RuleEngine.h"

namespace scag { namespace re
{
using core::synchronization::MutexGuard;

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


}}
