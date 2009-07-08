#include "BillActionTransfer.h"
#include "scag/bill/base/BillingManager.h"

namespace scag2 {
namespace re {
namespace actions {

// long call interface
void BillActionTransfer::init( const SectionParams& params, PropertyObject propertyObject )
{
    BillAction::init( params, propertyObject );
    abonent_.init( params, propertyObject );
    walletType_.init( params, propertyObject );
    dstWalletType_.init( params, propertyObject );
    externalId_.init( params, propertyObject );
    amount_.init( params, propertyObject );
    description_.init( params, propertyObject );
}


bool BillActionTransfer::RunBeforePostpone( ActionContext& ctx )
{
    LongCallContext& lcmCtx = ctx.getSession().getLongCallContext();
    std::auto_ptr<bill::EwalletTransferCallParams> bp(new bill::EwalletTransferCallParams(&lcmCtx));
    try {
        bp->setAgentId( ctx.getCommandProperty().serviceId );
        bp->setUserId( abonent_.getValue(ctx));
        bp->setSrcWalletType( walletType_.getValue(ctx));
        bp->setDstWalletType( dstWalletType_.getValue(ctx));
        if (externalId_.isFound())
            bp->setExternalId( externalId_.getValue(ctx));
        bp->setAmount(amount_.getValue(ctx));
        if (description_.isFound())
            bp->setDescription(description_.getValue(ctx));
    } catch ( std::exception& e ) {
        smsc_log_warn(logger,"exc in %s: %s", opname(),e.what());
        setBillingStatus(ctx,e.what(),false);
        return false;
    }
    lcmCtx.callCommandId = BILL_TRANSFER;
    lcmCtx.setParams(bp.release());
    return true;
}


void BillActionTransfer::ContinueRunning( ActionContext& context )
{
    bill::BillCallParams *bp = static_cast<bill::BillCallParams*>(context.getSession().getLongCallContext().getParams());
    if ( ! bp->exception.empty() )
    {
        smsc_log_warn(logger, "Action '%s' unable to process. Details: %s",
                      opname(), bp->exception.c_str());
        setBillingStatus(context, bp->exception.c_str(), false);
        // setTariffStatus(context,0);
        return;
    }
}


} // namespace actions
} // namespace re
} // namespace scag2
