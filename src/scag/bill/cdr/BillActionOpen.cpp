#include "BillActionOpen.h"
#include "CDRBillingMachine.h"

#include <iostream>

namespace scag { namespace bill {

bool BillActionOpen::run(ActionContext& context)
{
    smsc_log_error(logger,"Run Action '[CDR]BillActionOpen'...");

    Operation * operation = context.GetCurrentOperation();
    if (!operation) 
    {
        smsc_log_error(logger,"Fatal error in action: '[CDR]BillActionOpen' - operation from ActionContext is invalid");
        return false;
    }

    CDRBillingMachine * bm = CDRBillingMachine::Instance();
    if (!bm) 
    {
        smsc_log_error(logger,"Fatal error in action: '[CDR]BillActionOpen' - BillingMachine is invalid");
        return false;
    }

    Bill bill = bm->OpenTransaction();
    operation->attachBill(bill);

    return true;
}

}}

