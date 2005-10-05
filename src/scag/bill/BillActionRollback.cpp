#include "BillActionRollback.h"


namespace scag { namespace bill {

bool BillActionRollback::run(ActionContext& context)
{
    int BillId = 9;

    smsc_log_error(logger,"Run Action '[CDR]BillActionRollback'...");

    Operation * operation = context.GetCurrentOperation();
    if (!operation) 
    {
        smsc_log_error(logger,"Fatal error in action: '[CDR]BillActionRollback' - operation from ActionContext is invalid");
        return false;
    }

    try
    {
        BillKey billKey = context.CreateBillKey();
        //BillId = BillingManager::GetBill(billKey);
    } 
    catch (...)
    {
        smsc_log_error(logger,"Action: '[CDR]BillActionRollback' - cannot build bill for billKey");
        return true;
    }

    operation->detachBill(BillId);

    return true;
}

}}

