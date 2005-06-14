#ifndef SCAG_RULE_ENGINE_ACTION_CONTEXT
#define SCAG_RULE_ENGINE_ACTION_CONTEXT

#include <string>

#include <core/buffers/Hash.hpp>

#include <scag/re/RuleStatus.h>
#include <scag/util/properties/Properties.h>

namespace scag { namespace re { namespace actions 
{
    using core::buffers::Hash;
    using scag::re::RuleStatus;

    using namespace scag::util::properties;

    class ActionContext
    {
    private:

        RuleStatus              status;

        Hash<Property>          variables;
        const Hash<Property>&   constants;

        PropertyManager&        session;
        PropertyManager&        command;
    
    public:

        // TODO: implement, set commandAdapter & session
        ActionContext(const Hash<Property>& _constants,
                      PropertyManager& _session, PropertyManager& _command)
            : constants(_constants), session(_session), command(_command) {};
        ~ActionContext() {};
        
        inline RuleStatus& getStatus() {
            return status;
        }
        
        /** TODO: Implement access to command, session, local variables & constants
         *        define scope by var name prefix: #, $, %, @ 
         */

        Property* getProperty(const std::string& var);
    };

}}}

#endif // SCAG_RULE_ENGINE_ACTION_CONTEXT

