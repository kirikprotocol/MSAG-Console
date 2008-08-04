#ifndef SCAG_RULE_ENGINE2
#define SCAG_RULE_ENGINE2

//#include <scag/sessions/Session.h>
//#include <scag/re/actions/ActionFactory.h>
#include "scag/transport/SCAGCommand2.h"
#include "RuleStatus2.h"
#include "RuleKey2.h"
#include "scag/util/properties/Properties.h"

namespace scag2 {
namespace sessions {
    class Session;
}}

namespace scag2 {
namespace re {

using namespace scag::util::properties;

using namespace transport;
using namespace sessions;

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
