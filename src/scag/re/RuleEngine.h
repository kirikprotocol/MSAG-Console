#ifndef SCAG_RULE_ENGINE
#define SCAG_RULE_ENGINE

//#include <scag/sessions/Session.h>
//#include <scag/re/actions/ActionFactory.h>
#include "scag/transport/SCAGCommand.h"
#include "RuleStatus.h"
#include "RuleKey.h"
#include "scag/sessions/Session.h"

namespace scag { namespace re
{

using namespace scag::transport;
using namespace scag::sessions;

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

    virtual void process(SCAGCommand& command, Session& session, RuleStatus& rs) = 0;
    virtual void processSession(Session& session, RuleStatus& rs) = 0;
    virtual void updateRule(RuleKey& key) = 0;
    virtual void removeRule(RuleKey& key) = 0;
//    virtual ActionFactory& getActionFactory() = 0;
    virtual Hash<TransportType> getTransportTypeHash() = 0;
//    virtual bool findTransport(const char * name, TransportType& transportType) = 0;
    virtual Hash<Property>& getConstants() = 0;

};

}}
#endif // SCAG_RULE_ENGINE
