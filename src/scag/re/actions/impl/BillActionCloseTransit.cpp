#include "BillActionCloseTransit.h"

namespace scag2 {
namespace re {
namespace actions {

bill::BillOpenCallParamsData* BillActionCloseTransit::
postFillParamsData( bill::BillOpenCallParamsData* bp, ActionContext& context )
{
    if ( ! bp ) return bp;
    std::auto_ptr< bill::BillOpenCallParamsData > bpd(bp);
    if ( amount_.isFound() ) {
        const char* val = amount_.getValue(context);
        smsc_log_debug(logger,"tariff amount replaced: %s",val);
        bpd->tariffRec.setPrice(val);
    }
    return bpd.release();
}



bool BillActionCloseTransit::RunBeforePostpone(ActionContext& context)
{
    std::auto_ptr<bill::BillOpenCallParamsData> bpd(makeParamsData(context));
    if (!bpd.get()) return false;

    if (bpd->tariffRec.billType == bill::infrastruct::EWALLET ) {

        LongCallContext& lcmCtx = context.getSession().getLongCallContext();
        lcmCtx.callCommandId = actionCommit_ ? BILL_COMMIT : BILL_ROLLBACK;
        std::auto_ptr<bill::BillTransitParamsData> bctpd(new bill::BillTransitParamsData);

        bctpd->data.reset(bpd.release());

        // fill transid
        if ( transId_.isFound() ) {
            bctpd->transId = unsigned(transId_.getValue(context));
        } else if ( bctpd->data->billingInfoStruct.externalId.empty() ) {
            setBillingStatus(context, "transid/externalId should be specified", false);
            setTariffStatus(context,0);
            return false;
        }
        bill::EwalletCloseCallParams* ectp =
            new bill::EwalletCloseCallParams(bctpd.release(),&lcmCtx);
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
    if ( !params.Exists("action") )
        throw SCAGException( "Action '%s': missing 'action' parameter.", opname() );

    const std::string str = params["action"];

    actionCommit_ = ( str == "commit" );

    if ( !actionCommit_ && (str!="rollback") )
        throw SCAGException( "Action '%s': unrecognised 'action' parameter: '%s'",
                             opname(), str.c_str() );

    transId_.init(params,propertyObject);
    amount_.init(params,propertyObject);
}

}}}
