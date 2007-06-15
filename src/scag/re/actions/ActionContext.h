#ifndef SCAG_RULE_ENGINE_ACTION_CONTEXT
#define SCAG_RULE_ENGINE_ACTION_CONTEXT

#include <string>
#include <core/buffers/Hash.hpp>
#include <scag/re/RuleStatus.h>
#include "scag/bill/BillingManager.h"
//#include "scag/sessions/Session.h"
#include "scag/re/CommandBrige.h"
#include <stack>

namespace scag { namespace re { namespace actions 
{
    using namespace scag::util::properties;
    using smsc::core::buffers::Hash;
    using scag::re::RuleStatus;
    using namespace scag::stat;
    using namespace scag::bill;
    using namespace scag::sessions;
    using namespace scag::transport;

    enum FieldType
    {
        ftUnknown,
        ftLocal =   '%',
        ftConst =   '@',
        ftSession = '$',
        ftField =   '#'
    };
   /*
    struct ActionStackValue
    {
        int actionIndex;
        bool thenSection;

        ActionStackValue() : thenSection(false)
        {
        }

        ActionStackValue(int index, bool flag) : actionIndex(index), thenSection(flag)
        {
        }
    };
   */
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
        const Address& abonentAddr;
        int status;
        int protocol;
        int serviceId;

        int providerId;
        int operatorId;
        int msgRef;
        uint8_t commandId;
        CommandOperations cmdType;
        DataSmDirection direction;

        CommandProperty(SCAGCommand* command, int commandStatus, const Address& addr, int ProviderId, int OperatorId, int msgRef, CommandOperations CmdType);
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

        RuleStatus&              status;

        Hash<Property>          variables;
        Hash<Property>&         constants;

        Session&                session;
        CommandAccessor*        command;

        CommandProperty&        commandProperty;
        auto_ptr<TariffRec>     m_TariffRec;
    public:

        bool isTrueCondition;

        ActionContext(Hash<Property>& _constants,
                      Session& _session, CommandAccessor* _command, CommandProperty& _commandProperty, RuleStatus& rs)
            : constants(_constants), session(_session), command(_command), commandProperty(_commandProperty), status(rs)
        {
        };

        ~ActionContext() {};

        SCAGCommand& getSCAGCommand()
        {
            if(!command) throw SCAGException("ActionContext: command is not set");
            return command->getSCAGCommand();
        }

        inline RuleStatus& getRuleStatus() {
            return status;
        }

        void clearLongCallContext()
        {
            if(!command) throw SCAGException("ActionContext: command is not set");
            while (!command->getSCAGCommand().getSession()->getLongCallContext().ActionStack.empty()) 
                command->getSCAGCommand().getSession()->getLongCallContext().ActionStack.pop();
        }

        //Comment: 'name' is valid until 'var' is valid
        static FieldType Separate(const std::string& var, const char *& name);
        static bool ActionContext::StrToPeriod(CheckTrafficPeriod& period, std::string& str);

        CommandAccessor* getCommand() {
            if(!command) throw SCAGException("ActionContext: command is not set");
            return command;
        };

        Session& getSession() { return session; };        

        bool checkTraffic(std::string routeId, CheckTrafficPeriod period, int64_t value);
        Property* getProperty(const std::string& var);
        void abortSession();
        void AddPendingOperation(uint8_t type, time_t pendingTime, unsigned int billID);
        Operation * GetCurrentOperation() {return session.GetCurrentOperation();}

        CommandProperty& getCommandProperty() {return commandProperty;}

        void getBillingInfoStruct(BillingInfoStruct& billingInfoStruct)
        {
            billingInfoStruct.AbonentNumber = commandProperty.abonentAddr.toString();
            billingInfoStruct.serviceId = commandProperty.serviceId;
            billingInfoStruct.protocol = commandProperty.protocol;
            billingInfoStruct.providerId = commandProperty.providerId;
            billingInfoStruct.operatorId = commandProperty.operatorId;

            billingInfoStruct.SessionBornMicrotime = session.getPrimaryKey().BornMicrotime;
            billingInfoStruct.msgRef = commandProperty.msgRef;
        }

        TariffRec * getTariffRec(uint32_t category, uint32_t mediaType);
        bool checkIfCanSetPending(int operationType, int eventHandlerType, TransportType transportType);
        int getCurrentOperationBillID();
   };

}}}

#endif // SCAG_RULE_ENGINE_ACTION_CONTEXT

