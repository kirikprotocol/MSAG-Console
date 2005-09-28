#include "BillActionRollback.h"
#include "CDRBillingMachine.h"


namespace scag { namespace bill {

bool BillActionRollback::run(ActionContext& context)
{
    smsc_log_error(logger,"Run Action '[CDR]BillActionRollback'...");

    Operation * operation = context.GetCurrentOperation();
    if (!operation) 
    {
        smsc_log_error(logger,"Fatal error in action: '[CDR]BillActionRollback' - operation from ActionContext is invalid");
        return false;
    }

    CDRBillingMachine * bm = CDRBillingMachine::Instance();
    if (!bm) 
    {
        smsc_log_error(logger,"Fatal error in action: '[CDR]BillActionRollback' - BillingMachine is invalid");
        return false;
    }

    Bill bill; 

    try
    {
        BillKey billKey = context.CreateBillKey();
        bill = bm->GetBill(billKey);
    } 
    catch (...)
    {
        smsc_log_error(logger,"Action: '[CDR]BillActionRollback' - cannot build bill for billKey");
        return true;
    }

    bm->rollback(bill);
    operation->detachBill(bill);

    return true;
}

}}

