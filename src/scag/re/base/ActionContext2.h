#ifndef SCAG_RULE_ENGINE_ACTION_CONTEXT2
#define SCAG_RULE_ENGINE_ACTION_CONTEXT2

#include <stack>
#include <string>
#include <memory>
#include <core/buffers/Hash.hpp>
#include "sms/sms.h"
#include "scag/exc/SCAGExceptions.h"
#include "RuleStatus2.h"
#include "Rule2.h"
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
    commandProperty_(commandProperty),
    destroyService_(-1),
    rule_(0)
    {
        setInfrastructureConstants();
    }


    ~ActionContext() {
        if ( rule_ ) rule_->unref();
    }


    void setRule( Rule& r ) {
        r.ref();
        if ( rule_ ) rule_->unref();
        rule_ = &r;
    }


    Rule* getRule() const {
        return rule_;
    }


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

    /// return the flag of destroying service:
    /// <0 -- do not destroy the service;
    /// >=0 -- destroy the service and set the expiration time to the given amount of seconds
    int getDestroyService() const { return destroyService_; }
    void setDestroyService( int wait = 0 ) { destroyService_ = wait; }

    /// set the current context scope for property access.
    void setContextScope( int id );
    int getContextScope() const;

    /// get property, variable prefix defines the scope
    Property* getProperty(const std::string& var);

    CommandProperty& getCommandProperty() { return *commandProperty_; }

    /// fill billing infostructure with data
    void getBillingInfoStruct( bill::BillingInfoStruct& billingInfoStruct );

    bill::infrastruct::TariffRec * getTariffRec( uint32_t category,
                                                 uint32_t mediaType );

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
    CommandProperty*        commandProperty_;
    std::auto_ptr< bill::infrastruct::TariffRec >  tariffRec_;
    int                     destroyService_;

    Rule*                   rule_;        // a rule from long call context
};

}
}
}

#endif // SCAG_RULE_ENGINE_ACTION_CONTEXT

