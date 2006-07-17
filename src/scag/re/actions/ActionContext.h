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


    class CommandAccessor : public PropertyManager
    {
    public:
        virtual void fillChargeOperation(smsc::inman::interaction::ChargeSms& op, TariffRec& tariffRec) = 0;
        virtual void fillRespOperation(smsc::inman::interaction::DeliverySmsResult& op, TariffRec& tariffRec) = 0;
    };




    struct CommandProperty
    {
        Address& abonentAddr;
        int status;
        int protocol;
        int serviceId;

        int providerId;
        int operatorId;
        uint8_t commandId;
        CommandOperations cmdType;

        CommandProperty(SCAGCommand& command, int commandStatus, Address& addr, int ProviderId, int OperatorId, CommandOperations CmdType);
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
        CommandAccessor&        command;

        CommandProperty&        commandProperty;
        auto_ptr<TariffRec>     m_TariffRec;
    public:

        ActionContext(Hash<Property>& _constants,
                      Session& _session, CommandAccessor& _command, CommandProperty& _commandProperty)
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


        CommandAccessor& getCommand() { return command; };

        bool checkTraffic(std::string routeId, CheckTrafficPeriod period, int64_t value);
        Property* getProperty(const std::string& var);
        void abortSession();
        void AddPendingOperation(uint8_t type, time_t pendingTime, int billID);
        Operation * GetCurrentOperation() {return session.GetCurrentOperation();}

        void makeBillEvent(int billCommand, int commandStatus, TariffRec& tariffRec, SACC_BILLING_INFO_EVENT_t& ev);
        CommandProperty& getCommandProperty() {return commandProperty;}
        void fillChargeOperation(smsc::inman::interaction::ChargeSms& op, TariffRec& tariffRec);
        void fillRespOperation(smsc::inman::interaction::DeliverySmsResult& op, TariffRec& tariffRec);

        TariffRec * getTariffRec(uint32_t category, uint32_t mediaType);
        bool checkIfCanSetPending(int operationType, int eventHandlerType, TransportType transportType);
        int getCurrentOperationBillID();

   };

}}}

#endif // SCAG_RULE_ENGINE_ACTION_CONTEXT

