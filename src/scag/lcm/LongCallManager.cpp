/* $Id$ */

#include <core/threads/ThreadPool.hpp>
#include <core/synchronization/EventMonitor.hpp>
#include <scag/util/singleton/Singleton.h>
#include "scag/config/ConfigManager.h"
#include "scag/config/ConfigListener.h"
#include "scag/pers/PersClient.h"
#include "scag/pers/Types.h"
#include "scag/bill/BillingManager.h"
#include "scag/exc/SCAGExceptions.h"

#include "LongCallManager.h"

namespace scag { namespace lcm {

using namespace scag::util::singleton;
using smsc::util::Exception;
using smsc::logger::Logger;
using namespace smsc::core::threads;
using namespace scag::exceptions;

//using namespace scag::pers::client;
//using namespace scag::pers;
using scag::bill::BillOpenCallParams;
using scag::bill::BillCloseCallParams;

bool  LongCallManager::inited = false;
Mutex LongCallManager::initLock;

class LongCallManagerImpl: public LongCallManager, public ConfigListener{
public:
    LongCallManagerImpl(): ConfigListener(LONGCALLMAN_CFG), stopped(true) {};
    ~LongCallManagerImpl() {};
    
    LongCallContext* getContext();
    bool call(LongCallContext* context);
    void init(uint32_t _maxThreads);
    void shutdown();
    
protected:
    bool stopped;
    Logger * logger;
    EventMonitor mtx;
    uint32_t maxThreads;
    ThreadPool pool;
    LongCallContext *headContext, *tailContext;
    
