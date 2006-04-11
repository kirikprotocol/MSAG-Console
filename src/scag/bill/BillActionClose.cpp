#include "BillActionClose.h"
#include "scag/stat/Statistics.h"


namespace scag { namespace bill {

using namespace scag::stat;


void BillActionClose::init(const SectionParams& params,PropertyObject propertyObject)
{
    FieldType ft;
    const char * name = 0;

    if (!logger) 
        logger = Logger::getInstance("scag.bill.actions");

    if (params.Exists("action")) 
    {
        std::string str = ConvertWStrToStr(params["action"]);

        if ((str!="commit")&&(str!="rollback")) throw SCAGException("Action 'bill:close': unrecognised 'action' paramener ''",str.c_str());

        if (str == "commit") actionCommit = true;
        else actionCommit = false;
    }
    else 
        throw SCAGException("Action 'bill:close': missing 'action' paramener.");


    if (params.Exists("status")) 
    {
        m_sStatus = ConvertWStrToStr(params["status"]);
        ft = ActionContext::Separate(m_sStatus,name);
        if (ft==ftUnknown) 
            throw SCAGException("BillAction 'bill:open' : unrecognized variable prefix '%s' for 'status' parameter",m_sStatus.c_str());
    }

    if (params.Exists("msg")) 
    {
        m_sMessage = ConvertWStrToStr(params["msg"]);

        ft = ActionContext::Separate(m_sMessage,name);
        if (ft==ftUnknown) 
            throw SCAGException("BillAction 'bill:open' : unrecognized variable prefix '%s' for 'msg' parameter",m_sMessage.c_str());
    }

    smsc_log_debug(logger,"Action 'bill:close' init...");


}

bool BillActionClose::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'BillActionClose'...");

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
            return false;
        }

        CTransactionData transactionData = bm.getTransactionData(operation->getBillId());

        bm.commit(operation->getBillId());
        operation->detachBill();

        try 
        {
            context.makeBillEvent(TRANSACTION_REFUSED, transactionData.category, transactionData.mediatype, ev);
        } catch (SCAGException& e)
        {
            smsc_log_warn(logger,"BillAction 'bill:close' return false. Delails: %s", e.what());
            //TODO: set to status - false
            return true;
        }


        statistics.registerSaccEvent(ev);
    }
    else
    {
        Operation * operation = context.GetCurrentOperation();
        if (!operation)
        {
            smsc_log_error(logger,"BillAction 'bill:close': Fatal error in action - operation from ActionContext is invalid");
            return false;
        }

        CTransactionData transactionData = bm.getTransactionData(operation->getBillId());

        bm.rollback(operation->getBillId());
        operation->detachBill();
        
        try 
        {
            context.makeBillEvent(TRANSACTION_CALL_ROLLBACK, transactionData.category, transactionData.mediatype, ev);
        } catch (SCAGException& e)
        {
            smsc_log_warn(logger,"BillAction 'bill:close' return false. Delails: %s", e.what());
            //TODO: set to status - false
            return true;
        }

        statistics.registerSaccEvent(ev);
    }

 /*
    int BillId = 0;

    Operation * operation = context.GetCurrentOperation();
    if (!operation) 
    {
        smsc_log_error(logger,"BillActionCommit: Fatal error in action - operation from ActionContext is invalid");
        return false;
    }
  
    if (operation->hasBill()) 
    {
        BillingManager& bm = BillingManager::Instance();

        EventMonitor eventMonitor;

        TransactionStatus transactionStatus = bm.CheckBill(operation->getBillId(), &eventMonitor);
        if (transactionStatus == TRANSACTION_WAIT_ANSWER) 
        {
            //TODO: Понять какое время нужно ждать до таймаута
            eventMonitor.wait(1000);
        }

        transactionStatus = bm.GetStatus(operation->getBillId());

        switch (transactionStatus) 
        {
        case TRANSACTION_INVALID:
            smsc_log_error(logger,"BillActionCommit: billing transaction refused");

            bm.rollback(operation->getBillId());
            operation->detachBill();


            if (!context.makeBillEvent(TRANSACTION_REFUSED, m_category, m_mediaType, ev))
            {
                smsc_log_error(logger,"Cannot read tariff matrix parameners for category '%s', mediatype '%s'", m_category.c_str(), m_mediaType.c_str());
                //statistics.registerSaccEvent(
                return true;
            }

            statistics.registerSaccEvent(ev);

            return true;
            break;

        case TRANSACTION_WAIT_ANSWER:
            //TODO: дёрнуть тайм-аут на billing manager`е

            smsc_log_error(logger,"BillActionCommit: billing transaction time out");

            bm.rollback(operation->getBillId());
            operation->detachBill();

            if (!context.makeBillEvent(TRANSACTION_TIME_OUT, m_category, m_mediaType, ev))
            {
                smsc_log_error(logger,"Cannot read tariff matrix parameners for category '%s', mediatype '%s'", m_category.c_str(), m_mediaType.c_str());
                //statistics.registerSaccEvent(
                return true;
            }


            statistics.registerSaccEvent(ev);

            return true;
            break;
        }


        if (!context.makeBillEvent(TRANSACTION_COMMITED, m_category, m_mediaType, ev))
        {
            smsc_log_error(logger,"Cannot read tariff matrix parameners for category '%s', mediatype '%s'", m_category.c_str(), m_mediaType.c_str());
            //statistics.registerSaccEvent(
            return true;
        }


        statistics.registerSaccEvent(ev);

        bm.commit(operation->getBillId());
        operation->detachBill();
        smsc_log_error(logger,"BillActionCommit: billing transaction commited successfully");
    } 
    else
    {   
        smsc_log_error(logger,"BillActionCommit: cannot commit bill - bill for current operation not found");
    }

 */
    return true;
}

IParserHandler * BillActionClose::StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("Action 'bill:close' cannot include child objects");
}


bool BillActionClose::FinishXMLSubSection(const std::string& name)
{
    return true;
}


}}

