#include "ActionContext.h"
#include "scag/exc/SCAGExceptions.h"
#include "scag/re/CommandBrige.h"

namespace scag { namespace re { namespace actions 
{
    using smsc::core::buffers::Hash;
    using scag::re::RuleStatus;
    using namespace scag::util::properties;
    using namespace scag::exceptions;


CommandProperty::CommandProperty(SCAGCommand& command, int commandStatus, Address& addr, int ProviderId, int OperatorId, int MsgRef, CommandOperations CmdType)
    : abonentAddr(addr)
{
    cmdType = CmdType;
    serviceId = command.getServiceId();
    protocol = CommandBrige::getProtocolForEvent(command);

    commandId = command.getCommandId();
    
    status = commandStatus;
    
    providerId = ProviderId;
    operatorId = OperatorId;
    msgRef = MsgRef;
    direction = CommandBrige::getPacketDirection(command);
}




void ActionContext::AddPendingOperation(uint8_t type, time_t pendingTime, unsigned int billID)
{
    PendingOperation pendingOperation;
    pendingOperation.validityTime = pendingTime;
    pendingOperation.type = type;
    
    if (billID > 0) 
        pendingOperation.billID = billID;
    
    session.addPendingOperation(pendingOperation);
}


FieldType ActionContext::Separate(const std::string& var,const char *& name)
{
    if (var.size()<=1) return ftUnknown;
    
    name = var.c_str() + 1;

    if (var[0] == ftLocal) return ftLocal;
    if (var[0] == ftConst) return ftConst;
    if (var[0] == ftField) return ftField;
    if (var[0] == ftSession) return ftSession;

    return ftUnknown;
}

Property* ActionContext::getProperty(const std::string& var)
{
    FieldType prefix;
    const char * name;
    Property * propertyPtr = 0;

    prefix = Separate(var,name);


    switch (prefix) 
    {
    case ftLocal:
        propertyPtr = variables.GetPtr(name);

        if (!propertyPtr) 
        {
            Property property;
            variables.Insert(name,property);
            propertyPtr = variables.GetPtr(name);
        }
        return propertyPtr;
        break;

    case ftConst:
        return constants.GetPtr(name);
        break;

    case ftField: return command.getProperty(name);
    case ftSession: return session.getProperty(name);

    default:
        return 0;
    }

}

void ActionContext::abortSession()
{
    session.abort();
}


TariffRec * ActionContext::getTariffRec(uint32_t category, uint32_t medyaType)
{
    if (!m_TariffRec.get()) 
    {
        Infrastructure& istr = BillingManager::Instance().getInfrastructure();
        TariffRec * trec = istr.GetTariff(commandProperty.operatorId, category, medyaType);

        if (!trec) 
            throw SCAGException("BillEvent: Cannot find tariffRec for OID=%d, cat=%d, mtype=%d ", commandProperty.operatorId, category, medyaType);

        m_TariffRec.reset(trec);
    }

    return m_TariffRec.get();
}




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
    Operation * operation = session.GetCurrentOperation();
    if (!operation) throw SCAGException("Operation: cannot find billing current operation");

    if (!operation->hasBill()) throw SCAGException("Operation: cannot find billing transaction");

    return operation->getBillId();
}


}}}

