#include <limits.h>
#include "HttpLogger.h"
#include "Managers.h"
#include "HttpContext.h"
#include "IOTasks.h"
#include "ScagTask.h"

namespace scag { namespace transport { namespace http
{

HttpManager::HttpManager() : scags(*this),
    readers(*this), writers(*this), acceptor(*this)
{
}

void HttpManager::init(HttpProcessor& p, const HttpManagerConfig& _cfg)
{
    memcpy(&this->cfg, &_cfg, sizeof(HttpManagerConfig));
    
    readers.init(cfg.readerPoolSize, cfg.readerSockets);
    writers.init(cfg.writerPoolSize, cfg.writerSockets);    
    scags.init(cfg.scagPoolSize, cfg.scagQueueLimit, p);    
    acceptor.init(cfg.host, cfg.port);
}

void HttpManager::shutdown()
{
    acceptor.shutdown();
    while (HttpContext::getCount())
        sleep(1);
    readers.shutdown();
    writers.shutdown();
    sleep(1);
    scags.shutdown();
    sleep(1);
}

ScagTaskManager::ScagTaskManager(HttpManager& m) : manager(m)
{
}

void ScagTaskManager::shutdown()
{
    queMon.notify();
    pool.shutdown();
    
    for (;;) {
        HttpContext *cx = getFirst();
        
        if (cx)
            delete cx;
        else
            break;
    }
}

void ScagTaskManager::process(HttpContext* cx)
{
    procMut.Lock();

    cx->next = tailContext[cx->action]->next;

    switch (cx->action) {
    case 1:
        if (tailContext[0] == tailContext[1])
            tailContext[0] = cx;
        break;
    case 2:                    
        if (tailContext[1] == tailContext[2])
            tailContext[1] = cx;
        if (tailContext[0] == tailContext[2])
            tailContext[0] = cx;
    break;
    }
    tailContext[cx->action]->next = cx;
    tailContext[cx->action] = cx;
    
    if (++queueLength > scagQueueLimit)
        waitQueueShrinkage = true;

    //taskMon.Lock();
    taskMon.notify();
    //taskMon.Unlock();
    procMut.Unlock();
}

void ScagTaskManager::init(int maxThreads, int _scagQueueLimit, HttpProcessor& p)
{
    int i;
 
/*
    http_log_debug( "ScagTaskManager::taskMon == %p", &taskMon );
    http_log_debug( "ScagTaskManager::procMut == %p", &procMut );
    http_log_debug( "ScagTaskManager::queMon == %p", &queMon );
*/
    
    queueLength = 0;
    this->scagQueueLimit = _scagQueueLimit;
    waitQueueShrinkage = false;
        
    headContext = NULL;
    tailContext[0] = (HttpContext *)&headContext;
    tailContext[1] = (HttpContext *)&headContext;
    tailContext[2] = (HttpContext *)&headContext;

    pool.setMaxThreads(maxThreads);

    for (i = 0; i < maxThreads; i++) {
        ScagTask *t = new ScagTask(manager, p);

        pool.startTask(t);
    }
}

HttpContext* ScagTaskManager::getFirst() {
    HttpContext *cx;
    
    procMut.Lock();

    if (headContext) {
        cx = headContext;

        if (headContext == tailContext[0])
            tailContext[0] = (HttpContext *)&headContext;
        if (headContext == tailContext[1])
            tailContext[1] = (HttpContext *)&headContext;
        if (headContext == tailContext[2])
            tailContext[2] = (HttpContext *)&headContext;
        
        headContext = headContext->next;
        queueLength--;

        if (waitQueueShrinkage && queueLength <= scagQueueLimit) {
            //queMon.Lock();
            waitQueueShrinkage = false;
            queMon.notify();
            //queMon.Unlock();
        }
    }
    else {
        cx = NULL;
    }
    
    procMut.Unlock();
    
    return cx;
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
    http_log_debug("%s %p:%d choosen for context %p",
        cx->getTaskName(), t, t->getSocketCount(), cx);
    t->socketCount++;
    t->registerContext(cx);
    reorderTask(t);
}        

void IOTaskManager::process(HttpContext* cx)
{
    procMon.Lock();

    IOTask *t = getFirst();
    
    if (t->getSocketCount() < maxSockets) {
        giveContext(t, cx);
    }
    else {
        Call this_call(cx), *call;
                
        insertCall(&this_call);
    
        do {
            http_log_warn("Waiting for free %s", cx->getTaskName());            
            procMon.wait();

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
        
    procMon.Unlock();
}

void IOTaskManager::insertCall(Call *call) {
    ActionID action = call->context->action;
    Call *cc = &tailCall;

    do 
        cc = cc->prev;
    while (cc->context && cc->context->action < action);
    
    call->next = cc->next;
    call->prev = cc;
    cc->next->prev = call;
    cc->next = call;
}

void IOTaskManager::removeContext(IOTask* t, unsigned int nsub) {
    procMon.Lock();
    t->socketCount -= nsub;
    reorderTask(t);
    procMon.notifyAll();
    procMon.Unlock();
}

void IOTaskManager::shutdown()
{
    pool.shutdown();
    delete sortedTasks;
}

void IOTaskManager::assignTask(unsigned int i, IOTask *t)
{
    sortedTasks[i] = t;
    t->taskIndex = i;
}

void IOTaskManager::init(int _maxThreads, int _maxSockets)
{
    int i;

    //http_log_debug( "IOTaskManager::procMon == %p", &procMon );

    tailCall.next = &tailCall;
    tailCall.prev = &tailCall;
    headCall = &tailCall;
    this->maxSockets = _maxSockets;
    pool.setMaxThreads(_maxThreads);
    
    sortedTasks = new IOTask *[_maxThreads + 2];

    sortedTasks[0] = (IOTask *)&headTask;               // min socketCount
    sortedTasks[_maxThreads + 1] = (IOTask *)&tailTask;  // max socketCount

    for (i = 1; i <= _maxThreads; i++) {
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

}}}
