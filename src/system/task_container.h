/*
  $Id$
*/

#if !defined __Cxx_Header__task_container_h__
#define __Cxx_Header__task_container_h__

#include <vector>
#include <utility>
#include "sms/sms.h"
#include "core/synchronization/Mutex.hpp"
#include "smeman/smsccmd.h"

namespace smsc{
namespace system{

using namespace smsc::core::synchronization;

#define TASK_CONTAINER_MAX_PROCESSED   65536
#define TASK_CONTAINER_MAX_TIMEOUTS    64

struct Task
{
  uint32_t proxy_id;
  uint32_t sequenceNumber;
  time_t creationTime;
  time_t timeout;
  smsc::sms::SMSId messageId;
  smsc::sms::SMS  *sms;
  bool diverted;
  int inDlgId;

  Task* next;
  Task* timeout_prev;
  Task* timeout_next;

  smsc::sms::Address sourceAddress;


  Task(uint32_t proxy = 0,uint32_t sequence = 0,smsc::sms::SMS  *s=0):
    proxy_id(proxy),sequenceNumber(sequence),sms(s),diverted(false),inDlgId(0) {}
  ~Task()
  {
  }
};

static inline int calcHashCode(uint32_t proxy,uint32_t sequence)
{
  unsigned int code = proxy^sequence;
  code = (code&0xffff) ^ (code>>16);
  return code%TASK_CONTAINER_MAX_PROCESSED;
}

struct FindTaskData{
  smsc::sms::SMSId id;
  uint32_t proxy_idx;
  uint32_t sequenceNumber;
  smsc::smeman::SmscCommand cmd;
  bool found;
  FindTaskData(uint32_t argProxyIdx,uint32_t argSeqNum,const smsc::smeman::SmscCommand& argCmd):
    id(0),proxy_idx(argProxyIdx),sequenceNumber(argSeqNum),cmd(argCmd),found(false)
  {
  }
};

typedef std::vector<FindTaskData> FindTaskVector;

class TaskContainer
{
  Task* hash[TASK_CONTAINER_MAX_PROCESSED]; // hash table
  Task pool[TASK_CONTAINER_MAX_PROCESSED];
  Task *first_task;
  struct TimeOutList{
    Task *timeout_link_begin;
    Task *timeout_link_end;
    time_t timeout;
  };
  TimeOutList toList[TASK_CONTAINER_MAX_TIMEOUTS];
  int toCount;

  int tasksCount;

  Mutex mutex;
  friend class StatusSme;
public:
  TaskContainer()
  {
    toCount=0;
    for ( int i=0; i<TASK_CONTAINER_MAX_PROCESSED; ++i )
    {
      pool[i].next = pool+i+1;
    }
    pool[TASK_CONTAINER_MAX_PROCESSED-1].next = 0;
    first_task = pool;
    memset(hash,0,sizeof(hash));
    tasksCount=0;
  }
  ~TaskContainer() {}

  bool getExpired(Task* t)
  {
    MutexGuard guard(mutex);
    time_t now = time(NULL);
    for(int i=0;i<toCount;i++)
    {
      if ( toList[i].timeout_link_begin &&
           toList[i].timeout_link_begin->timeout+
             toList[i].timeout_link_begin->creationTime < now )
      {
        *t = *toList[i].timeout_link_begin;
        __trace2__("TASK::getExpired 0x%x:0x%x",t->sequenceNumber,t->proxy_id);
        __findAndRemove(i,toList[i].timeout_link_begin);
        return true;
      }
    }
    return false;
  }

  bool createTask(const Task& t,unsigned long preferred_timeout=8)
  {
    MutexGuard guard(mutex);
    //checkTimeout();
    if ( !first_task ) return false;
    Task* task = first_task;
    first_task = first_task->next;
    *task = t;

    int idx;
    for(idx=0;idx<toCount;idx++)
    {
      if(toList[idx].timeout==(time_t)preferred_timeout)break;
    }
    if(idx==toCount)
    {
      idx=toCount;
      toCount++;
      __require__(toCount<TASK_CONTAINER_MAX_TIMEOUTS);
      toList[idx].timeout_link_begin=0;
      toList[idx].timeout_link_end=0;
      toList[idx].timeout=preferred_timeout;
    }

    if ( toList[idx].timeout_link_end )
    {
      toList[idx].timeout_link_end->timeout_next = task;
    }
    task->timeout_prev = toList[idx].timeout_link_end;
    task->timeout_next = 0;
    toList[idx].timeout_link_end = task;
    if ( !toList[idx].timeout_link_begin ) toList[idx].timeout_link_begin = task;
    int hashcode = calcHashCode(task->proxy_id,task->sequenceNumber);
    task->next = hash[hashcode];
    hash[hashcode] = task;
    task->timeout = preferred_timeout;
    task->creationTime=time(NULL);
    __trace2__("TASK::createTask 0x%x:0x%x timeout 0x%lx",
               task->sequenceNumber,task->proxy_id,
               preferred_timeout);
    tasksCount++;
    return true;
  }

