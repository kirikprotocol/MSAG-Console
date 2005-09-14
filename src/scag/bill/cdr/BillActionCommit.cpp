#include "BillActionCommit.h"
#include "CDRBillingMachine.h"


namespace scag { namespace bill {

bool BillActionCommit::run(ActionContext& context)
{
    Operation * operation = context.GetCurrentOperation();
    if (!operation) return false;

    CDRBillingMachine * bm = CDRBillingMachine::Instance();

    if (!bm) return false;
    Bill bill; 

    try
    {
        BillKey billKey = context.CreateBillKey();
        bill = bm->GetBill(billKey);
    } 
    catch (...)
    {
        operation->detachBill(bill);
        return true;
    }
    
    bm->commit(bill);
    operation->detachBill(bill);

    return true;
}

}}

