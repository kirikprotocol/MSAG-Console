#include "ActionContext2.h"
#include "scag/exc/SCAGExceptions.h"
#include "CommandBridge.h"
#include "scag/sessions/base/Session2.h"
#include "scag/sessions/base/Operation.h"
#include "scag/bill/base/BillingManager.h"

namespace scag2 {
namespace re {
namespace actions {

using smsc::core::buffers::Hash;
using re::RuleStatus;
using namespace util::properties;
using namespace scag::exceptions;


CommandProperty::CommandProperty( SCAGCommand* command,
                                  int commandStatus,
                                  const Address& addr,
                                  int ProviderId,
                                  int OperatorId,
                                  int ServiceId,
                                  int MsgRef,
                                  transport::CommandOperation opType,
                                  const Property& _routeId,
                                  uint8_t hi ) :
abonentAddr(addr),
protocol(-1),
serviceId(-1),
commandId(-1),
handlerId(hi),
direction(dsdUnknown),
routeId(_routeId)
{
    cmdType = opType;

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
    if ( infrastructConstants_ ) infrastructConstants_->Empty();
    tariffRec_.reset(0);
    // setInfrastructureConstants();
};


void ActionContext::clearLongCallContext()
{
    while ( ! getSession().getLongCallContext().ActionStack.empty() ) {
        getSession().getLongCallContext().ActionStack.pop();        
    }
}


void ActionContext::setContextScope( int id )
{
    if (!session_->getCurrentOperation()) {
        throw Exception("setCtxScope: session has no operation %s",
                        session_->sessionKey().toString().c_str() );
    }
    session_->getCurrentOperation()->setContextScope( id );
}


int ActionContext::getContextScope() const
{
    if (!session_->getCurrentOperation()) {
        throw Exception("getCtxScope: session has no operation %s",
                        session_->sessionKey().toString().c_str() );
    }
    return session_->getCurrentOperation()->getContextScope();
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
    case ftGlobal: {
        scope = session_->getGlobalScope();
        break;
    }

    case ftService: {
        if ( ! commandProperty_ )
            throw SCAGException( "ActionContext::getProperty(%s): command is not set", var.c_str() );
        scope = session_->getServiceScope( commandProperty_->serviceId );
        break;
    }

    case ftContext: {
        if ( !session_->getCurrentOperation()) {
            throw Exception("getProperty(context): session has no operation %s",
                            session_->sessionKey().toString().c_str() );
        }
        scope = session_->getContextScope( session_->getCurrentOperation()->getContextScope() );
        break;
    }

    case ftOperation: {
        scope = session_->getOperationScope();
        break;
    }
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

    case ftConst: {
        propertyPtr = getInfrastructConstant(name);
        if ( ! propertyPtr ) propertyPtr = constants_->GetPtr(name);
        break;
    }

    case ftField: {
        if ( !command_ ) throw SCAGException("ActionContext.getProperty(%s): command is not set", var.c_str() );
        propertyPtr = command_->getProperty(name);
        break;
    }

    default:
        return 0;
    }

