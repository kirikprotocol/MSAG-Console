#include "ActionContext.h"
#include "scag/exc/SCAGExceptions.h"
#include "scag/re/CommandBrige.h"

namespace scag { namespace re { namespace actions 
{
    using smsc::core::buffers::Hash;
    using scag::re::RuleStatus;
    using namespace scag::util::properties;
    using namespace scag::exceptions;


CommandProperty::CommandProperty(SCAGCommand& command, int commandStatus, Address& addr, int ProviderId, int OperatorId, CommandOperations CmdType)
    : abonentAddr(addr)
{
    cmdType = CmdType;
    serviceId = command.getServiceId();
    protocol = CommandBrige::getProtocolForEvent(command);

    commandId = command.getCommandId();
    
    status = commandStatus;
    
    providerId = ProviderId;
    operatorId = OperatorId;
}




void ActionContext::AddPendingOperation(uint8_t type, time_t pendingTime, int billID)
{
    PendingOperation pendingOperation;
    pendingOperation.validityTime = pendingTime;
    pendingOperation.type = type;
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


void ActionContext::makeBillEvent(int billCommand, int commandStatus, TariffRec& tariffRec, SACC_BILLING_INFO_EVENT_t& ev)
{
    ev.Header.cCommandId = billCommand;

    ev.Header.cProtocolId = commandProperty.protocol;
    ev.Header.iServiceId = commandProperty.serviceId;

    ev.Header.iServiceProviderId = commandProperty.providerId;

    timeval tv;
    gettimeofday(&tv,0);

    ev.Header.lDateTime = (uint64_t)tv.tv_sec*1000 + (tv.tv_usec / 1000);

    ev.Header.pAbonentNumber = commandProperty.abonentAddr.toString();
    ev.Header.sCommandStatus = commandStatus;
    ev.Header.iOperatorId = commandProperty.operatorId;
    ev.iPriceCatId = tariffRec.CategoryId;
    
    
    ev.fBillingSumm = tariffRec.Price;
    ev.iMediaResourceType = tariffRec.MediaTypeId;

    ev.pBillingCurrency = tariffRec.Currency;

    CSessionPrimaryKey& sessionPrimaryKey = session.getPrimaryKey();

    ev.pSessionKey = sessionPrimaryKey.abonentAddr.toString();

    char buff[128];
    sprintf(buff,"%s/%ld%d", sessionPrimaryKey.abonentAddr.toString().c_str(), sessionPrimaryKey.BornMicrotime.tv_sec,sessionPrimaryKey.BornMicrotime.tv_usec / 1000);
    ev.pSessionKey.append(buff);

}

TariffRec * ActionContext::getTariffRec(uint32_t category, uint32_t medyaType)
{
    if (!m_TariffRec.get()) 
    {
        Infrastructure& istr = BillingManager::Instance().getInfrastructure();
        m_TariffRec.reset(istr.GetTariff(commandProperty.operatorId, category, medyaType));

        if (!m_TariffRec.get()) 
            throw SCAGException("BillEvent: Cannot find tariffRec for OID=%d, cat=%d, mtype=%d ", commandProperty.operatorId, category, medyaType);
    }

    return m_TariffRec.get();
}


void ActionContext::fillChargeOperation(smsc::inman::interaction::ChargeSms& op, TariffRec& tariffRec)
{
    command.fillChargeOperation(op, tariffRec);
}

void ActionContext::fillRespOperation(smsc::inman::interaction::DeliverySmsResult& op, TariffRec& tariffRec)
{
    command.fillRespOperation(op, tariffRec);
}


bool ActionContext::checkIfCanSetPending(int operationType, int eventHandlerType, TransportType transportType)
{
    //��������� �� ����������� ��������� pending operation ��� ������� ���������� � ��������:

   
    bool result = false;
    
    switch (transportType) 
    {
    case SMPP: 
        /*
        DELIVER_SM:          SUBMIT, USSD_DIALOG
        RECEIPT_DELIVER_SM:  SUBMIT, USSD_DIALOG
        USSD DELIVER_SM:     SUBMIT
        USSD SUBMIT_SM:      SUBMIT
        */

        if (commandProperty.commandId == DELIVERY)
        {
            if (operationType == CO_SUBMIT) result = true;
            else 
            if ((operationType == CO_USSD_DIALOG)&&
               ((eventHandlerType == EH_RECEIPT)||(eventHandlerType == EH_DELIVER_SM))) result = true;
        } 

        if ((commandProperty.commandId == SUBMIT)&&(commandProperty.cmdType == CO_USSD_DIALOG)&&(operationType == CO_SUBMIT)) 
            result = true;

        if ((commandProperty.commandId == SUBMIT)&&(!commandProperty.cmdType != CO_USSD_DIALOG)&&(operationType == CO_HTTP_DELIVERY)) 
            result = true;

        break;

    case HTTP:
        break;

    case MMS:
        break;
    }
    
    return result;
   
}

int ActionContext::getCurrentOperationBillID()
{
    Operation * operation = session.GetCurrentOperation();
    if (!operation) return 0;

    return operation->getBillId();
}


}}}

