#ifndef SCAG_RULE_ENGINE_ACTION_CONTEXT
#define SCAG_RULE_ENGINE_ACTION_CONTEXT

#include <string>
#include <core/buffers/Hash.hpp>
#include <scag/re/RuleStatus.h>
#include <scag/util/properties/Properties.h>
#include "scag/stat/Statistics.h"
#include "scag/sessions/Session.h"


namespace scag { namespace re { namespace actions 
{
    using smsc::core::buffers::Hash;
    using scag::re::RuleStatus;
    using namespace scag::stat;
    using namespace scag::sessions;

    using namespace scag::util::properties;

    enum FieldType
    {
        ftUnknown,
        ftLocal =   '%',
        ftConst =   '@',
        ftSession = '$',
        ftField =   '#'
    };

    class ActionContext
    {
    private:

        RuleStatus              status;

        Hash<Property>          variables;
        Hash<Property>&         constants;

        Session&                session;
        PropertyManager&        command;
        Statistics&             statistics;
    public:

        // TODO: implement, set commandAdapter & session
        ActionContext(Hash<Property>& _constants,
                      Session& _session, PropertyManager& _command,Statistics& _statistics)
            : constants(_constants), session(_session), command(_command), statistics(_statistics) {};
        ~ActionContext() {};
        
        inline RuleStatus& getStatus() {
            return status;
        }

        void SetRuleStatus(RuleStatus rs) {status = rs;};

        //Comment: 'name' is valid until 'var' is valid
        static FieldType Separate(const std::string& var, const char *& name);
        static bool ActionContext::StrToPeriod(CheckTrafficPeriod& period, std::string& str);


        bool checkTraffic(std::string routeId, CheckTrafficPeriod period, int64_t value);
        Property* getProperty(const std::string& var);
        void closeSession(bool Commit);
        void AddPendingOperation(uint8_t type, time_t pendingTime);
        Operation * GetCurrentOperation() {return session.GetCurrentOperation();}
    };

}}}

#endif // SCAG_RULE_ENGINE_ACTION_CONTEXT

