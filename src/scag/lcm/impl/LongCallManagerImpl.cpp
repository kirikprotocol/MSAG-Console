/* $Id$ */

#include "LongCallManagerImpl.h"
#include "scag/bill/base/BillingManager.h"
#include "scag/config/base/ConfigManager2.h"
#include "scag/exc/SCAGExceptions.h"
#include "scag/pvss/base/PersClient.h"
#include "scag/pvss/base/Types.h"
#include "scag/re/base/ActionContext2.h"
#include "scag/re/base/PersCallParams.h"
#include "scag/bill/ewallet/Exception.h"

namespace scag2 {
namespace lcm {

using smsc::util::Exception;
using smsc::logger::Logger;
using namespace smsc::core::threads;
using namespace scag::exceptions;
using namespace scag2::config;
using bill::BillOpenCallParams;
using bill::BillCloseCallParams;
using smsc::core::synchronization::MutexGuard;

class LongCallTask : public ThreadedTask
{
    LongCallManagerImpl* manager;
    Logger * logger;    
public:
    LongCallTask(LongCallManagerImpl* man) : manager(man) { logger = Logger::getInstance("lcm.task"); };
    void ExecutePersCall(LongCallContext* ctx);
    void ExecuteBillOpenCall(LongCallContext* ctx);
    void ExecuteBillCloseCall(LongCallContext* ctx);
    
