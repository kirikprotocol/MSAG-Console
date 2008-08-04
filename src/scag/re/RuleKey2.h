#ifndef SCAG_RULE_KEY2
#define SCAG_RULE_KEY2

namespace scag2 {
namespace re {

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
