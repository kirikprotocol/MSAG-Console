#include "ActionContext2.h"
#include "scag/exc/SCAGExceptions.h"
#include "scag/re/CommandBridge.h"
#include "scag/sessions/base/Session2.h"

namespace scag2 {
namespace re {
namespace actions {

    using smsc::core::buffers::Hash;
    using scag::re::RuleStatus;
    using namespace scag::util::properties;
    using namespace scag::exceptions;


CommandProperty::CommandProperty( SCAGCommand* command,
                                  int commandStatus,
                                  const Address& addr,
                                  int ProviderId,
                                  int OperatorId,
                                  int ServiceId,
                                  int MsgRef,
                                  CommandOperations CmdType,
                                  const Property& _routeId ) :
abonentAddr(addr),
protocol(-1),
serviceId(-1),
commandId(-1),
direction(dsdUnknown),
routeId(_routeId)
{
    cmdType = CmdType;

    if (command)
    {
        protocol = CommandBridge::getProtocolForEvent(*command);
        commandId = command->getCommandId();
        direction = CommandBridge::getPacketDirection(*command);    
    }

    status = commandStatus;

    providerId = ProviderId;
    operatorId = OperatorId;
    serviceId = ServiceId;
    msgRef = MsgRef;
}



FieldType ActionContext::Separate(const std::string& var,const char *& name)
{
    if ( var.size()<=1 ) return ftUnknown;
    
    name = var.c_str() + 1;

    switch ( var[0] ) {
    case ftGlobal:     return ftGlobal;
    case ftService:    return ftService;
    case ftContext:    return ftContext;
    case ftOperation:  return ftOperation;
    case ftConst:      return ftConst;
    case ftField:      return ftField;
    default:           return ftUnknown;
    }
}


void ActionContext::resetContext( Hash<Property>*  constants,
                                  Session*         session, 
                                  CommandAccessor* command,
                                  CommandProperty* commandProperty,
                                  RuleStatus*      rs )
{
    isTrueCondition = false;
    status_ = rs;
    constants_ = constants;
    session_ = session;
    command_ = command;
    commandProperty_ = commandProperty;
    setInfrastructureConstants();
};


void ActionContext::clearLongCallContext()
{
    while ( ! getSession().getLongCallContext().ActionStack.empty() ) {
        getSession().getLongCallContext().ActionStack.pop();        
    }
}


Property* ActionContext::getProperty( const std::string& var )
{
    FieldType prefix;
    const char* name;
    Property*              propertyPtr = 0;
    SessionPropertyScope*  scope = 0;

    prefix = Separate(var,name);

    switch (prefix) 
    {
    case ftGlobal:
        scope = session_->getGlobalScope();
        break;

    case ftService:
        if ( ! commandProperty_ )
            throw SCAGException( "ActionContext::getProperty(%s): command is not set", var.c_str() );
        scope = session_->getServiceScope( commandProperty_->serviceId );
        break;

    case ftContext:
        scope = session_->getContextScope( contextId_ );
        break;

    case ftOperation:
        scope = session_->getOperationScope();
        break;

        /*
    case ftLocal:
        propertyPtr = variables.GetPtr(name);

        if (!propertyPtr) 
        {
            Property property;
            variables.Insert(name,property);
            propertyPtr = variables.GetPtr(name);
        }
        return propertyPtr;
         */

    case ftConst:
        propertyPtr = infrastructConstants_.GetPtr(name);
        if ( ! propertyPtr ) propertyPtr = constants_->GetPtr(name);
        break;

    case ftField:
        if ( !command_ ) throw SCAGException("ActionContext.getProperty(%s): command is not set", var.c_str() );
        propertyPtr = command_->getProperty(name);
        break;

    default:
        return 0;
    }

    if ( ! propertyPtr && scope )
        propertyPtr = scope->getProperty(name);
    return propertyPtr;
}


void ActionContext::abortSession()
{
    session_->abort();
}


void ActionContext::getBillingInfoStruct( BillingInfoStruct& bis )
{
    bis.AbonentNumber = commandProperty_->abonentAddr.toString();
    bis.serviceId = commandProperty_->serviceId;
    bis.protocol = commandProperty_->protocol;
    bis.providerId = commandProperty_->providerId;
    bis.operatorId = commandProperty_->operatorId;

    // FIXME!
    timeval tv;
    bis.SessionBornMicrotime = tv; // session->getPrimaryKey().BornMicrotime;
    bis.msgRef = commandProperty_->msgRef;
}


TariffRec* ActionContext::getTariffRec(uint32_t category, uint32_t medyaType)
{
    if (!tariffRec_.get())
    {
        Infrastructure& istr = BillingManager::Instance().getInfrastructure();
        tariffRec_.reset( istr.GetTariff(commandProperty_->operatorId,
                                         category,
                                         medyaType) );

        if (!tariffRec_.get())
            throw SCAGException( "BillEvent: Cannot find tariffRec for OID=%d, cat=%d, mtype=%d ",
                                 commandProperty_->operatorId, category, medyaType);
    }
    return tariffRec_.get();
}




/*
void ActionContext::AddPendingOperation(uint8_t type, time_t pendingTime, unsigned int billID)
{
    PendingOperation pendingOperation;
    pendingOperation.validityTime = pendingTime;
    pendingOperation.type = type;
    
    if (billID > 0) 
        pendingOperation.billID = billID;
    
    session->addPendingOperation(pendingOperation);
}
 */


#if 0
bool ActionContext::checkIfCanSetPending(int operationType, int eventHandlerType, TransportType transportType)
{
    return (operationType != CO_DELIVER) && (operationType != CO_USSD_DIALOG);
    
    /* TODO: Reserved for future constrants 
    switch (transportType) 
    {
        case SMPP: 
        case MMS:
        case HTTP:
            return true;
    }
    return false;
    */
}

int ActionContext::getCurrentOperationBillID()
{
    Operation * operation = session->GetCurrentOperation();
    if (!operation) throw SCAGException("Operation: cannot find billing current operation");

    if (!operation->hasBill()) throw SCAGException("Operation: cannot find billing transaction");

    return operation->getBillId();
}

#endif


void ActionContext::setInfrastructureConstants() {
    if (!commandProperty_) {
        throw SCAGException("ActionContext: commandProperty is not set");
        return;
    }
    Property property;
    property.setInt(commandProperty_->serviceId);
    infrastructConstants_["service_id"] = property;

    property.setInt(commandProperty_->providerId);
    infrastructConstants_["provider_id"] = property;

    property.setInt(commandProperty_->operatorId);
    infrastructConstants_["operator_id"] = property;

    infrastructConstants_["route_id"] = commandProperty_->routeId;
}


}}}

