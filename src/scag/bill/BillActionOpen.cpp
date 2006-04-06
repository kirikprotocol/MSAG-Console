#include "BillActionOpen.h"


namespace scag { namespace bill {

bool BillActionOpen::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'BillActionOpen'...");

    /////////////////////////////////////////////

    Statistics& statistics = Statistics::Instance();

    SACC_BILLING_INFO_EVENT_t ev;


    /////////////////////////////////////////////




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

    context.makeBillEvent(TRANSACTION_OPEN, m_category, m_mediaType, ev);
    statistics.registerSaccEvent(ev);

    return true;
}

}}

