#include "ActionContext.h"
#include "scag/exc/SCAGExceptions.h"
#include "scag/re/CommandBrige.h"

namespace scag { namespace re { namespace actions 
{
    using smsc::core::buffers::Hash;
    using scag::re::RuleStatus;
    using namespace scag::util::properties;
    using namespace scag::exceptions;


CommandProperty::CommandProperty(SCAGCommand& command, int commandStatus, Address& addr, int ProviderId, int OperatorId)
    : abonentAddr(addr)
{
    serviceId = command.getServiceId();
    protocol = CommandBrige::getProtocolForEvent(command);

    status = commandStatus;

    providerId = ProviderId;
    operatorId = OperatorId;
}




void ActionContext::AddPendingOperation(uint8_t type, time_t pendingTime)
{
    PendingOperation pendingOperation;
    pendingOperation.validityTime = pendingTime;
    pendingOperation.type = type;
    
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
    Property * p = 0;

    prefix = Separate(var,name);

    switch (prefix) 
    {
    case ftLocal:
        if (!variables.Exists(name)) 
        {
            Property property;
            variables.Insert(name,property);
        }
        return variables.GetPtr(name);
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


void ActionContext::makeBillEvent(int billCommand, TariffRec& tariffRec, SACC_BILLING_INFO_EVENT_t& ev)
{
    ev.Header.cCommandId = billCommand;

    ev.Header.cProtocolId = commandProperty.protocol;
    ev.Header.iServiceId = commandProperty.serviceId;

    ev.Header.iServiceProviderId = commandProperty.providerId;

    timeval tv;
    gettimeofday(&tv,0);

    ev.Header.lDateTime = (uint64_t)tv.tv_sec*1000 + (tv.tv_usec / 1000);
    
    sprintf((char *)ev.Header.pAbonentNumber,"%s",commandProperty.abonentAddr.toString().c_str());

    ev.Header.sCommandStatus = commandProperty.status;
    
    ev.iOperatorId = commandProperty.operatorId;
    ev.iPriceCatId = tariffRec.CategoryId;
    
    
    ev.fBillingSumm = tariffRec.Price;
    ev.iMediaResourceType = tariffRec.MediaTypeId;

    int size = MAX_BILLING_CURRENCY_LENGTH;
    if (size > tariffRec.Currency.size()) size = tariffRec.Currency.size();

    memcpy(ev.pBillingCurrency, tariffRec.Currency.c_str(), size);

    CSessionPrimaryKey& sessionPrimaryKey = session.getPrimaryKey();
    sprintf((char *)ev.pSessionKey,"%s/%ld%d", sessionPrimaryKey.abonentAddr.toString().c_str(), sessionPrimaryKey.BornMicrotime.tv_sec,sessionPrimaryKey.BornMicrotime.tv_usec / 1000);

}

TariffRec * ActionContext::getTariffRec(std::string& category, std::string& medyaType)
{
    if (!m_TariffRec.get()) 
    {
        Infrastructure& istr = BillingManager::Instance().getInfrastructure();
        m_TariffRec.reset(istr.GetTariff(commandProperty.operatorId, category.c_str(), medyaType.c_str()));

        //if (!m_TariffRec.get()) 
          //  throw SCAGException("BillEvent: Cannot find tariffRec for OID=%d, cat=%s, mtype=%s ", commandProperty.operatorId, category.c_str(), medyaType.c_str());
    }

    return m_TariffRec.get();
}


void ActionContext::fillChargeOperation(smsc::inman::interaction::ChargeSms& op, TariffRec& tariffRec)
{
    
    command.fillChargeOperation(op, tariffRec);
}

}}}

