#ifndef SCAG_TRANSPORT_MANAGERS
#define SCAG_TRANSPORT_MANAGERS

#include <string>
#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/ThreadPool.hpp"
#include "core/network/Socket.hpp"
#include "HttpAcceptor.h"
#include "TaskList.h"

namespace scag { namespace transport { namespace http
{
using smsc::core::synchronization::EventMonitor;
using smsc::core::network::Socket;
using smsc::core::threads::ThreadPool;

class HttpAcceptor;
class IOTask;
class ScagTask;
class HttpContext;
class HttpProcessor;
class HttpManager;

class ScagTaskManager {
public:
    ScagTaskManager(HttpManager& m);

    void shutdown();
    void process(HttpContext* cx);
    void init(int maxThreads, int scagQueueLimit, HttpProcessor& p);
    HttpContext* getFirst();
    inline void fitQueueLimit();
    inline void looseQueueLimit();

    EventMonitor taskMon;

protected:
    ThreadPool pool;
    Mutex procMut;
    EventMonitor queMon;
    HttpManager &manager;
    
    HttpContext *headContext;
    HttpContext *tailContext[3];
    
    unsigned int queueLength;
    unsigned int scagQueueLimit;
    bool waitQueueShrinkage;
};

class IOTaskManager {
public:
    IOTaskManager(HttpManager& m);

    void process(HttpContext* cx);
    void shutdown();
    void removeContext(IOTask* t, unsigned int nsub);
    void init(int maxThreads, int maxSockets);

protected:
    struct Call {
        Call *next;
        Call *prev;
        HttpContext *context;
        
        Call(HttpContext *cx = NULL) : context(cx) {}
    };

    IOTask *getFirst() {
        return (IOTask *)sortedTasks[1];
    }

    virtual IOTask* newTask() = 0;
    inline void assignTask(unsigned int i, IOTask *t);
    inline void giveContext(IOTask *t, HttpContext* cx);
    void reorderTask(IOTask* t);
    void insertCall(Call *call);
    
    ThreadPool pool;
    EventMonitor procMon;
   
    Call tailCall;
    Call *headCall;
    IOTaskParent headTask;
    IOTaskParent tailTask;
    IOTask **sortedTasks;
    HttpManager &manager;
    unsigned int maxSockets;
};

class ReaderTaskManager : public IOTaskManager {
public:
    ReaderTaskManager(HttpManager& m) : IOTaskManager(m) {}

protected:
    virtual IOTask* newTask();
};

class WriterTaskManager : public IOTaskManager {
public:
    WriterTaskManager(HttpManager& m) : IOTaskManager(m) {}
protected:
    virtual IOTask* newTask();
};

struct HttpManagerConfig {
    int readerSockets;
    int writerSockets;
    int readerPoolSize;
    int writerPoolSize;
    int scagPoolSize;    
    int scagQueueLimit;
    int connectionTimeout;
    //unsigned int maxHeaderLength;
    const char *host;
    int port;
};

class HttpManager {
public:
    HttpManager();

    void init(HttpProcessor& p, const HttpManagerConfig& cfg);
    void shutdown();

    HttpManagerConfig cfg;
    ScagTaskManager scags;
    ReaderTaskManager readers;    
    WriterTaskManager writers;

protected:
    HttpAcceptor acceptor;
};

}}}

#endif // SCAG_TRANSPORT_MANAGERS
