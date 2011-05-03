#ifndef SCAG_TRANSPORT_HTTP_IMPL_MANAGERS
#define SCAG_TRANSPORT_HTTP_IMPL_MANAGERS

#include <string>
#include "HttpAcceptor.h"
#include "TaskList.h"
#include "core/network/Socket.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/ThreadPool.hpp"
#include "logger/Logger.h"
#include "scag/config/base/ConfigListener2.h"
#include "scag/config/base/ConfigManager2.h"
#include "scag/config/http/HttpManagerConfig.h"
#include "scag/re/base/LongCallContextBase.h"
#include "util/timeslotcounter.hpp"
#include "scag/transport/http/base/HttpManager.h"

namespace scag2 {
namespace transport {
namespace http {

using smsc::core::synchronization::EventMonitor;
using smsc::core::synchronization::MutexGuard;
using smsc::core::network::Socket;
using smsc::core::threads::ThreadPool;
using smsc::logger::Logger;
using namespace lcm;

class HttpAcceptor;
class IOTask;
class ScagTask;
class HttpContext;
class HttpProcessor;
class HttpProcessorImpl;
class HttpManager;
class HttpManagerImpl;

class ScagTaskManager : public lcm::LongCallInitiator {
public:
    ScagTaskManager(HttpManagerImpl& m);

    void shutdown();
    void process(HttpContext* cx, bool continued);
    void init(int maxThreads, int scagQueueLimit, HttpProcessor& p);
    bool canStop();
    void wakeTask();
    HttpContext *getFirst();
    void looseQueueLimit();

    void fitQueueLimit() {
        MutexGuard g(queMon);
        while (waitQueueShrinkage) {
            smsc_log_warn(logger, "queue overlimited");

            queMon.wait();
        }
    }
    void waitForContext() {
        MutexGuard g(taskMon);
        
        taskMon.wait();
    }

    void continueExecution(lcm::LongCallContextBase* context, bool dropped);

    void queueLen(uint32_t& reqLen, uint32_t& respLen, uint32_t& lcmLen);

    unsigned pushSessionCommand( HttpCommand* cmd, int action );
    
protected:
    ThreadPool pool;
    smsc::core::synchronization::Mutex procMut;
    smsc::core::synchronization::EventMonitor queMon;
    smsc::core::synchronization::EventMonitor taskMon;
    HttpManagerImpl &manager;
    Logger *logger;    
    
    HttpContext *tailContext[4];
    HttpContext *headContext[4];    
    
    unsigned int queueLength[4];
    unsigned int scagQueueLimit;
    unsigned int queuedCmdCount;
    bool waitQueueShrinkage;
    void deleteQueue(HttpContext* pcx);
};

class IOTaskManager {
public:
    IOTaskManager(HttpManagerImpl& m);
    virtual ~IOTaskManager();

    void process(HttpContext* cx);
    void shutdown();
    void removeContext(IOTask* t, unsigned int nsub);
    void init(int maxThread, int maxSock, const char *logName);
    bool canStop();

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
    
public:
    ThreadPool pool;
    EventMonitor procMon;
   
    Call tailCall;
    Call *headCall;
    IOTaskParent headTask;
    IOTaskParent tailTask;
    IOTask **sortedTasks;
    HttpManagerImpl &manager;
    Logger *logger;
    unsigned int maxSockets;    
    unsigned int maxThreads;
};

class ReaderTaskManager : public IOTaskManager {
public:
    ReaderTaskManager(HttpManagerImpl& m) : IOTaskManager(m) {}

protected:
    virtual IOTask* newTask();
};

class WriterTaskManager : public IOTaskManager {
public:
    WriterTaskManager(HttpManagerImpl& m) : IOTaskManager(m) {}
protected:
    virtual IOTask* newTask();
};

/*struct HttpManagerConfig {
    int readerSockets;
    int writerSockets;
    int readerPoolSize;
    int writerPoolSize;
    int scagPoolSize;    
    int scagQueueLimit;
    int connectionTimeout;
    //unsigned int maxHeaderLength;
    std::string host;
    int port;

    HttpManagerConfig(int rs, int ws, int rps, int wps, int sps, int sql, int ct, const std::string h, int p):
        readerSockets(rs), writerSockets(ws),
        readerPoolSize(rps),writerPoolSize(wps),
        scagPoolSize(sps), scagQueueLimit(sql),
        connectionTimeout(ct), host(h), port(p)
    {
    }

    HttpManagerConfig():
        readerSockets(0), writerSockets(0),
        readerPoolSize(0),writerPoolSize(0),
        scagPoolSize(0), scagQueueLimit(0),
        connectionTimeout(0), host(""), port(0)
    {
    }

    HttpManagerConfig(const HttpManagerConfig& cp)
    {
        readerSockets = cp.readerSockets;
        writerSockets = cp.writerSockets;
        readerPoolSize = cp.readerPoolSize;
        writerPoolSize = cp.writerPoolSize;
        scagPoolSize = cp.scagPoolSize;
        scagQueueLimit = cp.scagQueueLimit;
        connectionTimeout = cp.connectionTimeout;
        host = cp.host;
        port = cp.port;
    }
};*/

/*
class HttpManager{

    static bool  inited;
    static Mutex initLock;

public:
    HttpManager() {};
    virtual ~HttpManager() {};

    static HttpManager& Instance();
    static void Init(HttpProcessor& p, const HttpManagerConfig& cfg);

    virtual void shutdown() = 0;
    virtual void process(HttpContext *cx) = 0;
    virtual void readerProcess(HttpContext *cx) = 0;
    virtual void writerProcess(HttpContext* cx) = 0;
    virtual HttpManagerConfig& getConfig() = 0;
    virtual ScagTaskManager* getScagTaskManager() = 0;
    virtual void getQueueLen(uint32_t& reqLen, uint32_t& respLen, uint32_t& lcmLen) = 0;
    virtual bool isLicenseExpired() = 0;
};
 */

class HttpManagerImpl: public HttpManager, public config::ConfigListener {
public:
    HttpManagerImpl();
    ~HttpManagerImpl() {};

    void init( HttpProcessorImpl& p, const config::HttpManagerConfig& cfg );
    void configChanged();

    void shutdown();
    void process(HttpContext* cx) { scags.process(cx, false); };
    void readerProcess(HttpContext* cx) { readers.process(cx); };
    void writerProcess(HttpContext* cx) { writers.process(cx); };
    config::HttpManagerConfig& getConfig() { return cfg; };
    // ScagTaskManager* getScagTaskManager() { return &scags; };

    void getQueueLen(uint32_t& reqLen, uint32_t& respLen, uint32_t& lcmLen) { scags.queueLen(reqLen, respLen, lcmLen); };

    bool isLicenseExpired();
    bool licenseThroughputLimitExceed();
    void incLicenseCounter();

    unsigned pushSessionCommand( HttpCommand* cmd, int action );
    
public:
    config::HttpManagerConfig cfg;
    ScagTaskManager scags;
    ReaderTaskManager readers;    
    WriterTaskManager writers;
    HttpProcessorImpl* processor_; // not owned

    smsc::util::TimeSlotCounter<> licenseCounter;
    time_t lastLicenseExpTest;
    int licenseFileCheckHour;
    bool licenseExpired;

protected:
    Logger *logger;
    HttpAcceptor acceptor;
};

}}}

#endif // SCAG_TRANSPORT_MANAGERS
