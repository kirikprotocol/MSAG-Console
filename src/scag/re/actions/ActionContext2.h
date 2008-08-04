#ifndef SCAG_RULE_ENGINE_ACTION_CONTEXT2
#define SCAG_RULE_ENGINE_ACTION_CONTEXT2

#include <stack>
#include <string>
#include <core/buffers/Hash.hpp>
#include "scag/re/RuleStatus2.h"
#include "scag/sessions/Session2.h"
#include "scag/bill/BillingManager.h"
#include "scag/re/CommandBridge.h"
#include "scag/lcm/LongCallManager2.h"

namespace scag2 {
namespace re {
namespace actions {

    using namespace scag::util::properties;
    using smsc::core::buffers::Hash;
    using namespace scag::stat;
    using namespace scag::bill;
    using namespace sessions;
    using namespace transport;

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

        Property routeId;

        CommandProperty(SCAGCommand* command, int commandStatus, const Address& addr, int ProviderId, int OperatorId,
                         int ServiceId, int msgRef, CommandOperations CmdType, const Property& routeId);
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

        RuleStatus*             status;

        Hash<Property>          variables;
        Hash<Property>*         constants;
        Hash<Property>          infrastructConstants;

        Session*                session;
        CommandAccessor*        command;

        CommandProperty*        commandProperty;
        auto_ptr<TariffRec>     m_TariffRec;
        void setInfrastructureConstants();
    public:

        bool isTrueCondition;

        ActionContext(Hash<Property>* _constants, Session* _session, 
		      CommandAccessor* _command, CommandProperty* _commandProperty, RuleStatus* rs) :
        status(rs), 
        constants(_constants),
        session(_session), command(_command), commandProperty(_commandProperty)
        {
          setInfrastructureConstants();   
        };
        ~ActionContext() {};

        void resetContext(Hash<Property>* _constants, Session* _session, 
			  CommandAccessor* _command, CommandProperty* _commandProperty,
			  RuleStatus* rs);

        SCAGCommand& getSCAGCommand()
        {
            if(!command) throw SCAGException("ActionContext: command is not set");
            return command->getSCAGCommand();
        }

        inline RuleStatus& getRuleStatus() {
            return *status;
        }

        void clearLongCallContext()
        {
            while (!getSession().getLongCallContext().ActionStack.empty()) 
                getSession().getLongCallContext().ActionStack.pop();
        }

        //Comment: 'name' is valid until 'var' is valid
        static FieldType Separate(const std::string& var, const char *& name);
        static bool StrToPeriod(CheckTrafficPeriod& period, std::string& str);

        CommandAccessor* getCommand() {
            if(!command) throw SCAGException("ActionContext: command is not set");
            return command;
        };

        Session& getSession() { return *session; };        

        bool checkTraffic(std::string routeId, CheckTrafficPeriod period, int64_t value);
        Property* getProperty(const std::string& var);
        void abortSession();
        void AddPendingOperation(uint8_t type, time_t pendingTime, unsigned int billID);
        Operation * GetCurrentOperation() {return session->getCurrentOperation();}

        CommandProperty& getCommandProperty() { return *commandProperty; }

        /// FIXME: this should be changed (several billing transactions per session!)
        void getBillingInfoStruct(BillingInfoStruct& billingInfoStruct)
        {
            billingInfoStruct.AbonentNumber = commandProperty->abonentAddr.toString();
            billingInfoStruct.serviceId = commandProperty->serviceId;
            billingInfoStruct.protocol = commandProperty->protocol;
            billingInfoStruct.providerId = commandProperty->providerId;
            billingInfoStruct.operatorId = commandProperty->operatorId;

            // FIXME!
            timeval tv;
            billingInfoStruct.SessionBornMicrotime = tv; // session->getPrimaryKey().BornMicrotime;
            billingInfoStruct.msgRef = commandProperty->msgRef;
        }

        TariffRec * getTariffRec(uint32_t category, uint32_t mediaType);
        bool checkIfCanSetPending(int operationType, int eventHandlerType, TransportType transportType);
        int getCurrentOperationBillID();
   };

}}}

#endif // SCAG_RULE_ENGINE_ACTION_CONTEXT

