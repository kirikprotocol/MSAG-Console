#include <limits.h>
#include "Managers.h"
#include "HttpContext.h"
#include "IOTasks.h"
#include "ScagTask.h"

namespace scag { namespace transport { namespace http
{

HttpManager::HttpManager() : scags(*this),
    readers(*this), writers(*this), acceptor(*this), ConfigListener(HTTPMAN_CFG)
{
    logger = Logger::getInstance("scag.http.manager");
}

void HttpManager::configChanged()
{
    shutdown();
    init(HttpProcessor::Instance(), ConfigManager::Instance().getHttpManConfig());
}

void HttpManager::init(HttpProcessor& p, const HttpManagerConfig& conf)
{
    this->cfg = conf;

    readers.init(cfg.readerPoolSize, cfg.readerSockets, "scag.http.reader");
    writers.init(cfg.writerPoolSize, cfg.writerSockets, "scag.http.writer");
    scags.init(cfg.scagPoolSize, cfg.scagQueueLimit, p);
    acceptor.init(cfg.host.c_str(), cfg.port);
    smsc_log_info(logger, "Http manager inited host=%s:%d", cfg.host.c_str(), cfg.port);
}

void HttpManager::shutdown()
{
    acceptor.shutdown();

    while (!(readers.canStop() && writers.canStop() && scags.canStop()))
        sleep(1);
        
    scags.shutdown();
    readers.shutdown();
    writers.shutdown();
    smsc_log_info(logger, "HttpManager shutdown");
}

ScagTaskManager::ScagTaskManager(HttpManager& m) : manager(m)
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
    }
}

void ScagTaskManager::process(HttpContext* cx)
{
    MutexGuard g(procMut);

    cx->next = NULL;
    if(headContext[cx->action])
        tailContext[cx->action]->next = cx;
    else        
        headContext[cx->action] = cx;
    tailContext[cx->action] = cx;        
    
    if (++queueLength > scagQueueLimit)
        waitQueueShrinkage = true;

    {
        MutexGuard q(taskMon);
    
        taskMon.notify();   
    }
}

/*void ScagTaskManager::continueExecution(LongCallContext* context, bool dropped)
{
    HttpContext *cx = (HttpContext*)context->stateMachineContext;
    context->continueExec = true;

    if(!dropped)
        process(cx);
    else
        delete cx;
}*/

void ScagTaskManager::init(int maxThreads, int scagQueueLim, HttpProcessor& p)
{
    int i;
 
    queueLength = 0;
    scagQueueLimit = scagQueueLim;
    waitQueueShrinkage = false;
    
    headContext[PROCESS_REQUEST] = NULL;
    headContext[PROCESS_RESPONSE] = NULL;
    headContext[PROCESS_STATUS_RESPONSE] = NULL;

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

    if(headContext[PROCESS_STATUS_RESPONSE])
        i = PROCESS_STATUS_RESPONSE;        
    else if(headContext[PROCESS_RESPONSE])
        i = PROCESS_RESPONSE;
    else if(headContext[PROCESS_REQUEST])
        i = PROCESS_REQUEST;
    else
        return NULL;            
    
    cx = headContext[i];
    headContext[i] = headContext[i]->next;
    queueLength--;

    if (waitQueueShrinkage && queueLength <= scagQueueLimit) {
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

void ScagTaskManager::wakeTask()
{
    MutexGuard g(taskMon);
    
    taskMon.notifyAll();
}    

bool ScagTaskManager::canStop()
{
    MutexGuard g(procMut);
    
    return tailContext[PROCESS_REQUEST] == NULL && tailContext[PROCESS_RESPONSE] == NULL && tailContext[PROCESS_STATUS_RESPONSE] == NULL;
}

IOTask* ReaderTaskManager::newTask()
{
    return new HttpReaderTask(manager, *this, manager.cfg.connectionTimeout);
}

IOTask* WriterTaskManager::newTask()
{
    return new HttpWriterTask(manager, *this, manager.cfg.connectionTimeout);
}

IOTaskManager::IOTaskManager(HttpManager& m) : manager(m),
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
