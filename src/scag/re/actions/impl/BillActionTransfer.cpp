#include "BillActionTransfer.h"
#include "scag/bill/base/BillingManager.h"

namespace scag2 {
namespace re {
namespace actions {

// long call interface
void BillActionTransfer::init( const SectionParams& params, PropertyObject propertyObject )
{
    BillAction::init( params, propertyObject );

    // agentId = serviceId; from commandproperty
    // sourceId = "msag";

    bool bExist;
    abonentType_ = CheckParameter( params,
                                   propertyObject,
                                   opname(),
                                   "abonent",
                                   true,      // required
                                   true,       // readonly
                                   abonentName_,
                                   bExist );

    srcWalletType_ = CheckParameter( params,
                                     propertyObject,
                                     opname(),
                                     "srcWalletType",
                                     true,      // required
                                     true,       // readonly
                                     srcWalletName_,
                                     bExist );

    dstWalletType_ = CheckParameter( params,
                                     propertyObject,
                                     opname(),
                                     "dstWalletType",
                                     true,      // required
                                     true,       // readonly
                                     dstWalletName_,
                                     bExist );

    externalIdType_ = CheckParameter( params,
                                      propertyObject,
                                      opname(),
                                      "externalId",
                                      false,
                                      true,
                                      externalIdName_,
                                      hasExternalId_ );

    amountType_ = CheckParameter( params,
                                  propertyObject,
                                  opname(),
                                  "amount",
                                  true,
                                  true,
                                  amountName_,
                                  bExist );
    if ( amountType_ == ftUnknown ) {
        amount_ = atoi(amountName_.c_str());
    }

    descriptionType_ = CheckParameter( params,
                                       propertyObject,
                                       opname(),
                                       "description",
                                       false,
                                       true,
                                       descriptionName_,
                                       hasDescription_ );
}


bool BillActionTransfer::RunBeforePostpone( ActionContext& ctx )
{
    LongCallContext& lcmCtx = ctx.getSession().getLongCallContext();
    std::auto_ptr<bill::EwalletTransferCallParams> bp(new bill::EwalletTransferCallParams(&lcmCtx));
    try {
        bp->setAgentId( ctx.getCommandProperty().serviceId );
        bp->setUserId( getString(ctx,"abonent",abonentType_,abonentName_));
        bp->setSrcWalletType( getString(ctx,"srcWalletType",srcWalletType_,srcWalletName_));
        bp->setDstWalletType( getString(ctx,"dstWalletType",dstWalletType_,dstWalletName_));
        if (hasExternalId_)
            bp->setExternalId( getString(ctx,"externalId",externalIdType_,externalIdName_));
        bp->setAmount(getInt(ctx,"amount",amountType_,amountName_,amount_));
        if (hasDescription_)
            bp->setDescription(getString(ctx,"description",descriptionType_,descriptionName_));
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


std::string BillActionTransfer::getString( ActionContext& context,
                                           const char* fieldName,
                                           FieldType fieldType,
                                           const std::string& fieldValue )
{
    if ( fieldType == ftUnknown ) {
        return fieldValue;
    } else {
        Property* property = context.getProperty( fieldValue );
        if ( ! property ) {
            throw smsc::util::Exception("Invalid property %s for %s", fieldValue.c_str(), fieldName);
        }
        return property->getStr().c_str();
    }
}


int BillActionTransfer::getInt( ActionContext& context,
                                const char* fieldName,
                                FieldType fieldType,
                                const std::string& fieldValueStr,
                                int fieldValueInt )
{
    if ( fieldType == ftUnknown ) {
        return fieldValueInt;
    } else {
        Property* property = context.getProperty( fieldValueStr );
        if ( ! property ) {
            throw smsc::util::Exception("Invalid property %s for %s", fieldValueStr.c_str(), fieldName);
        }
        return atoi(property->getStr().c_str());
    }
}


} // namespace actions
} // namespace re
} // namespace scag2
