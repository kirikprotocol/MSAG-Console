#include "BillActionClose.h"
#include "scag/stat/Statistics.h"


namespace scag { namespace re { namespace actions {

using namespace scag::stat;


void BillActionClose::init(const SectionParams& params,PropertyObject propertyObject)
{
    logger = Logger::getInstance("scag.bill.actions");
    /*
    try
    {
        throw SCAGException("NOT CATCHED!");
    }
    catch (SCAGException& e)
    {
        smsc_log_debug(logger, "CATCHED!");
    } */


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

    Statistics& statistics = Statistics::Instance();

    SACC_BILLING_INFO_EVENT_t ev;

    
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
            smsc_log_error(logger,"BillAction 'bill:close': Fatal error in action - no bill is attached");
            SetBillingStatus(context, "no bill is attached", false);
            return true;
        }


        EventMonitor * eventMonitor = 0;
        TariffRec tariffRec;

        try
        {
            tariffRec = bm.CheckCharge(operation->getBillId(), &eventMonitor);
            if (!eventMonitor) throw SCAGException("Unknown error: EventMonitor is not valid");
        } catch (SCAGException& e)
        {
            smsc_log_error(logger,e.what());
            SetBillingStatus(context, e.what(), false);
            context.makeBillEvent(TRANSACTION_COMMITED, EXTERNAL_ERROR, tariffRec, ev);
            statistics.registerSaccEvent(ev);
            return true;
        }
        

        //TODO: Понять какое время нужно ждать до таймаута
        eventMonitor->wait(1000);
        TransactionStatus transactionStatus = bm.GetStatus(operation->getBillId());
        std::string logMessage;
        int CommandStatus = COMMAND_SUCCESSFULL;
        BillingTransactionEvent billTransactionEvent;

        switch (transactionStatus) 
        {
        case TRANSACTION_INVALID:
            logMessage = "abnormal transaction termination (billing transaction deny)";
            billTransactionEvent = TRANSACTION_COMMITED;
            CommandStatus = REJECTED_BY_SERVER;
            break;

        case TRANSACTION_WAIT_ANSWER:
            //TODO: дёрнуть тайм-аут на billing manager`е
            logMessage = "billing transaction time out";
            billTransactionEvent = TRANSACTION_TIME_OUT;
            CommandStatus = SERVER_NOT_RESPONSE;
            break;

        case TRANSACTION_NOT_STARTED:
            logMessage = "billing transaction invalid";
            billTransactionEvent = TRANSACTION_COMMITED;
            CommandStatus = INVALID_TRANSACTION;
            break;

        case TRANSACTION_VALID:
            logMessage = "billing transaction commited";
            billTransactionEvent = TRANSACTION_COMMITED;
            break;
        }

        if (CommandStatus == COMMAND_SUCCESSFULL) bm.commit(operation->getBillId());

        operation->detachBill();
        context.makeBillEvent(billTransactionEvent,CommandStatus, tariffRec, ev);

        SetBillingStatus(context, logMessage.c_str(), (CommandStatus == COMMAND_SUCCESSFULL));
        statistics.registerSaccEvent(ev);

        if (CommandStatus == COMMAND_SUCCESSFULL) 
            smsc_log_debug(logger,"Action 'bill:close': %s", logMessage.c_str());
        else
        {
            smsc_log_error(logger,"Action 'bill:close'. Details: %s", logMessage.c_str());
        }
    }
    else
    {
        Operation * operation = context.GetCurrentOperation();
        if (!operation)
        {
            smsc_log_error(logger,"BillAction 'bill:close': Fatal error in action - operation from ActionContext is invalid");
            SetBillingStatus(context,"operation from ActionContext is invalid", false);
            return true;
        }

        if (!operation->hasBill())
        {
            smsc_log_error(logger,"BillAction 'bill:close': Fatal error in action - bill is not attached to operation");
            SetBillingStatus(context,"Bill is not attached to operation", false);
            return true;
        }


        TariffRec tariffRec;

        try 
        {
            tariffRec = bm.getTransactionData(operation->getBillId());
        } catch (SCAGException& e)
        {
            smsc_log_error(logger,"BillAction 'bill:close' error. Delails: %s", e.what());
            SetBillingStatus(context,e.what(), false);
            return true;
        }
        
        try {
            bm.rollback(operation->getBillId());
            operation->detachBill();

            context.makeBillEvent(TRANSACTION_CALL_ROLLBACK, COMMAND_SUCCESSFULL, tariffRec, ev);
        } catch (SCAGException& e)
        {        
            smsc_log_error(logger,"BillAction 'bill:close' error. Delails: %s", e.what());
            context.makeBillEvent(TRANSACTION_CALL_ROLLBACK, EXTERNAL_ERROR, tariffRec, ev);
            SetBillingStatus(context,e.what(), false);
            statistics.registerSaccEvent(ev);
            return true;
        }   
         
        statistics.registerSaccEvent(ev);
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

