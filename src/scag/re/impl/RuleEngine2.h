#ifndef SCAG_RULE_IMPL_ENGINE2
#define SCAG_RULE_IMPL_ENGINE2

#include "scag/re/base/RuleEngine2.h"

#include "logger/Logger.h"
#include "core/buffers/XHash.hpp"
#include "core/synchronization/Mutex.hpp"
#include "scag/re/actions/impl/MainActionFactory2.h"
#include "scag/re/base/Rule2.h"
#include "scag/re/base/ActionContext2.h"
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
    virtual void updateRule(const RuleKey& key);
    virtual void removeRule(const RuleKey& key);
    void loadRuleFile( const RuleKey& key, const std::string& rulePath );
    virtual void process(SCAGCommand& command, Session& session, RuleStatus& rs, actions::CommandProperty& cp, util::HRTiming* hrt = 0);
    virtual void finalizeSession(Session& session, RuleStatus& rs);

    virtual Hash<TransportType>& getTransportTypeHash() {return TransportTypeHash;}
    virtual Hash<Property>& getConstants() { return ConstantsHash; };

private:
    typedef XHash< RuleKey, RulePtr, HashFunc > CRulesHash;

    struct Rules
    {
        friend class eyeline::informer::EmbedRefPtr< Rules >;
        CRulesHash      rules;

        Rules() : ref_(0) {}

    private:

        void ref()
        {
            MutexGuard mg(refLock_);
            ++ref_;
        }

        void unref()
        {
            {
                MutexGuard mg(refLock_);
                if (--ref_) return;
            }
            delete this;
        }

    private:
        Mutex           refLock_;
        unsigned        ref_;
    };

    typedef eyeline::informer::EmbedRefPtr< Rules > RulesReference;

    /*
    struct RulesReference
    {
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

        CRulesHash* operator->()
        {
            __require__(rules);
            return &(rules->rules);
        }

    private:
        RulesReference& operator = (const RulesReference&);
        Rules*  rules;
    };
     */

private:
    // RulesReference& operator = ( const RulesReference& );

    RulesReference getRules()
    {
        MutexGuard mg(rulesLock);
        return RulesReference(rules);
    }

    /// must be invoked only under changeLock
    void changeRules(Rules* _rules)
    {
        __require__(_rules);
        RulesReference rr(rules); // possible delete not under ruleslock
        {
            MutexGuard mg(rulesLock);
            rules.reset(_rules);
        }
    }

    /// must be invoked only under changeLock
    RulesReference copyReference()
    {
        RulesReference newRules( new Rules() );
        RulesReference oldRules = getRules();
        RuleKey oldKey;
        RulePtr rule;
        oldRules->rules.First();
        CRulesHash::Iterator it = oldRules->rules.getIterator();
        while (it.Next(oldKey, rule))
        {
            if (!rule.get()) continue;
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
    Mutex                       rulesLock;
    RulesReference              rules;
    Mutex   changeLock;
    Hash<Property> ConstantsHash;

    class AutoRuleTester;
    AutoRuleTester* autoRuleTester_;
};

} // namespace re
} // namespace scag2

#endif // SCAG_RULE_ENGINE
