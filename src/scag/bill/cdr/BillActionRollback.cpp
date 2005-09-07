#include "BillActionRollback.h"


namespace scag { namespace bill {

bool BillActionRollback::run(ActionContext& context)
{
    CDRBillingMachine * bm = CDRBillingMachine::Instance();
    if (!bm) return false;

    Operation * operation = context.GetCurrentOperation();
    if (!operation) return false;

    //bm->rollback(

    return true;
}

}}

