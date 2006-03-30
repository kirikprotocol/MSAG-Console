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

    class ActionContext
    {
    private:

        RuleStatus              status;

        Hash<Property>          variables;
        Hash<Property>&         constants;

        Session&                session;
        PropertyManager&        command;

        int                     m_ServiceId;
        Address                 m_AbonentAddr;
    public:

        ActionContext(Hash<Property>& _constants,
                      Session& _session, PropertyManager& _command, int serviceId, Address AbonentAddr)
            : constants(_constants), session(_session), command(_command) 
        {
            m_ServiceId = serviceId;
            m_AbonentAddr = AbonentAddr;
        };

        ~ActionContext() {};
        
        inline RuleStatus& getRuleStatus() {
            return status;
        }

        void setRuleStatus(RuleStatus rs) {status = rs;};
        int getServiceId() {return m_ServiceId;}
        //int getHandlerType() {return m_handlerType;}

        Address& getAbonentAddr() {return m_AbonentAddr;}

        //Comment: 'name' is valid until 'var' is valid
        static FieldType Separate(const std::string& var, const char *& name);
        static bool ActionContext::StrToPeriod(CheckTrafficPeriod& period, std::string& str);


        bool checkTraffic(std::string routeId, CheckTrafficPeriod period, int64_t value);
        Property* getProperty(const std::string& var);
        void abortSession();
        void AddPendingOperation(uint8_t type, time_t pendingTime);
        Operation * GetCurrentOperation() {return session.GetCurrentOperation();}

        bool makeBillEvent(int billCommand, std::string& category, std::string& medyaType, SACC_BILLING_INFO_EVENT_t& ev);
    };

}}}

#endif // SCAG_RULE_ENGINE_ACTION_CONTEXT

