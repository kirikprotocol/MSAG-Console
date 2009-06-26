#include <memory>
#include "BillActionOpen.h"
#include "scag/re/base/CommandAdapter2.h"
#include "scag/bill/base/BillingManager.h"
#include "scag/stat/base/Statistics2.h"
#include "scag/sessions/base/ExternalBillingTransaction.h"
#include "scag/util/Time.h"

namespace scag2 {
namespace re {
namespace actions {

using namespace bill;

BillActionOpen::BillActionOpen( bool transit ) :
BillActionPreOpen( transit )
{
}


BillOpenCallParamsData* BillActionOpen::postFillParamsData( BillOpenCallParamsData* data, ActionContext& context )
{
    std::auto_ptr< BillOpenCallParamsData > bpd(data);
    if ( bpd->tariffRec.billType == infrastruct::EWALLET ) {

        BillingInfoStruct& billingInfoStruct = bpd->billingInfoStruct;
        // description
        if ( hasDescription_ ) {
            if ( descriptionType_ == ftUnknown ) {
                billingInfoStruct.description = descriptionName_;
            } else {
                Property * property = context.getProperty(descriptionName_);
                if (!property) {
                    setBillingStatus( context, "Invalid property for description", false );
                    setTariffStatus( context, 0 );
                    smsc_log_error(logger,"Action '%s' :: Invalid property %s for description",
                                   opname(), descriptionName_.c_str() );
                    return 0;
                }
                billingInfoStruct.description = property->getStr().c_str();
            }
        }

        // timeout
        if ( hasTimeout_ ) {
            if ( timeoutFieldType_ == ftUnknown ) {
                billingInfoStruct.timeout = timeout_;
            } else {
                Property * property = context.getProperty(timeoutFieldName_);
                if (!property) {
                    setBillingStatus( context, "Invalid property for timeout", false );
                    setTariffStatus( context, 0 );
                    smsc_log_error(logger,"Action '%s' :: Invalid property %s for timeout",
                                   opname(), timeoutFieldName_.c_str() );
                    return 0;
                }
                billingInfoStruct.timeout = property->getInt();
            }
        }
    }
    return bpd.release();
}


bool BillActionOpen::RunBeforePostpone( ActionContext& context )
{
    auto_ptr<BillOpenCallParamsData> bpd(makeParamsData(context));
    if ( ! bpd.get() ) return false;

    if ( bpd->tariffRec.billType == infrastruct::EWALLET ) {
        LongCallContext& lcmCtx = context.getSession().getLongCallContext();
        lcmCtx.callCommandId = BILL_OPEN;
        EwalletOpenCallParams* eocp = 
            new EwalletOpenCallParams(isTransit(),bpd.release(),&lcmCtx);
        smsc_log_debug(logger,"eocp created @ %p, bocp=%p, lcp=%p",
                       eocp->getOpen(), static_cast<lcm::LongCallParams*>(eocp) );
        lcmCtx.setParams(eocp);
        smsc_log_debug(logger,"lcm ewallet params created: %p", lcmCtx.getParams() );
        return true;
    } else
#ifdef MSAG_INMAN_BILL
    if( bpd->tariffRec.billType == infrastruct::INMAN )
    {
        if ( isTransit() ) {
            throw SCAGException("Action '%s': transit is not allowed for non-ewallet type", opname());
        }

        LongCallContext& lcmCtx = context.getSession().getLongCallContext();
        lcmCtx.callCommandId = BILL_OPEN;
        lcmCtx.setParams( new InmanOpenCallParams(bpd.release()) );
        return true;
    } else
#endif
    {
        if ( isTransit() ) {
            throw SCAGException("Action '%s': transit is not allowed for non-ewallet type", opname());
        }

        try 
        {
            InmanOpenCallParams bp(bpd.release());
            billid_type bi = BillingManager::Instance().Open(bp);
            processResult( context, bi, bp.tariffRec() );
        }
        catch (SCAGException& e)
        {
            smsc_log_warn(logger, "Action '%s' unable to process. Delails: %s",
                          opname(), e.what());
            setBillingStatus(context, e.what(), false );
            setTariffStatus(context,0);
        }
    }
    return false;
}


void BillActionOpen::ContinueRunning(ActionContext& context)
{
    if ( ! preContinueRunning(context) ) return;

    BillCallParams *bp = static_cast<BillCallParams*>(context.getSession().getLongCallContext().getParams());
    BillOpenCallParams* bop = bp->getOpen();
    if ( ! isTransit() ) {
        processResult( context, bop->billId(), bop->tariffRec() );
    } else {
        // transit
        smsc_log_debug(logger,"Action '%s': not registering session transaction %llu as it is transit", bop->billId());
        assert( bop->tariffRec()->billType == infrastruct::EWALLET );
        EwalletCallParams* ecp = static_cast<EwalletCallParams*>(bp);
        char buf[30];
        sprintf(buf,"%u",ecp->getTransId());
        setBillingStatus(context,buf,true);
        setTariffStatus(context,bop->tariffRec());
    }
}


void BillActionOpen::postInit( const SectionParams& params,
                               PropertyObject propertyObject )
{
    timeoutFieldType_ = CheckParameter( params, 
                                        propertyObject, 
                                        opname(), "timeout",
                                        false, true,
                                        timeoutFieldName_,
                                        hasTimeout_ );
    timeout_ = 0;
    if ( hasTimeout_ && (timeoutFieldType_ == ftUnknown) ) {
        timeout_ = atoi(timeoutFieldName_.c_str());
        if (!timeout_) throw SCAGException("Action '%s': timeout should be integer or variable",
                                           opname() );
    }

    descriptionType_ = CheckParameter( params,
                                       propertyObject,
                                       opname(), "description",
                                       false, true,
                                       descriptionName_,
                                       hasDescription_ );

    if ( isTransit() && ! hasMessage() ) {
        throw SCAGException("Action '%s': transit action requires 'msg' to save transId", opname());
    }

    // --- output fields

    CheckParameter( params,
                    propertyObject,
                    opname(), "result_number",
                    false, false,
                    resultFieldName_,
                    hasResult_ );
}


void BillActionOpen::setTariffStatus( ActionContext&   context,
                                      const TariffRec* tariffRec )
{
    if (hasResult_) 
    {
        Property * propertyResNum = context.getProperty(resultFieldName_);

        if (!propertyResNum)
            smsc_log_debug(logger,"Action '%s' :: Invalid property %s for result_number",
                           opname(), resultFieldName_.c_str());
        else
            propertyResNum->setStr( tariffRec ? tariffRec->ServiceNumber.c_str() : "");
    }
}


void BillActionOpen::processResult( ActionContext& context,
                                    billid_type billId, 
                                    const infrastruct::TariffRec* tariffRec)
{
    std::string transId = getTransId( context );
    smsc_log_debug( logger, "Action '%s': process result trans-id='%s' billid=%llu", 
                    opname(), transId.c_str(), billId );
    std::auto_ptr<ExternalTransaction> trans( new sessions::ExternalBillingTransaction( billId ) );
    if ( ! context.getSession().addTransaction( transId.c_str(), trans ) ) {
        smsc_log_error( logger, "Action '%s': cannot put transaction '%s' in session",
                        opname(), transId.c_str() );
        setBillingStatus( context, "Session::addTransaction failed", false);
        setTariffStatus( context, 0 );
        return;
    }

    setBillingStatus( context, "", true );
    setTariffStatus( context, tariffRec );
    smsc_log_debug( logger, "Action '%s' transaction '%s' successfully opened (billId=%llu)",
                    opname(), transId.c_str(), billId );
    return;
}

}}}
