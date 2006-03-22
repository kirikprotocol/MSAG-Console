#include "BillActionCommit.h"
#include "scag/stat/Statistics.h"


namespace scag { namespace bill {

using namespace scag::stat;

bool BillActionCommit::run(ActionContext& context)
{
    smsc_log_error(logger,"Run Action 'BillActionCommit'...");

    /////////////////////////////////////////////

    Statistics& statistics = Statistics::Instance();

    SACC_BILLING_INFO_EVENT_t ev;

   
    /////////////////////////////////////////////


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
            //TODO: ������ ����� ����� ����� ����� �� ��������
            eventMonitor.wait(1000);
        }

        transactionStatus = bm.GetStatus(operation->getBillId());

        switch (transactionStatus) 
        {
        case TRANSACTION_INVALID:
            smsc_log_error(logger,"BillActionCommit: billing transaction refused");

            bm.rollback(operation->getBillId());
            operation->detachBill();

            CommandBrige::makeBillEvent(context.getServiceId(), context.getAbonentAddr().toString(), TRANSACTION_REFUSED, ev);
            statistics.registerSaccEvent(ev);

            return true;
            break;

        case TRANSACTION_WAIT_ANSWER:
            //TODO: ������ ����-��� �� billing manager`�

            smsc_log_error(logger,"BillActionCommit: billing transaction time out");

            bm.rollback(operation->getBillId());
            operation->detachBill();

            CommandBrige::makeBillEvent(context.getServiceId(), context.getAbonentAddr().toString(), TRANSACTION_TIME_OUT, ev);
            statistics.registerSaccEvent(ev);

            return true;
            break;
        }


        CommandBrige::makeBillEvent(context.getServiceId(), context.getAbonentAddr().toString(), TRANSACTION_COMMITED, ev);
        statistics.registerSaccEvent(ev);

        bm.commit(operation->getBillId());
        operation->detachBill();
        smsc_log_error(logger,"BillActionCommit: billing transaction commited successfully");
    } 
    else
    {   
        smsc_log_error(logger,"BillActionCommit: cannot commit bill - bill for current operation not found");
    }


    return true;
}

}}

