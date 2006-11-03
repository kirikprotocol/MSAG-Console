/* $Id$ */

#include <core/threads/ThreadPool.hpp>
#include <core/synchronization/EventMonitor.hpp>
#include <scag/util/singleton/Singleton.h>
#include "scag/config/ConfigManager.h"
#include "scag/config/ConfigListener.h"

#include "LongCallManager.h"

namespace scag { namespace lcm {

using namespace scag::util::singleton;
using smsc::util::Exception;
using smsc::logger::Logger;
using namespace smsc::core::threads;

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
    EventMonitor mtx;
    uint32_t maxThreads;
    ThreadPool pool;
    LongCallContext *headContext, *tailContext;
    
    void configChanged();
};

class LongCallTask : public ThreadedTask
{
    LongCallManagerImpl* manager;
public:
    LongCallTask(LongCallManagerImpl* man) : manager(man) {};
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
    headContext = NULL;
    maxThreads = maxThr;
    for(int i = 0; i < maxThreads; i++)
        pool.startTask(new LongCallTask(this));
}

void LongCallManagerImpl::configChanged()
{
    LongCallManagerConfig& cfg = ConfigManager::Instance().getLongCallManagerConfig();
    
}

void LongCallManagerImpl::shutdown()
{
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

int LongCallTask::Execute()
{
    LongCallContext* ctx;
    
    while(!isStopping)
    {
        ctx = manager->getContext();
        
        if(isStopping || !ctx) break;
        
        switch(ctx->callCommandId)        
        {
            case PERS_GET:
                break;
            case PERS_SET:
                break;
            case PERS_DEL:
                break;
            case PERS_INC:
                break;
        }
    }
    return 0;    
}

}}
