#include "BillActionCommit.h"


namespace scag { namespace bill {


bool BillActionCommit::run(ActionContext& context)
{
    smsc_log_error(logger,"Run Action 'BillActionCommit'...");

    int BillId = 0;

    Operation * operation = context.GetCurrentOperation();
    if (!operation) 
    {
        smsc_log_error(logger,"BillActionCommit: Fatal error in action - operation from ActionContext is invalid");
        return false;
    }
    /*
    try
    {
        BillKey billKey = context.CreateBillKey();
        //bill = BillingManager::GetBill(billKey);
    } 
    catch (...)
    {
        smsc_log_error(logger,"Action: '[CDR]BillActionCommit' - cannot build bill for billKey");
        return true;
    }   */


    if (operation->hasBill()) 
    {
        BillingManager& bm = BillingManager::Instance();

        EventMonitor eventMonitor;

        TransactionStatus transactionStatus = bm.CheckBill(operation->getBillId(), &eventMonitor);
        if (transactionStatus!=TRANSACTION_VALID) 
        {
            //TODO: Понять какое время нужно ждать до таймаута
            eventMonitor.wait();
        }

        transactionStatus = bm.GetStatus(operation->getBillId());

        if (transactionStatus!=TRANSACTION_VALID) 
        {
            smsc_log_error(logger,"BillActionCommit: billing transaction refused");

            bm.rollback(operation->getBillId());
            operation->detachBill();
            return true;
        }

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

