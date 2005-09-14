#include "BillActionOpen.h"
#include "CDRBillingMachine.h"


namespace scag { namespace bill {

bool BillActionOpen::run(ActionContext& context)
{
    Operation * operation = context.GetCurrentOperation();
    if (!operation) return false;

    CDRBillingMachine * bm = CDRBillingMachine::Instance();
    if (!bm) return false;

    Bill bill = bm->OpenTransaction();
    operation->attachBill(bill);

    return true;
}

}}

