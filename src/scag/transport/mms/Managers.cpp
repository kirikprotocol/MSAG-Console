#include "Managers.h"
#include "MmsContext.h"
#include "ScagTask.h"

namespace scag { namespace transport { namespace mms {

IOTaskManager::IOTaskManager(MmsManagerImpl& m) : manager(m), head_task(0), tail_task(UINT_MAX){
}

void IOTaskManager::giveContext(IOTask *t, MmsContext* cx) {
  smsc_log_debug(logger, "%p:%d choosen for context %p", t, t->getSocketCount(), cx);
  cx->flags = 0;
  cx->result = 0;
  t->socket_count++;
  t->registerContext(cx);
  reorderTask(t);
}        

void IOTaskManager::insertCall(Call *call) {
  ActionID action = call->context->action;
  Call *cc = &tail_call;

  do {
    cc = cc->prev;
  } while (cc->context && cc->context->action < action);

  call->next = cc->next;
  call->prev = cc;
  cc->next->prev = call;
  cc->next = call;
}

void IOTaskManager::removeContext(IOTask* t, unsigned int nsub) {
  MutexGuard g(proc_mon);
  t->socket_count -= nsub;
  reorderTask(t);
  proc_mon.notifyAll();
}

void IOTaskManager::shutdown() {
  proc_mon.notify();
  pool.shutdown(); 
  delete sorted_tasks;
}

bool IOTaskManager::canStop() {
  MutexGuard g(proc_mon);
  return sorted_tasks[max_threads]->getSocketCount() == 0;
}

void IOTaskManager::assignTask(unsigned int i, IOTask *t) {
  sorted_tasks[i] = t;
  t->task_index = i;
}

void IOTaskManager::init(int _max_threads, int _max_sockets, const char *log_name) {
  logger = Logger::getInstance(log_name);
  tail_call.next = &tail_call;
  tail_call.prev = &tail_call;
  head_call = &tail_call;
  max_sockets = _max_sockets;
  max_threads = _max_threads;
  pool.setMaxThreads(max_threads);

  sorted_tasks = new IOTask *[max_threads + 2];
  sorted_tasks[0] = (IOTask *)&head_task;               // min socketCount
  sorted_tasks[max_threads + 1] = (IOTask *)&tail_task; // max socketCount

  for (unsigned int i = 1; i <= max_threads; i++) {
    IOTask *t = newTask();
    assignTask(i, t);
    pool.startTask(t);
  }
}

void IOTaskManager::reorderTask(IOTask* t) {
  int i = t->task_index;
  unsigned int cc = t->getSocketCount();

  if (cc > sorted_tasks[i+1]->getSocketCount()) {
    /* go right */
    do {        
      assignTask(i, sorted_tasks[i+1]);
    } while (cc > sorted_tasks[++i+1]->getSocketCount());
    assignTask(i, t);
  }
  else if (cc < sorted_tasks[i-1]->getSocketCount()) {
    /* go left */
    do {
      assignTask(i, sorted_tasks[i-1]);
    } while (cc < sorted_tasks[--i-1]->getSocketCount());
    assignTask(i, t);
  }
}

void IOTaskManager::process(MmsContext* cx) {
  MutexGuard g(proc_mon);

  IOTask* t = getFirst();

  if (t->getSocketCount() < max_sockets) {
    giveContext(t,cx);
    return;
  }

  Call this_call(cx), *call;
  insertCall(&this_call);
  do {
    smsc_log_warn(logger, "%p waiting for free", cx);
    proc_mon.wait();
    smsc_log_warn(logger, "%p after waiting for free", cx);
    if (this_call.context) {
      call = head_call;
      for (;;) {
        call = call->next;
        t = getFirst();
        if (t->getSocketCount() < max_sockets) {
          call->context->flags = 0;
          giveContext(t, call->context);
          call->context = NULL;
          if (call == &this_call) {
            head_call = call->prev;
            break;
          }
        }
        else {
          head_call = call->prev;
          break;
        }                    
      }
    } else {
      if (head_call == &this_call) {
        head_call = head_call->prev;
      }
    }
  } while (this_call.context);
  smsc_log_debug(logger, "exclude this_call.cx=%p from queue", this_call.context);
  this_call.prev->next = this_call.next;
  this_call.next->prev = this_call.prev;
}

IOTaskManager::~IOTaskManager(){
}

ScagTaskManager::ScagTaskManager(MmsManagerImpl &m):manager(m), transaction_id(0) {
}

unsigned int ScagTaskManager::getNewTransactionId() {
  MutexGuard g(tid_mutex);
  return ++transaction_id;
}

unsigned int ScagTaskManager::getTransactionId() {
  MutexGuard g(tid_mutex);
  return transaction_id;
}

void ScagTaskManager::init(int _max_threads, int _scag_queue_limit, MmsProcessor &p) {
  queue_length[PROCESS_LCM] = 0; 
  queue_length[PROCESS_REQUEST] = 0;
  queue_length[PROCESS_RESPONSE] = 0;
  queue_length[PROCESS_STATUS_RESPONSE] = 0;

  scag_queue_limit = scag_queue_limit;
  wait_queue_shrinkage = false;

  head_context[PROCESS_REQUEST] = NULL;
  head_context[PROCESS_RESPONSE] = NULL;
  head_context[PROCESS_STATUS_RESPONSE] = NULL;
  head_context[PROCESS_LCM] = NULL;    

  logger = Logger::getInstance("mms.scag");
  pool.setMaxThreads(_max_threads);
  for (int i = 0; i < _max_threads; ++i) {
    pool.startTask(new ScagTask(manager, p));
  }
}

void ScagTaskManager::looseQueueLimit() {
  MutexGuard g(que_mon);
  wait_queue_shrinkage = false;
  que_mon.notify();
}

void ScagTaskManager::wakeTask() {
  MutexGuard g(task_mon);
  task_mon.notify();
}

bool ScagTaskManager::canStop() {
  MutexGuard g(proc_mut);
  return head_context[PROCESS_REQUEST] == NULL && head_context[PROCESS_RESPONSE] == NULL &&
    head_context[PROCESS_STATUS_RESPONSE] == NULL;
}

void ScagTaskManager::deleteQueue(MmsContext *pcx) {
  MmsContext *cx;
  while (pcx) {
    cx = pcx;
    pcx = pcx->next;
    delete cx;
  }
}

void ScagTaskManager::shutdown() {
  pool.shutdown();
  MutexGuard g(proc_mut);
  deleteQueue(head_context[PROCESS_REQUEST]);
  deleteQueue(head_context[PROCESS_RESPONSE]);
  deleteQueue(head_context[PROCESS_STATUS_RESPONSE]);
  deleteQueue(head_context[PROCESS_LCM]);
}

void ScagTaskManager::fitQueueLimit() {
  MutexGuard g(que_mon);
  while (wait_queue_shrinkage) {
    smsc_log_warn(logger, "queue overlimited");
    que_mon.wait();
  }
}

void ScagTaskManager::waitForContext() {
  MutexGuard g(task_mon);
  task_mon.wait();
}

MmsContext* ScagTaskManager::getFirst() {
  uint32_t i;
  MmsContext *cx;
  MutexGuard g(proc_mut);

  if(head_context[PROCESS_LCM])
      i = PROCESS_LCM;
  else if(head_context[PROCESS_STATUS_RESPONSE])
      i = PROCESS_STATUS_RESPONSE;        
  else if(head_context[PROCESS_RESPONSE])
      i = PROCESS_RESPONSE;
  else if(head_context[PROCESS_REQUEST])
      i = PROCESS_REQUEST;
  else
      return NULL;            

  cx = head_context[i];
  head_context[i] = head_context[i]->next;
  queue_length[i]--;

  if (wait_queue_shrinkage && queue_length[PROCESS_REQUEST] <= scag_queue_limit) {
      MutexGuard q(que_mon);
      wait_queue_shrinkage = false;
      que_mon.notify();
  }
  return cx;
}

void ScagTaskManager::process(MmsContext *cx, bool continued) {
  MutexGuard g(proc_mut);

  cx->next = NULL;
  uint32_t i = continued ? PROCESS_LCM : cx->action;

  (head_context[i]) ? tail_context[i]->next = cx : head_context[i] = cx;

  tail_context[i] = cx;        
  ++queue_length[i];
  if (queue_length[PROCESS_REQUEST] > scag_queue_limit) {
    wait_queue_shrinkage = true;
  }

  MutexGuard q(task_mon);
  task_mon.notify();   
}

void ScagTaskManager::queueLength(uint32_t &req_len, uint32_t &resp_len, uint32_t &lcm_len) {
  req_len = queue_length[PROCESS_REQUEST];
  resp_len = queue_length[PROCESS_RESPONSE];
  lcm_len = queue_length[PROCESS_LCM];
}

void ScagTaskManager::continueExecution(LongCallContext *context, bool dropped) {
  MmsContext *cx = (MmsContext*)context->stateMachineContext;
  context->continueExec = true;

  (dropped) ? delete cx : process(cx, true);
}

IOTask* ReaderTaskManager::newTask() {
  return new MmsReaderTask(manager, *this, manager.getConfig().connection_timeout);
}

IOTask* WriterTaskManager::newTask() {
  return new MmsWriterTask(manager, *this, manager.getConfig().connection_timeout);
}


}//mms
}//transport
}//scag
