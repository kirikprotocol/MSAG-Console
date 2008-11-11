/* $Id$ */

#include "LongCallManagerImpl.h"

#include "scag/bill/base/BillingManager.h"
#include "scag/config/base/ConfigManager2.h"
#include "scag/exc/SCAGExceptions.h"
#include "scag/pers/util/PersClient2.h"
#include "scag/pers/util/Types.h"
#include "scag/re/base/ActionContext2.h"

namespace scag2 {
namespace lcm {

using smsc::util::Exception;
using smsc::logger::Logger;
using namespace smsc::core::threads;
using namespace scag::exceptions;
using namespace scag2::config;

using namespace pers::util;
using bill::BillOpenCallParams;
using bill::BillCloseCallParams;


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
    if(stopped) return false;
    
    // smsc_log_debug( logger, "call context=%p session=%p cmdid=%d",
    // context, & context->getActionContext()->getSession(),
    // context->callCommandId );

    if (context->callCommandId == BILL_OPEN || context->callCommandId == BILL_COMMIT || context->callCommandId == BILL_ROLLBACK)
    {
        try {

            if(context->callCommandId == BILL_OPEN)
            {
                BillOpenCallParams * bp = (BillOpenCallParams*)context->getParams();
                bill::BillingManager::Instance().Open(bp->billingInfoStruct, bp->tariffRec, (LongCallContext*)context);
            }
            else if(context->callCommandId == BILL_COMMIT)
            {
                BillCloseCallParams * bp = (BillCloseCallParams*)context->getParams();
                bill::BillingManager::Instance().Commit(bp->BillId, (LongCallContext*)context);
            }
            else if(context->callCommandId == BILL_ROLLBACK)
            {
                BillCloseCallParams * bp = (BillCloseCallParams*)context->getParams();
                bill::BillingManager::Instance().Rollback(bp->BillId, (LongCallContext*)context);
            }
        }
        catch(SCAGException& e)
        {
            LongCallParams *lp = (LongCallParams*)context->getParams();            
            lp->exception = e.what();
            context->initiator->continueExecution(context, false);
        }

    }
    else if(context->callCommandId >= PERS_GET && context->callCommandId <= PERS_BATCH)
    {
        // PersCallParams* p = (PersCallParams*) context->getParams();
        if (!PersClient::Instance().call(context) ) return false;
    }
    else
    {
        MutexGuard mt(mtx);
        if(stopped) return false;
        context->next = NULL;

        if(headContext)
            tailContext->next = context;
        else
            headContext = context;
        tailContext = context;
        
        mtx.notify();        
    }
    return true;
}
    

int LongCallTask::Execute()
{
    LongCallContextBase* ctx;
    
    while(!isStopping)
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

}}
