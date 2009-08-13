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
        std::auto_ptr<bill::BillTransitParamsData> bctpd(new bill::BillTransitParamsData);

        bctpd->data.reset(bpd.release());

        // fill transid
        if ( ewalletTransType_ == ftUnknown ) {
            bctpd->transId = ewalletTransId_;
        } else {

            Property * property = context.getProperty(ewalletTransName_);
            if (!property)
            {
                setBillingStatus(context, "Invalid property for transId", false);
                // setTariffStatus( context, 0 );
                smsc_log_error(logger,"Action '%s' :: Invalid property %s for transId",
                               opname(), ewalletTransName_.c_str() );
                return false;
            }
            bctpd->transId = unsigned(property->getInt());
            
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
    bool bExist;
    ewalletTransType_ = CheckParameter( params,
                                        propertyObject,
                                        opname(),
                                        "transId",
                                        true,      // required
                                        true,       // readonly
                                        ewalletTransName_,
                                        bExist );
    if ( ewalletTransType_ == ftUnknown ) {
        ewalletTransId_ = atoi(ewalletTransName_.c_str());
    }

    if ( !params.Exists("action") )
        throw SCAGException( "Action '%s': missing 'action' parameter.", opname() );

    const std::string str = params["action"];

    actionCommit_ = ( str == "commit" );

    if ( !actionCommit_ && (str!="rollback") )
        throw SCAGException( "Action '%s': unrecognised 'action' parameter: '%s'",
                             opname(), str.c_str() );

    /*
    BillAction::init( params, propertyObject );

    smsc_log_debug(logger,"Action '%s' init...", opname());
     */
}

}}}
