#include "BillActionClose.h"

namespace scag { namespace re { namespace actions {


void BillActionClose::init(const SectionParams& params,PropertyObject propertyObject)
{
    logger = Logger::getInstance("scag.bill.actions");


    if (params.Exists("action")) 
    {
        std::string str = params["action"];

        if ((str!="commit")&&(str!="rollback")) throw SCAGException("Action 'bill:close': unrecognised 'action' paramener ''",str.c_str());

        if (str == "commit") actionCommit = true;
        else actionCommit = false;
    }
    else 
        throw SCAGException("Action 'bill:close': missing 'action' paramener.");


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



    /////////////////////////////////////////////
    BillingManager& bm = BillingManager::Instance();

    if (actionCommit) 
    {
        Operation * operation = context.GetCurrentOperation();
        if (!operation)
        {
            smsc_log_error(logger,"BillAction 'bill:close': Fatal error in action - operation from ActionContext is invalid");
            SetBillingStatus(context, "operation from ActionContext is invalid", false);
            return true;
        }

        if (!operation->hasBill()) 
        {
            smsc_log_error(logger,"Action 'bill:close': Fatal error in action - no bill is attached");
            SetBillingStatus(context, "no bill is attached", false);
            return true;
        }


        try
        {
            bm.Commit(operation->getBillId());
        } catch (SCAGException& e)
        {
            smsc_log_error(logger,"Action 'bill:close' cannot process. Delails: %s", e.what());
            SetBillingStatus(context, e.what(), false);

            operation->detachBill();
            return true;
        }
        
        operation->detachBill();
        smsc_log_debug(logger,"Action 'bill:close': transaction successfully commited");
    }
    else
    {
        Operation * operation = context.GetCurrentOperation();
        if (!operation)
        {
            smsc_log_error(logger,"BillAction 'bill:close' error: Operation from ActionContext is invalid");
            SetBillingStatus(context,"operation from ActionContext is invalid", false);
            return true;
        }

        if (!operation->hasBill())
        {
            smsc_log_error(logger,"BillAction 'bill:close' error: Bill is not attached to operation");
            SetBillingStatus(context,"Bill is not attached to operation", false);
            return true;
        }


        try {
            bm.Rollback(operation->getBillId());
        } catch (SCAGException& e)
        {        
            smsc_log_error(logger,"BillAction 'bill:close' error. Delails: %s", e.what());
            SetBillingStatus(context,e.what(), false);
            operation->detachBill();
            return true;
        }   
        operation->detachBill();
        SetBillingStatus(context,"", true);
    }

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
            smsc_log_debug(logger,"BillAction 'bill:open' :: Invalid property %s for status", m_sStatus.c_str());
            return;
        }

        if (isOK) 
        {
            property->setInt(0);
            return;
        }

        property->setInt(1);
    }

    if (m_MsgExist) 
    {
        property = context.getProperty(m_sMessage);

        if (!property) 
        {
            smsc_log_debug(logger,"BillAction 'bill:open' :: Invalid property %s for msg", m_sMessage.c_str());
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

