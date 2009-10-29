#include "BillActionCheck.h"

namespace scag2 {
namespace re {
namespace actions {

bool BillActionCheck::RunBeforePostpone(ActionContext& context)
{
    std::auto_ptr<bill::BillOpenCallParamsData> bpd(makeParamsData(context));
    if (!bpd.get()) return false;

    if (bpd->tariffRec.billType == bill::infrastruct::EWALLET ) {

        LongCallContext& lcmCtx = context.getSession().getLongCallContext();
        lcmCtx.callCommandId = BILL_CHECK;

        std::auto_ptr<bill::BillTransitParamsData> bcpd(new bill::BillTransitParamsData);
        bcpd->data.reset(bpd.release());

        // fill transid
        if ( transId_.isFound() ) {
            bcpd->transId = unsigned(transId_.getValue(context));
        } else if ( bcpd->data->billingInfoStruct.externalId.empty() ) {
            setBillingStatus(context,"transId/externalId should be set",false);
            return false;
        }

        bill::EwalletCheckCallParams* ectp =
            new bill::EwalletCheckCallParams(bcpd.release(),&lcmCtx);
        lcmCtx.setParams(ectp);
        return true;
    } else {
        throw SCAGException("Action '%s': is not allowed for non-ewallet types");
    }
}


void BillActionCheck::ContinueRunning( ActionContext& context )
{
    if ( ! preContinueRunning(context) ) return;
    // success
    bill::EwalletCheckCallParams* bp = static_cast<bill::EwalletCheckCallParams*>(context.getSession().getLongCallContext().getParams());
    // char buf[30];
    // sprintf(buf,"%u",bp->getTransStatus());
    setBillingStatus(context,"",true);
    if ( txStatus_.isFound() ) {
        Property* p = txStatus_.getProperty(context);
        if (p) p->setInt(bp->getTransStatus());
    }
    if ( txAmount_.isFound() ) {
        Property* p = txAmount_.getProperty(context);
        if (p) p->setInt(bp->getTxAmount());
    }
    if ( txEndDate_.isFound() ) {
        Property* p = txEndDate_.getProperty(context);
        if (p) p->setInt(bp->getTxEndDate());
    }
}


void BillActionCheck::postInit(const SectionParams& params,
                               PropertyObject propertyObject)
{
    transId_.init(params,propertyObject);
    txStatus_.init(params,propertyObject);
    txAmount_.init(params,propertyObject);
    txEndDate_.init(params,propertyObject);

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
