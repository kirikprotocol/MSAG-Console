#include "BillActionClose.h"

namespace scag { namespace re { namespace actions {

void BillActionClose::init(const SectionParams& params,PropertyObject propertyObject)
{
    BillID = 0;
    if(!params.Exists("action")) throw SCAGException("Action 'bill:close': missing 'action' paramener.");

    std::string str = params["action"];

    actionCommit = str == "commit";

    if(!actionCommit&&(str!="rollback")) throw SCAGException("Action 'bill:close': unrecognised 'action' paramener ''",str.c_str());

    CheckParameter(params, propertyObject, "bill:close", "status", false, false, m_sStatus, m_StatusExist);
    CheckParameter(params, propertyObject, "bill:close", "msg", false, false, m_sMessage, m_MsgExist);

    FieldType ft = CheckParameter(params, propertyObject, "bill:close", "bill_id", false, true, m_sBillID, m_BillIDExist);
    if(m_BillIDExist && ft == ftUnknown && !(BillID = atoi(m_sBillID.c_str())))
        throw SCAGException("Action 'bill:close': bill_id should be positive integer value. bill_id='%s'", m_sBillID.c_str());

    smsc_log_debug(logger,"Action 'bill:close' init...");    
}

bool BillActionClose::run(ActionContext& context)
{
    Operation * op = NULL;
    uint32_t bid = BillID;
    smsc_log_debug(logger, "Run Action 'bill:close'...");
    
    if(m_BillIDExist)
    {
        if(!bid)
        {
            Property * p = context.getProperty(m_sBillID);

            if (!p)
            {
                smsc_log_error(logger,"Action 'bill:close' :: Invalid property %s for BillID", m_sBillID.c_str());
                return true;
            }
            else
                bid = p->getInt();
        }
    }
    else
    {
        op = context.GetCurrentOperation();
        if (!op || !op->hasBill())
        {
            char *p = !op ? "Bill: Operation from ActionContext is invalid" : "Bill is not attached to operation";
            smsc_log_error(logger, p);
            SetBillingStatus(context, p, false);
            return true;
        }
        bid = op->getBillId();
    }
    if (!bid)
    {
        smsc_log_error(logger,"Action 'bill:close' :: Invalid BillID=0");
        if(op) op->detachBill();
        return true;
    }
    try {
        if(bid != (uint32_t)-1)
        {
            BillingManager& bm = BillingManager::Instance();
            if(actionCommit)
                bm.Commit(bid);
            else
                bm.Rollback(bid);
        }
        SetBillingStatus(context,"", true);
    } catch (SCAGException& e)
    {        
        smsc_log_error(logger,"BillAction 'bill:close' error. Delails: %s", e.what());
        SetBillingStatus(context, e.what(), false);
    }   
    if(op) op->detachBill();
    return true;
}

void BillActionClose::SetBillingStatus(ActionContext& context, const char * errorMsg, bool isOK)
{
    Property * property;
    if(m_StatusExist) 
    {
        if(!(property = context.getProperty(m_sStatus))) 
        {
            smsc_log_debug(logger,"BillAction 'bill:close' :: Invalid property %s for status", m_sStatus.c_str());
            return;
        }
        property->setInt(!isOK);
    }

    if(m_MsgExist) 
    {
        if(!(property = context.getProperty(m_sMessage))) 
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
