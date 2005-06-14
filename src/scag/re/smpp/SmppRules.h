#ifndef SCAG_RULE_ENGINE_SMPP_RULES
#define SCAG_RULE_ENGINE_SMPP_RULES

#include <scag/re/Rules.h>

namespace scag { namespace re { namespace smpp 
{
    using namespace scag::re::rules;

    class SmppEventHandler : public EventHandler
    {
    private:


    public:

        SmppEventHandler() : EventHandler() {};
        virtual ~SmppEventHandler() {};
    };

    class SmppRule : public Rule
    {
    
    public:

        SmppRule() : Rule() {};
        virtual ~SmppRule() {};
        
        // TODO: implement methods
        virtual void init(ConfigView *config);
        virtual RuleStatus process(SCAGCommand command);
    };

}}}

#endif // SCAG_RULE_ENGINE_SMPP_RULES

