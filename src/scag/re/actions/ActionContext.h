#ifndef SCAG_RULE_ENGINE_ACTION_CONTEXT
#define SCAG_RULE_ENGINE_ACTION_CONTEXT

#include <string>
#include <core/buffers/Hash.hpp>
#include <scag/re/RuleStatus.h>
//#include <scag/util/properties/Properties.h>
#include "scag/sessions/Session.h"


namespace scag { namespace re { namespace actions 
{
    using namespace scag::util::properties;
    using smsc::core::buffers::Hash;
    using scag::re::RuleStatus;
    using namespace scag::stat;
    using namespace scag::sessions;

    enum FieldType
    {
        ftUnknown,
        ftLocal =   '%',
        ftConst =   '@',
        ftSession = '$',
        ftField =   '#'
    };



    struct CommandProperty
    {
        Address& abonentAddr;
        int status;
        int protocol;
        int serviceId;

        CommandProperty(SCAGCommand& command, int commandStatus, Address& addr);
    };


    struct InfrastructIDs
    {
        int operatorId;
        int providerId;

        InfrastructIDs() : operatorId(0), providerId(0) {}
    };

    class ActionContext
    {
    private:

        RuleStatus              status;

        Hash<Property>          variables;
        Hash<Property>&         constants;

        Session&                session;
        PropertyManager&        command;

        CommandProperty&        commandProperty;
        InfrastructIDs          m_InfrastructIDs;
    public:

        ActionContext(Hash<Property>& _constants,
                      Session& _session, PropertyManager& _command, CommandProperty& _commandProperty)
            : constants(_constants), session(_session), command(_command), commandProperty(_commandProperty) 
        {
        };

        ~ActionContext() {};
        
        inline RuleStatus& getRuleStatus() {
            return status;
        }

        void setRuleStatus(RuleStatus rs) {status = rs;};
        //int getServiceId() {return commandProperty.serviceId;}
        //int getHandlerType() {return m_handlerType;}

        //Address& getAbonentAddr() {return commandProperty.abonentAddr;}

        //Comment: 'name' is valid until 'var' is valid
        static FieldType Separate(const std::string& var, const char *& name);
        static bool ActionContext::StrToPeriod(CheckTrafficPeriod& period, std::string& str);


        bool checkTraffic(std::string routeId, CheckTrafficPeriod period, int64_t value);
        Property* getProperty(const std::string& var);
        void abortSession();
        void AddPendingOperation(uint8_t type, time_t pendingTime);
        Operation * GetCurrentOperation() {return session.GetCurrentOperation();}

        void makeBillEvent(int billCommand, std::string& category, std::string& medyaType, SACC_BILLING_INFO_EVENT_t& ev);
        CommandProperty& getCommandProperty() {return commandProperty;}

        void setInfrastructIDs(InfrastructIDs& infrastructIDs) {m_InfrastructIDs = infrastructIDs;}
        InfrastructIDs getInfrastructIDs() {return m_InfrastructIDs;}

    };

}}}

#endif // SCAG_RULE_ENGINE_ACTION_CONTEXT