    void configChanged();
};

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


inline unsigned GetLongevity(LongCallManager*) { return 5; }
typedef SingletonHolder<LongCallManagerImpl> SingleLCM;

LongCallManager& LongCallManager::Instance()
{
    if (!LongCallManager::inited)
    {
        MutexGuard guard(LongCallManager::initLock);
        if (!LongCallManager::inited) 
            throw std::runtime_error("LongCallManager not inited!");
    }
    return SingleLCM::Instance();
}

void LongCallManager::Init(uint32_t maxthr)
{
    if (!LongCallManager::inited)
    {
        MutexGuard guard(LongCallManager::initLock);
        if(!inited) {
            LongCallManagerImpl& lcm = SingleLCM::Instance();
            lcm.init(maxthr);
            LongCallManager::inited = true;
        }
    }
}

void LongCallManager::Init(const LongCallManagerConfig& cfg)
{
    if (!LongCallManager::inited)
    {
        MutexGuard guard(LongCallManager::initLock);
        if(!inited) {
            LongCallManagerImpl& lcm = SingleLCM::Instance();
            lcm.init(cfg.maxThreads);
            LongCallManager::inited = true;
        }
    }
}

void LongCallManager::shutdown()
{
    SingleLCM::Instance().shutdown();
}

void LongCallManagerImpl::init(uint32_t maxThr)
{
    logger = Logger::getInstance("lcm");
    headContext = NULL;
    maxThreads = maxThr;
    for(int i = 0; i < maxThreads; i++)
        pool.startTask(new LongCallTask(this));

    MutexGuard mt(mtx);        
    stopped = false;
}

void LongCallManagerImpl::configChanged()
{
    LongCallManagerConfig& cfg = ConfigManager::Instance().getLongCallManConfig();
    
}

void LongCallManagerImpl::shutdown()
{
    smsc_log_debug(logger, "shutdown");
    mtx.Lock();
    stopped = true;
    LongCallContext *ctx;
    while(headContext)
    {
        ctx = headContext;
        ctx->initiator->continueExecution(ctx, true);
        headContext = headContext->next;
        delete ctx;
    }
    mtx.notifyAll();
    mtx.Unlock();
    pool.shutdown();
}

LongCallContext* LongCallManagerImpl::getContext()
{
    LongCallContext *ctx;
    MutexGuard mt(mtx);
    if(stopped) return NULL;
    if(!headContext) mtx.wait();
    ctx = headContext;    
    if(headContext) headContext = headContext->next;
    return ctx;        
}

bool LongCallManagerImpl::call(LongCallContext* context)
{
    if(context->callCommandId == BILL_OPEN || context->callCommandId == BILL_COMMIT || context->callCommandId == BILL_ROLLBACK)
    {
        try{
            if(context->callCommandId == BILL_OPEN)
            {
                BillOpenCallParams * bp = (BillOpenCallParams*)context->getParams();
                scag::bill::BillingManager::Instance().Open(bp->billingInfoStruct, bp->tariffRec, context);
            }
            else if(context->callCommandId == BILL_COMMIT)
            {
                BillCloseCallParams * bp = (BillCloseCallParams*)context->getParams();
                scag::bill::BillingManager::Instance().Commit(bp->BillId, context);
            }
            else if(context->callCommandId == BILL_ROLLBACK)
            {
                BillCloseCallParams * bp = (BillCloseCallParams*)context->getParams();
                scag::bill::BillingManager::Instance().Rollback(bp->BillId, context);
            }
        }
        catch(SCAGException& e)
        {
            LongCallParams *lp = (LongCallParams*)context->getParams();            
            lp->exception = e.what();
            context->initiator->continueExecution(context, false);
        }
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

void LongCallTask::ExecutePersCall(LongCallContext* ctx)
{
/*    PersCallParams* persParams = (PersCallParams*)ctx->getParams();
    PersClient& pc = PersClient::Instance();
    smsc_log_debug(logger, "ExecutePersCall: command=%d %s", ctx->callCommandId, persParams->skey.c_str());
    try{
        if(persParams->pt == PT_ABONENT)        
            switch(ctx->callCommandId)
            {
                case PERS_GET: pc.GetProperty(persParams->pt, persParams->skey.c_str(), persParams->propName.c_str(), persParams->prop); break;
                case PERS_SET: pc.SetProperty(persParams->pt, persParams->skey.c_str(), persParams->prop); break;
                case PERS_DEL: pc.DelProperty(persParams->pt, persParams->skey.c_str(), persParams->propName.c_str()); break;
                case PERS_INC_MOD: persParams->result = pc.IncModProperty(persParams->pt, persParams->skey.c_str(), persParams->prop, persParams->mod); break;
                case PERS_INC: pc.IncProperty(persParams->pt, persParams->skey.c_str(), persParams->prop); break;
            }
        else
            switch(ctx->callCommandId)
            {
                case PERS_GET: pc.GetProperty(persParams->pt, persParams->ikey, persParams->propName.c_str(), persParams->prop); break;
                case PERS_SET: pc.SetProperty(persParams->pt, persParams->ikey, persParams->prop); break;
                case PERS_DEL: pc.DelProperty(persParams->pt, persParams->ikey, persParams->propName.c_str()); break;
                case PERS_INC_MOD: persParams->result = pc.IncModProperty(persParams->pt, persParams->ikey, persParams->prop, persParams->mod); break;
                case PERS_INC: pc.IncProperty(persParams->pt, persParams->ikey, persParams->prop); break;
            }
        persParams->error = 0;
        persParams->exception.assign("");
    }
    catch(PersClientException& exc)
    {
        persParams->error = exc.getType();
        persParams->exception = exc.what();        
    }
    catch(Exception& exc)
    {
        persParams->error = 0;        
        persParams->exception = exc.what();
    }
    catch(...)
    {
        persParams->error = 0;        
        persParams->exception = "LongCallManager: Unknown exception";
    }*/
}
    
int LongCallTask::Execute()
{
    LongCallContext* ctx;
    
    while(!isStopping)
    {
        ctx = manager->getContext();
        
        if(isStopping || !ctx) break;

        if(ctx->callCommandId >= PERS_GET && ctx->callCommandId <= PERS_INC)
            ExecutePersCall(ctx);
            
        ctx->initiator->continueExecution(ctx, false);
    }
    return 0;
}

}}