    if ( ! propertyPtr && scope )
        propertyPtr = scope->getProperty(name);
    return propertyPtr;
}


void ActionContext::delProperty( const std::string& var )
{
    FieldType prefix;
    const char* name;
    prefix = Separate(var,name);
    SessionPropertyScope* scope = 0;

    switch (prefix) {
    case ftGlobal: {
        scope = session_->getGlobalScope();
        break;
    }
    case ftService: {
        if ( !commandProperty_ ) {
            throw SCAGException( "ActionContext:delProperty(%s): command is not set", var.c_str());
        }
        scope = session_->getServiceScope( commandProperty_->serviceId );
        break;
    }
    case ftContext: {
        if ( ! session_->getCurrentOperation() ) {
            throw SCAGException( "ActionContext:delProperty(%s): no current operation", var.c_str());
        }
        scope = session_->getContextScope( session_->getCurrentOperation()->getContextScope() );
        break;
    }
    case ftOperation: {
        scope = session_->getOperationScope();
        break;
    }
    case ftConst: {
        throw SCAGException( "ActionContext:delProperty(%s): constants cannot be deleted", var.c_str());
    }
    case ftField: {
        if (!command_) {
            throw SCAGException("ActionContext:delProperty(%s): command is not set", var.c_str());
        }
        command_->delProperty(name);
        break;
    }
    default:
        throw SCAGException( "ActionContext:delProperry(%s): unknowns cannot be deleted", var.c_str());
    }
    if (scope) {
        scope->delProperty(name);
    }
}


void ActionContext::getBillingInfoStruct( bill::BillingInfoStruct& bis )
{
    bis.AbonentNumber = commandProperty_->abonentAddr.toString().c_str();
    bis.serviceId = commandProperty_->serviceId;
    bis.protocol = commandProperty_->protocol;
    bis.providerId = commandProperty_->providerId;
    bis.operatorId = commandProperty_->operatorId;
    bis.SessionBornMicrotime = session_->sessionPrimaryKey().bornTime();
    bis.msgRef = commandProperty_->msgRef;
}


bill::infrastruct::TariffRec* ActionContext::getTariffRec( uint32_t category,
                                                           uint32_t medyaType)
{
    if ( tariffRec_.get() ) {
        if ( tariffRec_->CategoryId != category ||
             tariffRec_->MediaTypeId != medyaType ||
             tariffOperId_ != commandProperty_->operatorId ) {
            tariffRec_.reset(0);
        }
    }
    if (!tariffRec_.get())
    {
        bill::infrastruct::Infrastructure& istr = 
            bill::BillingManager::Instance().getInfrastructure();
        tariffRec_.reset( istr.GetTariff(commandProperty_->operatorId,
                                         category,
                                         medyaType) );
        tariffOperId_ = commandProperty_->operatorId;
        if (!tariffRec_.get())
            throw SCAGException( "BillEvent: Cannot find tariffRec for OID=%d, cat=%d, mtype=%d ",
                                 commandProperty_->operatorId, category, medyaType);
    }
    return tariffRec_.get();
}


Property* ActionContext::getInfrastructConstant( const char* pname )
{
    // determine the validity of the name
    Property* ret = 0;
    if ( pname ) {

        if (!commandProperty_) {
            throw SCAGException("ActionContext: commandProperty is not set");
            return 0;
        }

        int64_t propval;
        switch (pname[0]) {
        case 'p' :
            if ( strcmp(pname,"provider_id")==0 ) {
                propval = int64_t(commandProperty_->serviceId);
            } else {
                pname = 0;
            }
            break;
        case 'o' :
            if ( strcmp(pname,"operator_id")==0 ) {
                propval = int64_t(commandProperty_->operatorId);
            } else if ( strcmp(pname,"operation_id") ) {
                propval = int64_t(session_->getCurrentOperationId());
            } else {
                pname = 0;
            }
            break;
        case 'r' :
            if ( strcmp(pname,"route_id")==0 ) {
                if (!infrastructConstants_) {
                    infrastructConstants_ = new Hash<Property>();
                } else if ((ret = infrastructConstants_->GetPtr(pname))) {
                    break;
                }
                (*infrastructConstants_)[pname] = commandProperty_->routeId;
                ret = infrastructConstants_->GetPtr(pname);
            }
            pname = 0;
            break;
        case 's' :
            if ( strcmp(pname,"service_id")==0 ) {
                propval = int64_t(commandProperty_->serviceId);
            } else {
                pname = 0;
            }
            break;
        default :
            pname = 0;
        }

        if ( pname ) {
            if ( ! infrastructConstants_ ) infrastructConstants_ = new Hash<Property>();
            ret = infrastructConstants_->GetPtr(pname);
            if ( ! ret ) {
                Property p;
                p.setInt( propval );
                (*infrastructConstants_)[pname] = p;
            }
        }
    } // if pname
    return ret;
}

/*
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

    property.setInt(session_->getCurrentOperationId());
    infrastructConstants_["operation_id"] = property;

    // property.setInt(session_->sessionKey().toIndex());
    // infrastructConstants_["abonent"] = property;

    infrastructConstants_["route_id"] = commandProperty_->routeId;
}
 */

}}}

