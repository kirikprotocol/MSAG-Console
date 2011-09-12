#ifndef SCAG_RULE_ENGINE2
#define SCAG_RULE_ENGINE2

//#include <scag/sessions/Session.h>
//#include <scag/re/actions/ActionFactory.h>
#include "scag/transport/SCAGCommand2.h"
#include "RuleStatus2.h"
#include "ActionContext2.h"
#include "RuleKey2.h"
#include "scag/util/properties/Properties2.h"

namespace scag2 {

namespace sessions {
    class Session;
}

namespace util {
    class HRTiming;
}

namespace re {

using namespace util::properties;

using namespace transport;
using namespace sessions;

class RuleEngine
{
public:

    // static void Init(const std::string& dir);
    static RuleEngine& Instance();

    virtual void process(SCAGCommand& command, Session& session, RuleStatus& rs, actions::CommandProperty& cp, util::HRTiming* hrt = 0 ) = 0;

    /// this method is invoked (directly) for session destruction only!
    virtual void finalizeSession(Session& session, RuleStatus& rs) = 0;

    virtual void updateRule(const RuleKey& key) = 0;
    virtual void removeRule(const RuleKey& key) = 0;
//    virtual ActionFactory& getActionFactory() = 0;
    virtual smsc::core::buffers::Hash<TransportType>& getTransportTypeHash() = 0;
//    virtual bool findTransport(const char * name, TransportType& transportType) = 0;
    virtual smsc::core::buffers::Hash<Property>& getConstants() = 0;

    virtual ~RuleEngine();

protected:
    RuleEngine();

private:
    RuleEngine(const RuleEngine& re);
    RuleEngine& operator=(const RuleEngine& re);
};

}}
#endif // SCAG_RULE_ENGINE
