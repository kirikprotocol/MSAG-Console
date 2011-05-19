#include "BillActionClose.h"
#include "scag/sessions/base/ExternalBillingTransaction.h"
#include "scag/bill/base/Infrastructure.h"
#include "scag/bill/base/BillingInfoStruct.h"

namespace scag2 {
namespace re {
namespace actions {

bool BillActionClose::RunBeforePostpone(ActionContext& context)
{
    smsc_log_debug( logger, "Run Action '%s'...", opname() );

    const std::string transId = getTransId( context );
    if ( transId.empty() ) {
        // smsc_log_error( logger, "Action '%s': invalid (empty) trans-id", opname() );
        // setBillingStatus( context, "Empty trans-id", false );
        return false;
    }
    
    ExternalTransaction* etrans = context.getSession().getTransaction(transId.c_str());
    if ( ! etrans ) {
        smsc_log_error( logger, "Action '%s': transaction '%s' not found",
                        opname(), transId.c_str() );
        setBillingStatus( context, "transaction not found", false );
        return false;
    }

    ExternalBillingTransaction* trans = etrans->castToBilling();
    if ( ! trans ) {
        smsc_log_error( logger, "Action '%s': not a billing transaction '%s'",
                        opname(), transId.c_str() );
        setBillingStatus( context, "not a billing transaction", false );
        return false;
    } else {
        bill::BillingInfoStruct bis;
        bill::infrastruct::TariffRec tr;

        smsc_log_debug( logger, "Action '%s': transid=%s billid=%lld",
                        opname(), transId.c_str(), trans->billId() );

        try {
            trans->info(bis, tr);
        } catch (SCAGException& e) {
            smsc_log_error(logger,"Action '%s': No transaction '%s' billid=%lld in BM. Error: %s",
                           opname(), transId.c_str(), trans->billId(), e.what() );
            setBillingStatus( context, e.what(), false );
            return false;
        }

#ifdef MSAG_INMAN_BILL
        if ( tr.billType == bill::infrastruct::INMAN )
        {
            bill::InmanCloseCallParams* bp = new bill::InmanCloseCallParams(trans->billId(), true); //GVR: verify timeout arg!
            // bp->BillId = trans->billId();
            LongCallContext& lcmCtx = context.getSession().getLongCallContext();
            lcmCtx.callCommandId = actionCommit_ ? BILL_COMMIT : BILL_ROLLBACK;
            lcmCtx.setParams(bp);
            return true;
        }
        else 
#endif
        if ( tr.billType == bill::infrastruct::EWALLET ) {
            LongCallContext& lcmCtx = context.getSession().getLongCallContext();
            lcmCtx.callCommandId = actionCommit_ ? BILL_COMMIT : BILL_ROLLBACK;
            bill::EwalletCloseCallParams* bp = 
                new bill::EwalletCloseCallParams(trans->billId(),true,&lcmCtx);
            lcmCtx.setParams(bp);
            return true;
        } else {
            try {
                if (actionCommit_)
                    trans->commit();
                else
                    trans->rollback(false);
            } catch (SCAGException& e)
            {        
                smsc_log_error(logger,"Action '%s' error. Delails: %s", opname(), e.what());
                setBillingStatus( context, e.what(), false );
                return false;
            }
        }
    }
    setBillingStatus(context,"", true);
    // if (op && op->getBillId() == bid) op->detachBill();
    context.getSession().releaseTransaction( transId.c_str() );
    return false;
}


void BillActionClose::ContinueRunning( ActionContext& context )
{
    smsc_log_debug(logger, "ContinueExecution Action '%s'...", opname());
    
    bill::BillCallParams *bp = 
        static_cast<bill::BillCallParams*>(context.getSession().getLongCallContext().getParams());

    if ( !bp->exception.empty() )
    {        
        smsc_log_error( logger, "BillAction '%s' error. Delails: %s",
                        opname(), bp->exception.c_str());
        setBillingStatus(context, bp->exception.c_str(), false);
        return;
    }   

    const std::string transId = getTransId( context );
    context.getSession().releaseTransaction( transId.c_str() );
}


void BillActionClose::init( const SectionParams& params,PropertyObject propertyObject)
{
    BillAction::init( params, propertyObject );

    if ( !params.Exists("action") )
        throw SCAGException( "Action '%s': missing 'action' parameter.", opname() );

    const std::string str = params["action"];

    actionCommit_ = ( str == "commit" );

    if ( !actionCommit_ && (str!="rollback") )
        throw SCAGException( "Action '%s': unrecognised 'action' parameter: '%s'",
                             opname(), str.c_str() );

    smsc_log_debug(logger,"Action '%s' init...", opname());
}

}}}
