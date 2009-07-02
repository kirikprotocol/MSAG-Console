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
        if ( hasTransId_ ) {
            if ( ewalletTransType_ == ftUnknown ) {
                bcpd->transId = ewalletTransId_;
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
                bcpd->transId = unsigned(property->getInt());
            }
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
    char buf[30];
    sprintf(buf,"%u",bp->getTransStatus());
    setBillingStatus(context,buf,true);
}


void BillActionCheck::postInit(const SectionParams& params,
                               PropertyObject propertyObject)
{
    ewalletTransType_ = CheckParameter( params,
                                        propertyObject,
                                        opname(),
                                        "transId",
                                        false,      // required
                                        true,       // readonly
                                        ewalletTransName_,
                                        hasTransId_ );
    if ( hasTransId_ ) {
        if ( ewalletTransType_ == ftUnknown ) {
            ewalletTransId_ = atoi(ewalletTransName_.c_str());
        }
    }

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
