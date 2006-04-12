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


struct RuleKey
{
    scag::transport::TransportType transport;
    int serviceId;

    bool operator == (const RuleKey& key)
    {
        return serviceId==key.serviceId && transport==key.transport;
    }
    RuleKey() : serviceId(-1) {};
};


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
    virtual void updateRule(RuleKey& key) = 0;
    virtual void removeRule(RuleKey& key) = 0;
    virtual ActionFactory& getActionFactory() = 0;
    virtual Hash<TransportType> getTransportTypeHash() = 0;
//    virtual bool findTransport(const char * name, TransportType& transportType) = 0;

};

}}
#endif // SCAG_RULE_ENGINE
