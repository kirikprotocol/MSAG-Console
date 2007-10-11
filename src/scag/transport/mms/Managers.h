#ifndef __SCAG_TRANSPORT_MMS_MANAGERS_H__
#define __SCAG_TRANSPORT_MMS_MANAGERS_H__

#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/ThreadPool.hpp"
#include "scag/config/mms/MmsManagerConfig.h"
#include "scag/config/ConfigListener.h"
#include "scag/lcm/LongCallManager.h"
#include "logger/Logger.h"
#include "MmsAcceptor.h"
#include "IOTasks.h"


namespace scag { namespace transport { namespace mms {

using smsc::core::synchronization::EventMonitor;
using smsc::core::synchronization::MutexGuard;
using smsc::core::threads::ThreadPool;
using smsc::logger::Logger;
using smsc::core::buffers::Hash;
using namespace scag::config;
using namespace scag::lcm;

class MmsAcceptor;
class IOTask;
class ScagTask;
class MmsContext;
class MmsProcessor;
class MmsManager;
class MmsManagerImpl;

class ScagTaskManager : public LongCallInitiator {
public:
  ScagTaskManager(MmsManagerImpl &m);

  void init(int _max_threads, int _scag_queue_limit, MmsProcessor &p);
  bool canStop();
  void wakeTask();
  MmsContext* getFirst();
  void shutdown();
  void process(MmsContext *cx, bool continued);
  void looseQueueLimit();
  void fitQueueLimit();
  void waitForContext();
  void continueExecution(LongCallContext *context, bool dropped);
  void queueLength(uint32_t &req_len, uint32_t &resp_len, uint32_t &lcm_len);

  unsigned int getNewTransactionId();
  unsigned int getTransactionId();

protected:

  void deleteQueue(MmsContext *cx);

  MmsManagerImpl &manager;
  ThreadPool pool;
  Mutex proc_mut;
  EventMonitor que_mon;
  EventMonitor task_mon;
  Logger *logger;
  MmsContext *head_context[4];
  MmsContext *tail_context[4];
  unsigned int queue_length[4];
  unsigned int scag_queue_limit;
  bool wait_queue_shrinkage;

  unsigned int transaction_id;
  Mutex tid_mutex;
};

class IOTaskManager {
public:
  IOTaskManager(MmsManagerImpl &m);
  virtual ~IOTaskManager();
  void process(MmsContext* cx);
  void shutdown();
  void removeContext(IOTask* t, unsigned int nsub);
  void init(int _max_threads, int _max_sockets, const char* log_name);
  bool canStop();
protected:
  struct Call {
    Call *next;
    Call *prev;
    MmsContext *context;
    Call(MmsContext *cx = NULL) : context(cx) {}
  };
  IOTask *getFirst() {
      return (IOTask *)sorted_tasks[1];
  }
  virtual IOTask* newTask() = 0;
  inline void assignTask(unsigned int i, IOTask *t);
  inline void giveContext(IOTask *t, MmsContext* cx);
  void reorderTask(IOTask* t);
  void insertCall(Call *call);

  ThreadPool pool;
  EventMonitor proc_mon;

  Call tail_call;
  Call *head_call;
  IOTaskParent head_task;
  IOTaskParent tail_task;
  IOTask **sorted_tasks;
  MmsManagerImpl &manager;
  Logger *logger;
  unsigned int max_sockets;    
  unsigned int max_threads;
};


class ReaderTaskManager : public IOTaskManager {
public:
  ReaderTaskManager(MmsManagerImpl &m) : IOTaskManager(m) {
  }
protected:
  virtual IOTask * newTask();
};

class WriterTaskManager : public IOTaskManager {
public:
  WriterTaskManager(MmsManagerImpl &m) : IOTaskManager(m) {
  }
protected:
  virtual IOTask * newTask();
};

class MmsManager {
public:
  MmsManager() {};
  virtual ~MmsManager() {};
  static void Init(MmsProcessor& p, const MmsManagerConfig& cfg, const char* mmscfgFile);
  static MmsManager& Instance();
  virtual void shutdown() = 0;
  virtual void process(MmsContext *cx) = 0;
  virtual void readerProcess(MmsContext *cx) = 0;
  virtual void writerProcess(MmsContext *cx) = 0;
  virtual void getQueueLength(uint32_t &req_len, uint32_t &resp_len, uint32_t &lcm_len) = 0;
private:
  static bool inited;
  static Mutex init_lock;
};

class MmsManagerImpl : public MmsManager, public ConfigListener {
public:
  MmsManagerImpl();  
  ~MmsManagerImpl() {};
  void init(MmsProcessor& p, const MmsManagerConfig& cfg, const char* mmscfgFile);
  void configChanged();
  void shutdown();

  void process(MmsContext *cx) {
    scags.process(cx, false);
  }
  void readerProcess(MmsContext *cx) {
    readers.process(cx);
  }
  void writerProcess(MmsContext *cx) {
    writers.process(cx);
  }
  MmsManagerConfig& getConfig() {
    return cfg;
  }
  ScagTaskManager* getScagTaskManager() {
    return &scags;
  }
  void getQueueLength(uint32_t &req_len, uint32_t &resp_len, uint32_t &lcm_len) {
    scags.queueLength(req_len, resp_len, lcm_len);
  }

  MmsManagerConfig cfg;
  ScagTaskManager scags;
  ReaderTaskManager readers;    
  WriterTaskManager writers;
protected:
  Logger *logger;
  MmsAcceptor rs_acceptor;
  MmsAcceptor vasp_acceptor;
};

}//mms
}//transport
}// scag

#endif
