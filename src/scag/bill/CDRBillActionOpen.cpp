#include "BillActionOpen.h"


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

    int BillId = 0;// = bm->OpenTransaction();
    operation->attachBill(BillId);

    return true;
}

}}

