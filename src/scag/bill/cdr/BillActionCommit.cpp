#include "BillActionCommit.h"
#include "CDRBillingMachine.h"


namespace scag { namespace bill {


bool BillActionCommit::run(ActionContext& context)
{
    smsc_log_error(logger,"Run Action '[CDR]BillActionCommit'...");


    Operation * operation = context.GetCurrentOperation();
    if (!operation) 
    {
        smsc_log_error(logger,"Fatal error in action: '[CDR]BillActionCommit' - operation from ActionContext is invalid");
        return false;
    }

    CDRBillingMachine * bm = CDRBillingMachine::Instance();

    if (!bm) 
    {
        smsc_log_error(logger,"Fatal error in action: '[CDR]BillActionCommit' - BillingMachine is invalid");
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
        smsc_log_error(logger,"Action: '[CDR]BillActionCommit' - cannot build bill for billKey");
        return true;
    }
    
    bm->commit(bill);
    operation->detachBill(bill);

    return true;
}

}}

