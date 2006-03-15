#include "BillActionRollback.h"


namespace scag { namespace bill {

bool BillActionRollback::run(ActionContext& context)
{
    smsc_log_error(logger,"Run Action 'BillActionRollback'...");

    Operation * operation = context.GetCurrentOperation();
    if (!operation) 
    {
        smsc_log_error(logger,"BillActionRollback: Fatal error in action - operation from ActionContext is invalid");
        return false;
    }

    BillingManager& bm = BillingManager::Instance();
    bm.rollback(operation->getBillId());

    operation->detachBill();

    return true;
}

}}

