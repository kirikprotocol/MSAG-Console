#include "BillActionOpen.h"


namespace scag { namespace bill {

bool BillActionOpen::run(ActionContext& context)
{
    smsc_log_error(logger,"Run Action 'BillActionOpen'...");

    Operation * operation = context.GetCurrentOperation();
    if (!operation) 
    {
        smsc_log_error(logger,"BillActionOpen: Fatal error in action - operation from ActionContext is invalid");
        return false;
    }

    BillingManager& bm = BillingManager::Instance();

    CTransportId transportId;
    int BillId = bm.ChargeBill(transportId);
    operation->attachBill(BillId);

    return true;
}

}}

