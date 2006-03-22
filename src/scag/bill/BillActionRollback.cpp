#include "BillActionRollback.h"


namespace scag { namespace bill {

bool BillActionRollback::run(ActionContext& context)
{
    smsc_log_error(logger,"Run Action 'BillActionRollback'...");

    /////////////////////////////////////////////

    Statistics& statistics = Statistics::Instance();

    SACC_BILLING_INFO_EVENT_t ev;


    /////////////////////////////////////////////



    Operation * operation = context.GetCurrentOperation();
    if (!operation) 
    {
        smsc_log_error(logger,"BillActionRollback: Fatal error in action - operation from ActionContext is invalid");
        return false;
    }

    BillingManager& bm = BillingManager::Instance();
    bm.rollback(operation->getBillId());

    operation->detachBill();

    CommandBrige::makeBillEvent(context.getServiceId(), context.getAbonentAddr().toString(), TRANSACTION_CALL_ROLLBACK, ev);
    statistics.registerSaccEvent(ev);


    return true;
}

}}