  /*void checkTimeout(&Task)
  {
    unsigned long _time = time(NULL);
    //while ( timeout_link_begin && timeout_link_begin->timeout < _time )
    //{
      __findAndRemove(timeout_link_begin);
    //}
  }*/

  void findAndRemoveTaskEx(FindTaskVector& in)
  {
    MutexGuard guard(mutex);

    for(FindTaskVector::iterator it=in.begin();it!=in.end();it++)
    {
      uint32_t proxy_idx=it->proxy_idx;
      uint32_t sequenceNumber=it->sequenceNumber;

      int hashcode = calcHashCode(proxy_idx,sequenceNumber);
      Task* _res = hash[hashcode];
      Task* prev = 0;
      while ( _res )
      {
        if ( _res->proxy_id == proxy_idx && _res->sequenceNumber == sequenceNumber )
        {

          int idx;
          for(idx=0;idx<toCount;idx++)
          {
            __trace2__("toList.timeout=%d, _res->timeout=%d",toList[idx].timeout,_res->timeout);
            if(toList[idx].timeout==_res->timeout)break;
          }
          __require__(idx<toCount);

          remove(idx,_res,prev,hash+hashcode);
          //*res = *_res;
          //return true;
          //out.push(*_res);
          it->cmd->get_resp()->set_sms(_res->sms);
          it->cmd->get_resp()->set_diverted(_res->diverted);
          it->cmd->get_resp()->set_inDlgId(_res->inDlgId);
          it->cmd->set_priority(31);
          it->id=_res->messageId;
          it->found=true;
          break;
        }
        prev = _res;
        _res = prev->next;
      }
    }
  }


  bool findAndRemoveTask(uint32_t proxy_idx,uint32_t sequenceNumber,Task *res)
  {
    MutexGuard guard(mutex);
    __require__(res);
    //checkTimeout();
    int hashcode = calcHashCode(proxy_idx,sequenceNumber);
    Task* _res = hash[hashcode];
    Task* prev = 0;
    while ( _res )
    {
      if ( _res->proxy_id == proxy_idx && _res->sequenceNumber == sequenceNumber )
      {

        int idx;
        for(idx=0;idx<toCount;idx++)
        {
          __trace2__("toList.timeout=%d, _res->timeout=%d",toList[idx].timeout,_res->timeout);
          if(toList[idx].timeout==_res->timeout)break;
        }
        __require__(idx<toCount);

        remove(idx,_res,prev,hash+hashcode);
        *res = *_res;
        return true;
      }
      prev = _res;
      _res = prev->next;
    }
    return false;
  }

  void __findAndRemove(int idx,Task* task)
  {
    int hashcode = calcHashCode(task->proxy_id,task->sequenceNumber);
    Task* res = hash[hashcode];
    Task* prev = 0;
    while ( res )
    {
      if ( task == res )
      {
        remove(idx,task,prev,hash+hashcode);
        return;
      }
      prev = res;
      res = prev->next;
    }
    //throw Exception("");
  }
  void remove(int idx,Task* task,Task* prev,Task** addr)
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
      if ( task->timeout_next )
      {
        task->timeout_next->timeout_prev = task->timeout_prev;
      }
      else
      {
        __require__(toList[idx].timeout_link_end==task);
        toList[idx].timeout_link_end=toList[idx].timeout_link_end->timeout_prev;
      }
    }
    else
    {
      __require__( task == toList[idx].timeout_link_begin );
      toList[idx].timeout_link_begin = task->timeout_next;
      if (toList[idx].timeout_link_begin)
      {
        toList[idx].timeout_link_begin->timeout_prev = 0;
      }
      else
      {
        toList[idx].timeout_link_end = 0;
      }
    }

    // add into free_list

    task->next = first_task;
    first_task = task;
    tasksCount--;
  }
};

} // namespace system
} // namespace smsc

#endif
