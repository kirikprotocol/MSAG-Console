#include "BillActionCommit.h"


namespace scag { namespace bill {


bool BillActionCommit::run(ActionContext& context)
{
    smsc_log_error(logger,"Run Action '[CDR]BillActionCommit'...");

    int BillId = 0;

    Operation * operation = context.GetCurrentOperation();
    if (!operation) 
    {
        smsc_log_error(logger,"Fatal error in action: '[CDR]BillActionCommit' - operation from ActionContext is invalid");
        return false;
    }

    try
    {
        BillKey billKey = context.CreateBillKey();
        //bill = BillingManager::GetBill(billKey);
    } 
    catch (...)
    {
        smsc_log_error(logger,"Action: '[CDR]BillActionCommit' - cannot build bill for billKey");
        return true;
    }
    
    operation->detachBill(BillId);

    return true;
}

}}

