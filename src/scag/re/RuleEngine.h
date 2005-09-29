#ifndef SCAG_RULE_ENGINE
#define SCAG_RULE_ENGINE

#include "scag/re/Rule.h"

namespace scag { namespace re 
{



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


        /**
         * Processes specified command on rule difined by id (obtained in RouteInfo).
         * @param   ruleId      rule to run
         * @param   command     command to process
         * @return  status      command processing status
         */
    virtual RuleStatus process(SCAGCommand& command,Session& session) = 0;
    virtual void updateRule(int ruleId) = 0;
    virtual bool removeRule(int ruleId) = 0;
    virtual ActionFactory& getActionFactory() = 0;

    
};

}}
#endif // SCAG_RULE_ENGINE

