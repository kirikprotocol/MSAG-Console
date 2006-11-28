#ifndef SCAG_RULE_ENGINE_ACTION_CONTEXT
#define SCAG_RULE_ENGINE_ACTION_CONTEXT

#include <sms/sms_serializer.h>
#include <string>
#include <core/buffers/Hash.hpp>
#include <scag/re/RuleStatus.h>
#include "scag/sessions/Session.h"
#include "scag/lcm/LongCallManager.h"

namespace scag { namespace re { namespace actions 
{
 
    using namespace scag::util;
    using namespace scag::util::properties;
    using smsc::core::buffers::Hash;
    using scag::re::RuleStatus;
    using namespace scag::stat;
    using namespace scag::sessions;
    using namespace scag::lcm;
    using namespace smsc::core::buffers;

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
        /*
        virtual void fillChargeOperation(smsc::inman::interaction::ChargeSms& op, TariffRec& tariffRec) = 0;
        virtual void fillRespOperation(smsc::inman::interaction::DeliverySmsResult& op, TariffRec& tariffRec) = 0;
        */
        //virtual SMS& getSMS() = 0;
        virtual SCAGCommand& getSCAGCommand() = 0;
    };

   
    struct CommandProperty
    {
        Address& abonentAddr;
        int status;
        int protocol;
        int serviceId;

        int providerId;
        int operatorId;
        int msgRef;
        uint8_t commandId;
        CommandOperations cmdType;

        CommandProperty(SCAGCommand& command, int commandStatus, Address& addr, int ProviderId, int OperatorId, int msgRef, CommandOperations CmdType);
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
            : constants(_constants), session(_session), 
              command(_command), commandProperty(_commandProperty) 
        {
        };

        ~ActionContext() {};
        
        inline RuleStatus& getRuleStatus() {
            return status;
        }

        void setRuleStatus(RuleStatus rs) {status = rs;};

        //Comment: 'name' is valid until 'var' is valid
        static FieldType Separate(const std::string& var, const char *& name);
        static bool ActionContext::StrToPeriod(CheckTrafficPeriod& period, std::string& str);


        CommandAccessor& getCommand() { return command; };
        SCAGCommand& getSCAGCommand() { return command.getSCAGCommand(); };
        Session& getSession() { return session; };        

        bool checkTraffic(std::string routeId, CheckTrafficPeriod period, int64_t value);
        Property* getProperty(const std::string& var);
        void abortSession();
        void AddPendingOperation(uint8_t type, time_t pendingTime, int billID);
        Operation * GetCurrentOperation() {return session.GetCurrentOperation();}

        CommandProperty& getCommandProperty() {return commandProperty;}

        BillingInfoStruct getBillingInfoStruct()
        {
            BillingInfoStruct billingInfoStruct;

            billingInfoStruct.AbonentNumber = commandProperty.abonentAddr.toString();
            billingInfoStruct.serviceId = commandProperty.serviceId;
            billingInfoStruct.protocol = commandProperty.protocol;
            billingInfoStruct.providerId = commandProperty.providerId;
            billingInfoStruct.operatorId = commandProperty.operatorId;

            billingInfoStruct.SessionBornMicrotime = session.getPrimaryKey().BornMicrotime;
            billingInfoStruct.msgRef = commandProperty.msgRef;

            return billingInfoStruct;
        }


        TariffRec * getTariffRec(uint32_t category, uint32_t mediaType);
        bool getTariffRec(uint32_t category, uint32_t mediaType, TariffRec& tr);
        bool checkIfCanSetPending(int operationType, int eventHandlerType, TransportType transportType);
        int getCurrentOperationBillID();

        void clearLongCallContext()
        {
            LongCallContext& longCallContext = command.getSCAGCommand().getLongCallContext();
            while (longCallContext.ActionStack.empty()) longCallContext.ActionStack.pop();
        }

        /*LongCallBuffer& getBuffer()
        {
            return command.getSCAGCommand().getLongCallContext().contextActionBuffer;
        } */
        

   };

}}}

#endif // SCAG_RULE_ENGINE_ACTION_CONTEXT

