#include <limits.h>
#include "Managers.h"
#include "HttpContext.h"
#include "IOTasks.h"
#include "ScagTask.h"
#include <scag/util/singleton/Singleton.h>

namespace scag { namespace transport { namespace http
{

using namespace scag::util::singleton;

bool HttpManager::inited = false;
Mutex HttpManager::initLock;

inline unsigned GetLongevity(HttpManager*) { return 251; }
typedef SingletonHolder<HttpManagerImpl> SingleHM;

HttpManager& HttpManager::Instance()
{
    if (!inited) 
    {
        MutexGuard guard(initLock);
        if (!inited) 
            throw std::runtime_error("HttpManager not inited!");
    }
    return SingleHM::Instance();
}

void HttpManager::Init(HttpProcessor& p, const HttpManagerConfig& conf)
{
    if (!inited)
    {
        MutexGuard guard(initLock);
        if(!inited) {
            HttpManagerImpl& m = SingleHM::Instance();
            m.init(p, conf);
            inited = true;
        }
    }
}

HttpManagerImpl::HttpManagerImpl() : scags(*this), licenseExpired(false), licenseFileCheckHour(0),
    readers(*this), writers(*this), acceptor(*this), ConfigListener(HTTPMAN_CFG), licenseCounter(10, 20)
{
    logger = Logger::getInstance("scag.http.manager");
    lastLicenseExpTest = 0;
}

void HttpManagerImpl::configChanged()
{
    shutdown();
    init(HttpProcessor::Instance(), ConfigManager::Instance().getHttpManConfig());
}

void HttpManagerImpl::init(HttpProcessor& p, const HttpManagerConfig& conf)
{
    this->cfg = conf;

    readers.init(cfg.readerPoolSize, cfg.readerSockets, "scag.http.reader");
    writers.init(cfg.writerPoolSize, cfg.writerSockets, "scag.http.writer");
    scags.init(cfg.scagPoolSize, cfg.scagQueueLimit, p);
    acceptor.init(cfg.host.c_str(), cfg.port);
    smsc_log_info(logger, "Http manager inited host=%s:%d", cfg.host.c_str(), cfg.port);
}

void HttpManagerImpl::shutdown()
{
    acceptor.shutdown();

    while(1)
    {
        if(!readers.canStop())
            smsc_log_info(logger, "Waiting readers to stop");
        else if(!writers.canStop())
            smsc_log_info(logger, "Waiting writers to stop");
        else if(!scags.canStop())
            smsc_log_info(logger, "Waiting scagtasks to stop");
        else
            break;
        sleep(1);
    }
        
    scags.shutdown();
    readers.shutdown();
    writers.shutdown();
    smsc_log_info(logger, "HttpManager shutdown");
}

bool HttpManagerImpl::isLicenseExpired() {
  time_t now = time(NULL);
  if (now - lastLicenseExpTest < 600) {
    return licenseExpired;
  }
  struct tm ltm;
  localtime_r(&now,&ltm);
  if (ltm.tm_hour != licenseFileCheckHour) {
    try {
      ConfigManager::Instance().checkLicenseFile();
    } catch (const std::runtime_error& ex) {
      smsc_log_error(logger, "check license file error: '%s'", ex.what());
    } catch (const Exception& ex) {
      smsc_log_error(logger, "check license file error: '%s'", ex.what());
    }
    licenseFileCheckHour = ltm.tm_hour;
  }
  if (now > ConfigManager::Instance().getLicense().expdate) {
    smsc_log_error(logger,"License expired");
    licenseExpired = true;
  } else {
    licenseExpired = false;
  }
  lastLicenseExpTest = now;
  return licenseExpired;
}

bool HttpManagerImpl::licenseThroughputLimitExceed() {
  int licLimit = ConfigManager::Instance().getLicense().maxhttp;
  int cntValue = licenseCounter.Get() / 10;
  if (cntValue > licLimit) {
    smsc_log_info(logger,"Request denied by license limitation:%d/%d", cntValue, licLimit);
    return true;
  }
  return false;
}

void HttpManagerImpl::incLicenseCounter() {
  licenseCounter.Inc();
}



ScagTaskManager::ScagTaskManager(HttpManagerImpl& m) : manager(m)
{
}

void ScagTaskManager::deleteQueue(HttpContext* pcx)
{
    HttpContext *cx;
    while (pcx) {
        cx = pcx;
        pcx = pcx->next;
        delete cx;
    }
}

void ScagTaskManager::shutdown()
{
    pool.shutdown();
    
    {
        HttpContext *cx, *pcx;
        MutexGuard g(procMut);
        
        deleteQueue(headContext[PROCESS_REQUEST]);
        deleteQueue(headContext[PROCESS_RESPONSE]);
        deleteQueue(headContext[PROCESS_STATUS_RESPONSE]);
        deleteQueue(headContext[PROCESS_LCM]);        
    }
}

void ScagTaskManager::process(HttpContext* cx, bool continued)
{
    MutexGuard g(procMut);

    cx->next = NULL;
    uint32_t i = continued ? PROCESS_LCM : cx->action;
    if(headContext[i])
        tailContext[i]->next = cx;
    else        
        headContext[i] = cx;
    tailContext[i] = cx;        
    
    ++queueLength[i];
    if (queueLength[PROCESS_REQUEST] > scagQueueLimit)
        waitQueueShrinkage = true;

    {
        MutexGuard q(taskMon);
    
        taskMon.notify();   
    }
}

void ScagTaskManager::continueExecution(LongCallContext* context, bool dropped)
{
    HttpContext *cx = (HttpContext*)context->stateMachineContext;
    context->continueExec = true;

    if(!dropped)
        process(cx, true);
    else
        delete cx;
}

void ScagTaskManager::init(int maxThreads, int scagQueueLim, HttpProcessor& p)
{
    int i;
 
    queueLength[PROCESS_LCM] = 0; 
    queueLength[PROCESS_REQUEST] = 0;
    queueLength[PROCESS_RESPONSE] = 0;
    queueLength[PROCESS_STATUS_RESPONSE] = 0;
    
    scagQueueLimit = scagQueueLim;
    waitQueueShrinkage = false;
    
    headContext[PROCESS_REQUEST] = NULL;
    headContext[PROCESS_RESPONSE] = NULL;
    headContext[PROCESS_STATUS_RESPONSE] = NULL;
    headContext[PROCESS_LCM] = NULL;    

    logger = Logger::getInstance("scag.http.scag");

    pool.setMaxThreads(maxThreads);

    for (i = 0; i < maxThreads; i++)
        pool.startTask(new ScagTask(manager, p));
}

HttpContext *ScagTaskManager::getFirst()
{
    uint32_t i;
    HttpContext *cx;
    MutexGuard g(procMut);

    if(headContext[PROCESS_LCM])
        i = PROCESS_LCM;
    else if(headContext[PROCESS_STATUS_RESPONSE])
        i = PROCESS_STATUS_RESPONSE;        
    else if(headContext[PROCESS_RESPONSE])
        i = PROCESS_RESPONSE;
    else if(headContext[PROCESS_REQUEST])
        i = PROCESS_REQUEST;
    else
        return NULL;            
    
    cx = headContext[i];
    headContext[i] = headContext[i]->next;
    queueLength[i]--;

    if (waitQueueShrinkage && queueLength[PROCESS_REQUEST] <= scagQueueLimit) {
        MutexGuard q(queMon);
        
        waitQueueShrinkage = false;
        queMon.notify();
    }
    
    return cx;
}

void ScagTaskManager::looseQueueLimit()
{
    MutexGuard g(queMon);

    waitQueueShrinkage = false;
    queMon.notify();
}

void ScagTaskManager::queueLen(uint32_t& reqLen, uint32_t& respLen, uint32_t& lcmLen)
{
//    MutexGuard g(queMon);
    reqLen = queueLength[PROCESS_REQUEST];
    respLen = queueLength[PROCESS_RESPONSE];
    lcmLen = queueLength[PROCESS_LCM];
}

void ScagTaskManager::wakeTask()
{
    MutexGuard g(taskMon);
    
    taskMon.notifyAll();
}    

bool ScagTaskManager::canStop()
{
    MutexGuard g(procMut);
    
    return headContext[PROCESS_REQUEST] == NULL && headContext[PROCESS_RESPONSE] == NULL && headContext[PROCESS_STATUS_RESPONSE] == NULL;
}

IOTask* ReaderTaskManager::newTask()
{
    return new HttpReaderTask(manager, *this, manager.getConfig().connectionTimeout);
}

IOTask* WriterTaskManager::newTask()
{
    return new HttpWriterTask(manager, *this, manager.getConfig().connectionTimeout);
}

IOTaskManager::IOTaskManager(HttpManagerImpl& m) : manager(m),
    headTask(0), tailTask(UINT_MAX)
{
}

void IOTaskManager::giveContext(IOTask *t, HttpContext* cx) {
    smsc_log_debug(logger, "%p:%d choosen for context %p", t, t->getSocketCount(), cx);

    t->socketCount++;
    t->registerContext(cx);
    reorderTask(t);
}        

void IOTaskManager::process(HttpContext* cx)
{
    MutexGuard g(procMon);

    IOTask *t = getFirst();
    
    if (t->getSocketCount() < maxSockets) {
        giveContext(t, cx);
    }
    else {
        Call this_call(cx), *call;
                
        insertCall(&this_call);
    
        do {
            smsc_log_warn(logger, "%p waiting for free", cx);
            procMon.wait();
            smsc_log_warn(logger, "%p after waiting for free", cx);
            if (this_call.context) {
                call = headCall;
                for (;;) {
                    call = call->next;
                    t = getFirst();

                    if (t->getSocketCount() < maxSockets) {
                        giveContext(t, call->context);
                        call->context = NULL;
                        
                        if (call == &this_call) {
                            headCall = call->prev;
                            break;
                        }
                    }
                    else {
                        headCall = call->prev;
                        break;
                    }                    
                }
            }
            else {
                if (headCall == &this_call)
                    headCall = headCall->prev;
            }
        } while (this_call.context);
        /* exclude this_call from the queue and exit */
        this_call.prev->next = this_call.next;
        this_call.next->prev = this_call.prev;
    }
}

void IOTaskManager::insertCall(Call *call) {
    ActionID action = call->context->action;
    Call *cc = &tailCall;

//    smsc_log_error(logger, "||cc=%p, cc.prev=%p", cc, cc->prev);
    do 
        cc = cc->prev;
    while (cc->context && cc->context->action < action);
    
    call->next = cc->next;
    call->prev = cc;
    cc->next->prev = call;
    cc->next = call;
}

void IOTaskManager::removeContext(IOTask* t, unsigned int nsub) {
    MutexGuard g(procMon);
    
    t->socketCount -= nsub;
    reorderTask(t);
    procMon.notifyAll();
}

void IOTaskManager::shutdown()
{
    procMon.notify();
    pool.shutdown();
    delete sortedTasks;
}

bool IOTaskManager::canStop()
{
    MutexGuard g(procMon);
    
    return sortedTasks[maxThreads]->getSocketCount() == 0;
}

void IOTaskManager::assignTask(unsigned int i, IOTask *t)
{
    sortedTasks[i] = t;
    t->taskIndex = i;
}

void IOTaskManager::init(int maxThread, int maxSock, const char *logName)
{
    unsigned int i;

    logger = Logger::getInstance(logName);

    tailCall.next = &tailCall;
    tailCall.prev = &tailCall;
    headCall = &tailCall;
   
    maxSockets = maxSock;
    maxThreads = maxThread;
    pool.setMaxThreads(maxThreads);
    
    sortedTasks = new IOTask *[maxThreads + 2];

    sortedTasks[0] = (IOTask *)&headTask;               // min socketCount
    sortedTasks[maxThreads + 1] = (IOTask *)&tailTask;  // max socketCount

    for (i = 1; i <= maxThreads; i++) {
        IOTask *t = newTask();

        assignTask(i, t);
        pool.startTask(t);
    }
}

void IOTaskManager::reorderTask(IOTask* t)
{
    int i = t->taskIndex;
    unsigned int cc = t->getSocketCount();
      
    if (cc > sortedTasks[i+1]->getSocketCount()) {
        /* go right */
        do {        
            assignTask(i, sortedTasks[i+1]);
        } while (cc > sortedTasks[++i+1]->getSocketCount());
        
        assignTask(i, t);
    }
    else if (cc < sortedTasks[i-1]->getSocketCount()) {
        /* go left */
        do {
            assignTask(i, sortedTasks[i-1]);
        } while (cc < sortedTasks[--i-1]->getSocketCount());
        
        assignTask(i, t);
    }
}

IOTaskManager::~IOTaskManager()
{
}

}}}
