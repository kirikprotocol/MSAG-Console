#include "BillActionCloseTransit.h"

namespace scag2 {
namespace re {
namespace actions {

bool BillActionCloseTransit::RunBeforePostpone(ActionContext& context)
{
    std::auto_ptr<bill::BillOpenCallParamsData> bpd(makeParamsData(context));
    if (!bpd.get()) return false;

    if (bpd->tariffRec.billType == bill::infrastruct::EWALLET ) {
        LongCallContext& lcmCtx = context.getSession().getLongCallContext();
        lcmCtx.callCommandId = actionCommit_ ? BILL_COMMIT : BILL_ROLLBACK;
        bill::EwalletCloseTransitParams* ectp =
            new bill::EwalletCloseTransitParams(bpd.release(),&lcmCtx);
        lcmCtx.setParams(ectp);
        return true;
    } else {
        throw SCAGException("Action '%s': is not allowed for non-ewallet types");
    }
}


void BillActionCloseTransit::ContinueRunning( ActionContext& context )
{
    if ( ! preContinueRunning(context) ) return;
    // bill::BillCallParams* bp = static_cast<bill::BillCallParams*>(context.getSession().getLongCallContext().getParams());
    setBillingStatus(context,"",true);
}


void BillActionCloseTransit::postInit(const SectionParams& params,
                                      PropertyObject propertyObject)
{
    /*
    BillAction::init( params, propertyObject );

    if ( !params.Exists("action") )
        throw SCAGException( "Action '%s': missing 'action' parameter.", opname() );

    const std::string str = params["action"];

    actionCommit_ = ( str == "commit" );

    if ( !actionCommit_ && (str!="rollback") )
        throw SCAGException( "Action '%s': unrecognised 'action' parameter: '%s'",
                             opname(), str.c_str() );

    smsc_log_debug(logger,"Action '%s' init...", opname());
     */
}

}}}
