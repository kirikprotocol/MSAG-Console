#ifndef SCAG_RULE_ENGINE
#define SCAG_RULE_ENGINE

#include <scag/sessions/Session.h>
#include <scag/re/actions/ActionFactory.h>
#include "RuleStatus.h"

namespace scag { namespace re
{

using namespace scag::transport;
using namespace scag::sessions;
using namespace scag::re::actions;


class RuleEngine
{
    RuleEngine(const RuleEngine& re);
    RuleEngine& operator=(const RuleEngine& re);

protected:

    RuleEngine() {};
    virtual ~RuleEngine() {};

public:

    static void Init(const std::string& dir);
    static RuleEngine& Instance();

    virtual RuleStatus process(SCAGCommand& command, Session& session) = 0;
    virtual void updateRule(int ruleId) = 0;
    virtual void removeRule(int ruleId) = 0;
    virtual ActionFactory& getActionFactory() = 0;

};

}}
#endif // SCAG_RULE_ENGINE