    int Execute();
    const char* taskName() { return "LongCallTask"; };
};



void LongCallManagerImpl::init(uint32_t maxThr)
{
    {
        MutexGuard mt(mtx);
        if ( !stopped ) return;
    }

    logger = Logger::getInstance("lcm");
    headContext = NULL;
    maxThreads = maxThr;
    for (unsigned i = 0; i < maxThreads; i++)
        pool.startTask(new LongCallTask(this));

    MutexGuard mt(mtx);        
    stopped = false;
}


void LongCallManagerImpl::configChanged()
{
    // FIXME: do we need a reaction on config change?
    // LongCallManagerConfig& cfg = ConfigManager::Instance().getLongCallManConfig();
}

void LongCallManagerImpl::shutdown()
{
    smsc_log_info(logger, "shutting down");
    {
        MutexGuard mg(mtx);
        stopped = true;
        LongCallContextBase *ctx;
        while(headContext)
        {
            ctx = headContext;
            ctx->initiator->continueExecution(ctx, true);
            headContext = headContext->next;
            delete ctx;
        }
        mtx.notifyAll();
    }
    pool.shutdown();
    smsc_log_info(logger, "shutdown");    
}


LongCallContextBase* LongCallManagerImpl::getContext()
{
    MutexGuard mt(mtx);
    if(stopped) return NULL;
    if(!headContext) mtx.wait();
    LongCallContextBase *ctx = headContext;    
    if (headContext) {
        headContext = headContext->next;
    }
    return ctx;        
}


bool LongCallManagerImpl::call(LongCallContextBase* context)
{
    bool res = false;
    while ( !stopped ) {
    
        // smsc_log_debug( logger, "call context=%p session=%p cmdid=%d",
        // context, & context->getActionContext()->getSession(),
        // context->callCommandId );

        if ( context->callCommandId == BILL_OPEN ||
             context->callCommandId == BILL_COMMIT ||
             context->callCommandId == BILL_ROLLBACK ||
             context->callCommandId == BILL_CHECK ||
             context->callCommandId == BILL_TRANSFER ||
             context->callCommandId == BILL_INFO ) {

            try {
                if(context->callCommandId == BILL_OPEN)
                {
                    bill::BillCallParams* bcp = static_cast<bill::BillCallParams*>(context->getParams());
                    bill::BillOpenCallParams * bp = bcp->getOpen();
                    bill::BillingManager::Instance().Open( *bp, (LongCallContext*)context);
                }
                else if(context->callCommandId == BILL_COMMIT)
                {
                    bill::BillCallParams* bcp = static_cast<bill::BillCallParams*>(context->getParams());
                    bill::BillCloseCallParams * bp = bcp->getClose();
                    if ( bp->getTransitData() ) {
                        // transit
                        bill::BillingManager::Instance().CommitTransit(*bp,static_cast<LongCallContext*>(context));
                    } else {
                        bill::BillingManager::Instance().Commit(bp->billId(), (LongCallContext*)context);
                    }
                }
                else if(context->callCommandId == BILL_ROLLBACK)
                {
                    bill::BillCallParams* bcp = static_cast<bill::BillCallParams*>(context->getParams());
                    bill::BillCloseCallParams * bp = bcp->getClose();
                    if ( bp->getTransitData() ) {
                        // transit
                        bill::BillingManager::Instance().RollbackTransit(*bp,static_cast<LongCallContext*>(context));
                    } else {
                        bill::BillingManager::Instance().Rollback(bp->billId(), bp->isTimeout(), (LongCallContext*)context);
                    }
                } else if ( context->callCommandId == BILL_CHECK ) {
                    bill::BillCallParams* bcp = static_cast<bill::BillCallParams*>(context->getParams());
                    bill::BillCheckCallParams* bp = bcp->getCheck();
                    bill::BillingManager::Instance().Check(*bp,static_cast<LongCallContext*>(context));
                } else if ( context->callCommandId == BILL_TRANSFER ) {
                    bill::BillCallParams* bcp = static_cast<bill::BillCallParams*>(context->getParams());
                    bill::BillTransferCallParams* bp = bcp->getTransfer();
                    bill::BillingManager::Instance().Transfer(*bp,static_cast<LongCallContext*>(context));
                } else if ( context->callCommandId == BILL_INFO ) {
                    bill::EwalletInfoCallParams* bcp = static_cast<bill::EwalletInfoCallParams*>(context->getParams());
                    bill::BillingManager::Instance().Info(*bcp,static_cast<LongCallContext*>(context));
                }
            } catch( bill::ewallet::Exception& e ) {
                smsc_log_warn(logger,"exc: %s type=%s", e.what(), bill::ewallet::Status::statusToString(e.getStatus()));
                bill::EwalletCallParams* lp = 
                    static_cast<bill::EwalletCallParams*>(context->getParams());
                lp->setStatus( e.getStatus(), e.what() );
                break;
            } catch(SCAGException& e) {
                smsc_log_warn(logger,"exc: %s", e.what());
                LongCallParams *lp = (LongCallParams*)context->getParams();
                lp->exception = e.what();
                // NOTE: we should not do continueExecution if longcall has failed!
                // This is because the initiator->continueExecution will typically put
                // the command to the transport queue and it may become being processed
                // in parallel in another state machine!
                // context->initiator->continueExecution(context, false);
                break;
            }

        }
        else if(context->callCommandId >= PERS_GET && context->callCommandId <= PERS_BATCH)
        {
            PersCallParams* p = (PersCallParams*) context->getParams();
            // pvss::PersCall* call = p->getPersCall();
            // call->setContext(context);
            if ( ! p->callPvss(context) ) {
                // pvss::PersClient::Instance().callAsync(call,*this) ) {
                // context->initiator->continueExecution(context, false);
                break;
            }
        }
        else
        {
            MutexGuard mt(mtx);
            if (stopped) break;
            context->next = NULL;

            if(headContext)
                tailContext->next = context;
            else
                headContext = context;
            tailContext = context;
            mtx.notify();        
        }
        res = true;
        break;
    }
    if ( ! res ) context->continueExec = false;
    return res;
}
    

int LongCallTask::Execute()
{
    LongCallContextBase* ctx;
    
    while (!isStopping)
    {
        ctx = manager->getContext();
        if(isStopping || !ctx) break;
        // presently do nothing
            
        // smsc_log_debug( logger, "pass context=%p session=%p cmdid=%d back to initiator",
        // ctx, & ctx->getActionContext()->getSession(),
        // ctx->callCommandId );

        ctx->initiator->continueExecution(ctx, false);
    }
//    smsc_log_debug(logger, "Stopped lcm task");
    return 0;
}


void LongCallManagerImpl::continuePersCall( pvss::PersCall* pc, bool drop )
{
    LongCallContextBase* context = (LongCallContextBase*)pc->context();
    context->initiator->continueExecution(context, drop);
}

}}
