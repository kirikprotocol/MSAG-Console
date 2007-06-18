#ifndef SCAG_RULE_KEY
#define SCAG_RULE_KEY

namespace scag { namespace re
{

struct RuleKey
{
    int transport;
    int serviceId;

    bool operator == (const RuleKey& key)
    {
        return serviceId==key.serviceId && transport==key.transport;
    }
    RuleKey() : serviceId(-1) {};
};

}}
#endif // SCAG_RULE_ENGINE
