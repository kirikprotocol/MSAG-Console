#include "ActionContext.h"
#include "scag/exc/SCAGExceptions.h"

namespace scag { namespace re { namespace actions 
{
    using smsc::core::buffers::Hash;
    using scag::re::RuleStatus;
    using namespace scag::util::properties;
    using namespace scag::exceptions;

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


bool ActionContext::makeBillEvent(int billCommand, std::string& category, std::string& medyaType, SACC_BILLING_INFO_EVENT_t& ev)
{
    Infrastructure& istr = BillingManager::Instance().getInfrastructure();

    int operatorId = istr.GetOperatorID(m_AbonentAddr);
    if (operatorId == 0) return false;

    auto_ptr<TariffRec> tariffRec(istr.GetTariff(operatorId, category.c_str(), medyaType.c_str()));
    if (!tariffRec.get()) return false;

    ev.Header.cCommandId = billCommand;

    ev.Header.cProtocolId = 1;
    ev.Header.iServiceId = m_ServiceId;
    ev.Header.iServiceProviderId = istr.GetProviderID(m_ServiceId);

    long now;
    time(&now);

    ev.Header.lDateTime = now;

    sprintf((char *)ev.Header.pAbonentNumber,"%s",m_AbonentAddr.toString().c_str());

    ev.Header.sCommandStatus = 1;
    
    ev.iOperatorId = operatorId;
    ev.iPriceCatId = 1;


    ev.fBillingSumm = tariffRec->Price;
    //ev.iMediaResourceType = 

    int size = MAX_BILLING_CURRENCY_LENGTH;
    if (size > tariffRec->Currency.size()) size = tariffRec->Currency.size();

    memcpy(ev.pBillingCurrency, tariffRec->Currency.c_str(), size);
    
}



}}}

