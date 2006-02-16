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


    BillingManager& bm = BillingManager::Instance();

    bm.commit(BillId);
    operation->detachBill(BillId);

    return true;
}

}}

