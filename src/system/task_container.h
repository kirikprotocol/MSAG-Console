#if !defined __Cxx_Header__task_container_h__
#define __Cxx_Header__task_container_h__

namespace smsc{
namespace system{

#define TASK_CONTAINER_MAX_PROCESSED   (200*(8+1)) /* 200 msg/s * 8 sec */

struct Task
{
  uint32_t proxy_id;
  uint32_t sequenceNumber;
  unsigned long timeout;
  Task* next;
  Task* timeout_prev;
  Task* timeout_next;
  Task(uint32_t proxy = 0,uint32_t sequence = 0):
    proxy_id(proxy),sequenceNumber(sequence) {}
  ~Task()
  {
  }
};

static inline int calcHashCode(uint32_t proxy,uint32_t sequence)
{
  int code = proxy^sequence;
  code = (code&0xffff) ^ (code>>16);
  return code%TASK_CONTAINER_MAX_PROCESSED;
}

class TaskContainer
{
  Task* hash[TASK_CONTAINER_MAX_PROCESSED*2]; // hash table
  Task pool[TASK_CONTAINER_MAX_PROCESSED];
  Task *first_task;
  Task *timeout_link_begin;
  Task *timeout_link_end;
public:  
  TaskContainer():
    first_task(pool),
    timeout_link_begin(0),
    timeout_link_end(0)
  {
    for ( int i=0; i<TASK_CONTAINER_MAX_PROCESSED; ++i )
    {
      pool[i].next = pool+i;
    }
    pool[TASK_CONTAINER_MAX_PROCESSED-1].next = 0;
    memset(hash,0,sizeof(hash));
  }
  ~TaskContainer() {}
  
  bool createTask(const Task& t)
  {
    checkTimeout();
    if ( !first_task ) return false;
    Task* task = first_task;
    first_task = first_task->next;
    *task = t;
    if ( timeout_link_end )
    {
      timeout_link_end->timeout_next = task;
    }
    task->timeout_prev = timeout_link_end;
    task->timeout_next = 0;
    timeout_link_end = task;
    if ( !timeout_link_begin ) timeout_link_begin = task;
    int hashcode = calcHashCode(task->proxy_id,task->sequenceNumber);
    task->next = hash[hashcode];
    hash[hashcode] = task;
    task->timeout = time(NULL)+8;
		return true;
  }
  
  void checkTimeout()
  {
    unsigned long _time = time(NULL);
    while ( timeout_link_begin && timeout_link_begin->timeout < _time )
    {
      __findAndRemove(timeout_link_begin);
    }
  }
  
  bool findAndRemoveTask(uint32_t proxy_idx,uint32_t sequenceNumber,Task *res)
  {
    __require__(res);
    //checkTimeout();
    int hashcode = calcHashCode(proxy_idx,sequenceNumber);
    Task* _res = hash[hashcode];
    Task* prev = 0;
    while ( res )
    {
      if ( _res->proxy_id == proxy_idx && _res->sequenceNumber == sequenceNumber )
      {
        remove(_res,prev,hash+hashcode);
        *res = *_res;
        return true;
      }
      prev = _res;
      res = prev->next;
    }
    return false;
  }

  void __findAndRemove(Task* task)
  {
    int hashcode = calcHashCode(task->proxy_id,task->sequenceNumber);
    Task* res = hash[hashcode];
    Task* prev = 0;
    while ( res )
    {
      if ( task == res )
      {
        remove(task,prev,hash+hashcode);
        return;
      }
      prev = res;
      res = prev->next;
    }
    //throw Exception("");
  }
  void remove(Task* task,Task* prev,Task** addr)
  {
    // remove from hash
    if ( prev )
    {
      prev->next = task->next;
    }
    else 
    {
      __require__(addr);
      *addr = task->next;
    }
    
    // remove from time link
    
    if ( task->timeout_prev )
    {
      task->timeout_prev->timeout_next = task->timeout_next;
    }
    else
    {
      timeout_link_begin = task->timeout_next;
    }
    
    if ( task->timeout_next )
    {
      task->timeout_next->timeout_prev = task->timeout_prev;
    }
    else
    {
      timeout_link_end = task->timeout_prev;
    }

    // add into free_list

    task->next = first_task; 
    first_task = task;
  }
};

}; // namespace system
}; // namespace smsc

#endif
