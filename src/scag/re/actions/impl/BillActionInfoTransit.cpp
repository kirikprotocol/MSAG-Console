#include "BillActionInfoTransit.h"
#include "scag/bill/base/BillingManager.h"

namespace scag2 {
namespace re {
namespace actions {

BillActionInfoTransit::BillActionInfoTransit() :
BillActionPreOpen(true),
totalAmount_(*this,"resultAmount",false,false),
chargeThreshold_(*this,"resultChargeThreshold",false,false)
{
    setExternalId("");
}


bool BillActionInfoTransit::RunBeforePostpone( ActionContext& context )
{
    std::auto_ptr< bill::BillOpenCallParamsData > bpd(makeParamsData(context));
    if ( ! bpd.get() ) return false;

    if ( bpd->tariffRec.billType != bill::infrastruct::EWALLET ) {
        throw SCAGException("Action '%s': is not allowed for non-ewallet type", opname());
    }

    LongCallContext& lcmCtx = context.getSession().getLongCallContext();
    lcmCtx.callCommandId = BILL_INFO;
    bill::EwalletInfoCallParams* eicp = 
        new bill::EwalletInfoCallParams(bpd.release(),&lcmCtx);
    lcmCtx.setParams(eicp);
    // smsc_log_debug(logger,"lcm ewallet params created: %p", lcmCtx.getParams() );
    return true;
}


void BillActionInfoTransit::ContinueRunning( ActionContext& context )
{
    if ( ! preContinueRunning(context) ) return;

    bill::EwalletInfoCallParams* eicp = static_cast<bill::EwalletInfoCallParams*>
        (context.getSession().getLongCallContext().getParams());
    setBillingStatus(context,"",true);
    if ( totalAmount_.isFound() ) {
        Property* p = totalAmount_.getProperty(context);
        if (p) p->setInt(eicp->getResultAmount());
    }
    if ( chargeThreshold_.isFound() ) {
        Property* p = chargeThreshold_.getProperty(context);
        if (p) p->setInt(eicp->getChargeThreshold());
    }
}

void BillActionInfoTransit::postInit( const SectionParams& params,
                                      PropertyObject propertyObject )
{
    // optional fields
    totalAmount_.init(params,propertyObject);
    chargeThreshold_.init(params,propertyObject);
}

}
}
}
