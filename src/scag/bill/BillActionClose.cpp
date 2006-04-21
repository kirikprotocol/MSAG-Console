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
            return true;
        }

        if (!operation->hasBill()) 
        {
            smsc_log_error(logger,"BillAction 'bill:close': Fatal error in action - no bill is attached");
            return true;
        }


        
        EventMonitor eventMonitor;

        TariffRec tariffRec;
        TransactionStatus transactionStatus = bm.CheckBill(operation->getBillId(), &eventMonitor, tariffRec);


        if (transactionStatus == TRANSACTION_WAIT_ANSWER) 
        {
            //TODO: Понять какое время нужно ждать до таймаута
            eventMonitor.wait(1000);
            transactionStatus = bm.GetStatus(operation->getBillId());
        }

        switch (transactionStatus) 
        {
        case TRANSACTION_INVALID:
            smsc_log_error(logger,"BillAction 'bill:close': billing transaction deny");

            bm.rollback(operation->getBillId());
            operation->detachBill();


            try
            {
                context.makeBillEvent(TRANSACTION_REFUSED, tariffRec, ev);
            } catch (SCAGException& e) {
                smsc_log_warn(logger,"BillAction 'bill:close' return false. Delails: %s", e.what());
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

            try 
            {
                context.makeBillEvent(TRANSACTION_TIME_OUT, tariffRec, ev);
            } catch (SCAGException& e) 
            {
                smsc_log_warn(logger,"BillAction 'bill:close' return false. Delails: %s", e.what());
                return true;
            }

            return true;
            break;
        }

        bm.commit(operation->getBillId());
        operation->detachBill();

        try 
        {
            context.makeBillEvent(TRANSACTION_COMMITED, tariffRec, ev);
        } catch (SCAGException& e)
        {
            smsc_log_warn(logger,"BillAction 'bill:close' return false. Delails: %s", e.what());
            return true;
        }


        smsc_log_error(logger,"Action 'bill:close': Transaction commited");
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

        
        try 
        {
            TariffRec tariffRec = bm.getTransactionData(operation->getBillId());

            bm.rollback(operation->getBillId());
            operation->detachBill();

            context.makeBillEvent(TRANSACTION_CALL_ROLLBACK, tariffRec, ev);
        } catch (SCAGException& e)
        {        
            smsc_log_warn(logger,"BillAction 'bill:close' return false. Delails: %s", e.what());
            //TODO: set to status - false
            return true;
        }

        statistics.registerSaccEvent(ev);
    }

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

