#include "BillActionClose.h"

namespace scag { namespace re { namespace actions {

void BillActionClose::init(const SectionParams& params,PropertyObject propertyObject)
{
    if(!params.Exists("action")) throw SCAGException("Action 'bill:close': missing 'action' paramener.");

    std::string str = params["action"];

    actionCommit = str == "commit";

    if(!actionCommit&&(str!="rollback")) throw SCAGException("Action 'bill:close': unrecognised 'action' paramener ''",str.c_str());

    FieldType ft;
    std::string temp;
    bool bExist;

    ft = CheckParameter(params, propertyObject, "bill:close", "status", false, false, temp, m_StatusExist);
    if (m_StatusExist) m_sStatus = temp;

    ft = CheckParameter(params, propertyObject, "bill:close", "msg", false, false, temp, m_MsgExist);
    if (m_MsgExist) m_sMessage = temp;

    smsc_log_debug(logger,"Action 'bill:close' init...");    
}

bool BillActionClose::run(ActionContext& context)
{
    smsc_log_debug(logger, "Run Action 'bill:close'...");

    Operation * operation = context.GetCurrentOperation();
    if (!operation || !operation->hasBill())
    {
        char *p = !operation ? "Bill: Operation from ActionContext is invalid" : "Bill is not attached to operation";
        smsc_log_error(logger, p);
        SetBillingStatus(context, p, false);
        return true;
    }

    try {
        if(operation->getBillId() != (uint32_t)-1)
        {
            BillingManager& bm = BillingManager::Instance();
            if(actionCommit)
                bm.Commit(operation->getBillId());
            else
                bm.Rollback(operation->getBillId());
        }
        SetBillingStatus(context,"", true);
    } catch (SCAGException& e)
    {        
        smsc_log_error(logger,"BillAction 'bill:close' error. Delails: %s", e.what());
        SetBillingStatus(context, e.what(), false);
    }   
    operation->detachBill();
    return true;
}

void BillActionClose::SetBillingStatus(ActionContext& context, const char * errorMsg, bool isOK)
{
    Property * property;
    if (m_StatusExist) 
    {
        property = context.getProperty(m_sStatus);
        if (!property) 
        {
            smsc_log_debug(logger,"BillAction 'bill:close' :: Invalid property %s for status", m_sStatus.c_str());
            return;
        }

        property->setInt(!isOK);
    }

    if (m_MsgExist) 
    {
        property = context.getProperty(m_sMessage);

        if (!property) 
        {
            smsc_log_debug(logger,"BillAction 'bill:close' :: Invalid property %s for msg", m_sMessage.c_str());
            return;
        }
        property->setStr(std::string(errorMsg));
    }

    return;
}

IParserHandler * BillActionClose::StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("Action 'bill:close' cannot include child objects");
}

bool BillActionClose::FinishXMLSubSection(const std::string& name)
{
    return true;
}

}}}
