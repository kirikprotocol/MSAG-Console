#ifndef SCAG_RULE_IMPL_ENGINE2
#define SCAG_RULE_IMPL_ENGINE2

#include "scag/re/base/RuleEngine2.h"

#include "logger/Logger.h"
#include "core/buffers/XHash.hpp"
#include "core/synchronization/Mutex.hpp"
#include "scag/re/actions/impl/MainActionFactory2.h"
#include "scag/re/base/Rule2.h"
// #include "scag/re/base/TransportRule.h"
#include "scag/re/Constants.h"

namespace scag2 {
namespace re {

namespace Const = scag::re::Const;

struct HashFunc
{
static unsigned int CalcHash(const RuleKey& key)
{
    return key.serviceId + key.transport;
}
};

using namespace smsc::core::buffers;
using namespace smsc::core::synchronization;

class RuleEngineImpl : public RuleEngine
{

public:

    RuleEngineImpl();
    virtual ~RuleEngineImpl();

    void init( const std::string& dir );

    virtual ActionFactory& getActionFactory() {return factory;}
    virtual void updateRule(RuleKey& key);
    virtual void removeRule(RuleKey& key);
    virtual void process(SCAGCommand& command, Session& session, RuleStatus& rs, util::HRTimer* hrt = 0);
    virtual void processSession(Session& session, RuleStatus& rs);

    virtual Hash<TransportType> getTransportTypeHash() {return TransportTypeHash;}
    virtual Hash<Property>& getConstants() { return ConstantsHash; };

//    virtual bool findTransport(const char * name, TransportType& transportType);


private:
    typedef XHash< RuleKey, Rule*, HashFunc > CRulesHash;

    struct Rules
    {
        Mutex           rulesLock;
        CRulesHash      rules;
        int             useCounter;

        Rules() : useCounter(1) {}
        ~Rules()
         {
             rules.First();

             CRulesHash::Iterator it = rules.getIterator();
             RuleKey key;
             Rule* rule = 0;
             while (it.Next(key, rule))
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

        CRulesHash& operator->()
        {
            __require__(rules);
            return rules->rules;
        }
    };

private:

    // TransportRule* getTransportRule( TransportType tt ) const;


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

    Rules* copyReference()
    {
        Rules* newRules = new Rules();
        rules->rules.First();

        CRulesHash::Iterator it = rules->rules.getIterator();
        RuleKey oldKey; Rule* rule = 0;
        while (it.Next(oldKey, rule))
        {
            if (!rule) continue;
            rule->ref();
            newRules->rules.Insert(oldKey, rule);
        }

        return newRules;
    }

    Rule * ParseFile(const std::string& xmlFile);
    bool isValidFileName(std::string fname,int& ruleId);
    std::string CreateRuleFileName(const std::string& dir,const RuleKey& key);

    void ReadRulesFromDir(TransportType transport, const char * dir);


private:
    static const unsigned maxrule = 3;

private:
    actions::MainActionFactory  factory;
    std::string                 RulesDir;
    smsc::logger::Logger*       logger;
    Hash<TransportType>         TransportTypeHash;
    Mutex  rulesLock;
    Rules* rules;
    // mutable std::auto_ptr< TransportRule > transportRules_[maxrule];
    Mutex   changeLock;
    Hash<Property> ConstantsHash;
};

} // namespace re
} // namespace scag2

#endif // SCAG_RULE_ENGINE
