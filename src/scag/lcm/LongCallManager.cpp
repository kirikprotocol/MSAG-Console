/* $Id$ */

#include <core/threads/ThreadPool.hpp>
#include <core/synchronization/EventMonitor.hpp>
#include <scag/util/singleton/Singleton.h>
#include "scag/config/ConfigManager.h"
#include "scag/config/ConfigListener.h"
#include "scag/pers/PersClient.h"
#include "scag/pers/Types.h"

#include "LongCallManager.h"

namespace scag { namespace lcm {

using namespace scag::util::singleton;
using smsc::util::Exception;
using smsc::logger::Logger;
using namespace smsc::core::threads;

using namespace scag::pers::client;
using namespace scag::pers;

bool  LongCallManager::inited = false;
Mutex LongCallManager::initLock;

class LongCallManagerImpl: public LongCallManager, public ConfigListener{
public:
    LongCallManagerImpl(): ConfigListener(LONGCALLMAN_CFG) {};
    ~LongCallManagerImpl() {};
    
    LongCallContext* getContext();
    void call(LongCallContext* context);
    void init(uint32_t _maxThreads);
    void shutdown();
    
protected:
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

void LongCallManager::Init(uint32_t maxthr) //throw(LongCallManagerException)
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

void LongCallManager::Init(const LongCallManagerConfig& cfg)// throw(LongCallManagerException);    
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

void LongCallManagerImpl::init(uint32_t maxThr)
{
    logger = Logger::getInstance("lcm");
    headContext = NULL;
    maxThreads = maxThr;
    for(int i = 0; i < maxThreads; i++)
        pool.startTask(new LongCallTask(this));
}

void LongCallManagerImpl::configChanged()
{
    LongCallManagerConfig& cfg = ConfigManager::Instance().getLongCallManConfig();
    
}

void LongCallManagerImpl::shutdown()
{
    smsc_log_debug(logger, "shutdown");
    pool.stopNotify();
    mtx.Lock();
    LongCallContext *cx;
    while(headContext)
    {
        cx = headContext;
        headContext = headContext->next;
        delete cx;
    }
    mtx.notifyAll();
    mtx.Unlock();
    pool.shutdown();    
}

LongCallContext* LongCallManagerImpl::getContext()
{
    LongCallContext *ctx = NULL;
    MutexGuard mt(mtx);
    if(!headContext) mtx.wait();
    if(headContext)
    {
        ctx = headContext;
        headContext = headContext->next;
    }
    return ctx;        
}

void LongCallManagerImpl::call(LongCallContext* context)
{
    MutexGuard mt(mtx);
    if(headContext)
        tailContext = tailContext->next = context;
    else
        headContext = tailContext = context;
    mtx.notify();        
}

void LongCallTask::ExecutePersCall(LongCallContext* ctx)
{
    PersCallParams* persParams = (PersCallParams*)ctx->param;
    PersClient& pc = PersClient::Instance();
    smsc_log_debug(logger, "ExecutePersCall: command=%d", ctx->callCommandId);
    try{
        if(persParams->pt == PT_ABONENT)        
            switch(ctx->callCommandId)
            {
                case PERS_GET: pc.GetProperty(persParams->pt, persParams->skey, persParams->propName.c_str(), persParams->prop); break;
                case PERS_SET: pc.SetProperty(persParams->pt, persParams->skey, persParams->prop); break;
                case PERS_DEL: pc.DelProperty(persParams->pt, persParams->skey, persParams->propName.c_str()); break;
                case PERS_INC_MOD: persParams->result = pc.IncModProperty(persParams->pt, persParams->skey, persParams->prop, persParams->mod); break;
                case PERS_INC: pc.IncProperty(persParams->pt, persParams->skey, persParams->prop); break;
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
    }
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
            
        ctx->initiator->continueExecution(ctx);
    }
    return 0;
}

}}
