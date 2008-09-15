#ifndef SCAG_RULE_ENGINE_ACTION_CONTEXT2
#define SCAG_RULE_ENGINE_ACTION_CONTEXT2

#include <stack>
#include <string>
#include <memory>
#include <core/buffers/Hash.hpp>
#include "sms/sms.h"
#include "scag/exc/SCAGExceptions.h"
#include "RuleStatus2.h"
#include "scag/util/properties/Properties2.h"
#include "scag/transport/CommandOperation.h"
#include "scag/transport/smpp/base/SmppCommandIds.h"
#include "scag/bill/base/BillingInfoStruct.h"
#include "scag/bill/base/Infrastructure.h"

namespace scag2 {

namespace sessions {
class Operation;
class Session;
}

namespace transport {
class SCAGCommand;
}

namespace re {
namespace actions {

using namespace scag::util::properties;
using smsc::core::buffers::Hash;
using namespace sessions;
using transport::SCAGCommand;

enum FieldType
{
        ftUnknown,
        ftGlobal    = '$',
        ftService   = '~',
        ftContext   = '^',
        ftOperation = '%',
        ftConst     = '@',
        ftField     = '#'
};


class CommandAccessor : public PropertyManager
{
public:
    virtual SCAGCommand& getSCAGCommand() = 0;
};

   
    struct CommandProperty
    {
        const smsc::sms::Address& abonentAddr;
        int status;
        int protocol;
        int serviceId;

        int providerId;
        int operatorId;
        int msgRef;
        uint8_t commandId;
        transport::CommandOperation cmdType;
        transport::smpp::DataSmDirection direction;

        Property routeId;

        CommandProperty( SCAGCommand* command,
                         int commandStatus,
                         const smsc::sms::Address& addr,
                         int ProviderId,
                         int OperatorId,
                         int ServiceId,
                         int msgRef,
                         transport::CommandOperation CmdType,
                         const Property& routeId );
    };


    struct InfrastructIDs
    {
        int operatorId;
        int providerId;

        InfrastructIDs() : operatorId(0), providerId(0) {}
    };


    class ActionContext
    {
    public:
        static FieldType Separate( const std::string& var, const char *& name );

    public:
        ActionContext( Hash<Property>*  constants,
                       Session*         session, 
                       CommandAccessor* command,
                       CommandProperty* commandProperty,
                       RuleStatus*      rs) :
        isTrueCondition(false),
        status_(rs),
        constants_(constants),
        session_(session),
        command_(command),
        contextId_(0),
        commandProperty_(commandProperty)
        {
            setInfrastructureConstants();
        }


        ~ActionContext() {}


        void resetContext( Hash<Property>* _constants,
                           Session* _session,
                           CommandAccessor* _command,
                           CommandProperty* _commandProperty,
                           RuleStatus* rs );

        SCAGCommand& getSCAGCommand()
        {
            if (!command_) throw scag::exceptions::SCAGException("ActionContext: command is not set");
            return command_->getSCAGCommand();
        }

        inline RuleStatus& getRuleStatus() {
            return *status_;
        }

        void clearLongCallContext();
        /*
        {
            while (!getSession().getLongCallContext().ActionStack.empty()) 
                getSession().getLongCallContext().ActionStack.pop();
        }
         */

        // static bool StrToPeriod(CheckTrafficPeriod& period, std::string& str);

        CommandAccessor* getCommand() {
            if (!command_) throw scag::exceptions::SCAGException("ActionContext: command is not set");
            return command_;
        };

        Session& getSession() { return *session_; };

        // bool checkTraffic(std::string routeId, CheckTrafficPeriod period, int64_t value);
        bool getDestroyService() const { return destroyService_; }
        void setDestroyService() { destroyService_ = true; }

        /// set the current context scope for property access.
        void setContextScope( int id ) {
            contextId_ = id;
        }
        int getContextScope() const { return contextId_; }

        /// get property, variable prefix defines the scope
        Property* getProperty(const std::string& var);

        // void abortSession();

        // void AddPendingOperation(uint8_t type, time_t pendingTime, unsigned int billID);
        // Operation * getCurrentOperation() { return session_->getCurrentOperation(); }

        CommandProperty& getCommandProperty() { return *commandProperty_; }

        /// fill billing infostructure with data
        void getBillingInfoStruct( bill::BillingInfoStruct& billingInfoStruct );

        /*
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
         */

        bill::infrastruct::TariffRec * getTariffRec( uint32_t category,
                                                     uint32_t mediaType );

        // bool checkIfCanSetPending(int operationType, int eventHandlerType, TransportType transportType);
        // int getCurrentOperationBillID();

    private:
        void setInfrastructureConstants();

    public:
        bool                    isTrueCondition;

    private:
        RuleStatus*             status_;

        // Hash<Property>          variables_;
        Hash<Property>*         constants_;
        Hash<Property>          infrastructConstants_;

        Session*                session_;
        CommandAccessor*        command_;
        int                     contextId_;   // current context scope id (0 -- invalid)
        CommandProperty*        commandProperty_;
        std::auto_ptr< bill::infrastruct::TariffRec >  tariffRec_;
        bool                    destroyService_;
   };

}}}

#endif // SCAG_RULE_ENGINE_ACTION_CONTEXT

