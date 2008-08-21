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
        smsc_log_error( logger, "Action '%s': invalid (empty) trans-id", opname() );
        setBillingStatus( context, "Empty trans-id", false );
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

        try {
            trans->info(bis, tr);
        } catch (SCAGException& e) {
            smsc_log_error(logger,"Action '%s': No transaction '%s' billid=%d in BM. Error: %s",
                           opname(), transId.c_str(), trans->billId(), e.what() );
            setBillingStatus( context, e.what(), false );
            return false;
        }

#ifdef MSAG_INMAN_BILL
        if ( tr.billType == scag::bill::infrastruct::INMAN )
        {
            BillCloseCallParams* bp = new BillCloseCallParams();
            bp->BillId = trans->billId();
            LongCallContext& lcmCtx = context.getSession().getLongCallContext();
            lcmCtx.callCommandId = actionCommit_ ? BILL_COMMIT : BILL_ROLLBACK;
            lcmCtx.setParams(bp);
            return true;
        }
        else
        {
#endif
            try {
                if (actionCommit_)
                    trans->commit();
                else
                    trans->rollback();
            } catch (SCAGException& e)
            {        
                smsc_log_error(logger,"Action '%s' error. Delails: %s", opname(), e.what());
                setBillingStatus( context, e.what(), false );
                return false;
            }
        }
#ifdef MSAG_INMAN_BILL
    }
#endif
    setBillingStatus(context,"", true);
    // if (op && op->getBillId() == bid) op->detachBill();
    // FIXME: should we extract transaction just before trans->commit/rollback ?
    context.getSession().releaseTransaction( transId.c_str() );
    return false;
}


void BillActionClose::ContinueRunning( ActionContext& context )
{
    smsc_log_debug(logger, "ContinueExecution Action '%s'...", opname());
    
    bill::BillCloseCallParams *bp = 
        (bill::BillCloseCallParams*)context.getSession().getLongCallContext().getParams();

    if ( bp->exception.length() )
    {        
        smsc_log_error( logger, "BillAction '%s' error. Delails: %s",
                        opname(), bp->exception.c_str());
        setBillingStatus(context, bp->exception.c_str(), false);
        return;
    }   

    // FIXME: should we check things?
    const std::string transId = getTransId( context );
    context.getSession().releaseTransaction( transId.c_str() );
    /*
    Operation *op = context.GetCurrentOperation();
    if(!op || !op->hasBill())
    {
        const char *p = !op ? "Bill: Operation from ActionContext is invalid" : "Bill is not attached to operation";
        smsc_log_error(logger, p);
        SetBillingStatus(context, p, false);
        return;
    }
    SetBillingStatus(context,"", true);
    if(op && bp->BillId == op->getBillId()) op->detachBill();
     */
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
