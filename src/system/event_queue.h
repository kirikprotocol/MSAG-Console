/*
  $Id$
*/


#if !defined (__Cxx_Header__EventQueue_h__)
#define __Cxx_Header__EventQueue_h__

#include "core/buffers/PriorityQueue.hpp"
#include "core/buffers/CyclicQueue.hpp"
#include "core/synchronization/Event.hpp"
#include "core/synchronization/Mutex.hpp"
#include "smeman/smsccmd.h"
#include "system/state_checker.hpp"
//#include <stdexcept>
#include <inttypes.h>
//#include <stdint.h>
#include <string.h>
#include <list>

#define DISABLE_LIST_DUMP

namespace smsc {
namespace system {

using namespace smsc::smeman;
using namespace smsc::core::synchronization;
using namespace smsc::core::buffers;

typedef uint64_t MsgIdType;
//typedef uint64_t StateType;
//typedef uint64_t CommandType;

const int MAX_COMMAND_PROCESSED = 2000;
const int LOCK_LIFE_LENGTH = 8;

using std::runtime_error;

struct Tuple
{
  CommandType command;
  MsgIdType msgId;
  StateType state;
};

class EventQueue
{
  public:
  uint64_t counter;

  // ������ ������� ����������
  struct Locker
  {
    bool locked;
    bool enqueued;
    Locker* next_hash;
    MsgIdType msgId;
    StateType state;
    typedef std::list<CommandType> CmdList;
    CmdList cmds;

    Locker() : locked(false),enqueued(false){}
    ~Locker()
    {
    }

    void push_back(const CommandType& c)
    {
      cmds.push_back(c);
    }

    // �������� ��������� ���������� �������
    // ���������� true ���� ������� ������� - false � ������� ������
    // ������� ��� ������� ��� ������� �������� ������� � �� �����
    bool getNextCommand(CommandType& c,bool remove=true)
    {
      for(CmdList::iterator i=cmds.begin();i!=cmds.end();i++)
      {
        if(StateChecker::commandIsValid(state,*i))
        {
          c = *i;
          if(remove)cmds.erase(i);
          return true;
        }
      }
      return false;
    }
  };

  class HashTable
  {
  public:
    static const int TABLE_SIZE = MAX_COMMAND_PROCESSED*LOCK_LIFE_LENGTH;
    Locker* table[TABLE_SIZE];
    int count;
    HashTable()
    {
      memset(table,0,sizeof(Locker*)*TABLE_SIZE);
      count=0;
    }
    ~HashTable()
    {
      int i;
      Locker *l,*tmp;
      for(i=0;i<TABLE_SIZE;i++)
      {
        l=table[i];
        while(l)
        {
          tmp=l->next_hash;
          __trace2__("deleting in hash destructor:msgid=%lld",l->msgId);
          delete l;
          l=tmp;
        }
      }
    }
    void put(MsgIdType key,Locker* value)
    {
      unsigned int idx = (unsigned int)(key%(TABLE_SIZE));
      if ( table[idx] )
      {
        value->next_hash = table[idx];
      }
      else value->next_hash = 0;
      table[idx] = value;
      count++;
      //!__trace2__("put locker %p(key=%lld) hash count:%d",value,key,count);
    }
    Locker* get(MsgIdType key)
    {
      unsigned int idx = (unsigned int)(key%(TABLE_SIZE));
      for ( Locker* l = table[idx]; l!=0; l = l->next_hash)
      {
        if ( l->msgId == key ) return l;
      }
      return 0;
    }
    void remove(MsgIdType key)
    {
      unsigned int idx = (unsigned int)(key%(TABLE_SIZE));
      for ( Locker** l = table+idx; *l!=0; l = &((*l)->next_hash))
      {
        if ( (*l)->msgId == key )
        {
          *l = (*l)->next_hash;
          count--;
          //!__trace2__("delete locker(%lld) hash count:%d",key,count);
          return;
        }
      }
      //!__trace2__("deleting non-existent locker:%lld",key);
    }
    int getCount(){return count;}
  };
  HashTable hash;

  Event event;
  Mutex mutex;


  typedef PriorityQueue<Locker*,CyclicQueue<Locker*>,0,31> LockerQueue;
  LockerQueue queue;

public:
#define __synchronized__ MutexGuard mguard(mutex);

  EventQueue() : counter(0)
  {
  }

  ~EventQueue() {}

  uint64_t getCounter()
  {
  __synchronized__
    return counter;
  }

  void getStats(int& hcnt,int& qcnt)
  {
    __synchronized__
    hcnt=hash.getCount();
    qcnt=queue.Count();
  }

  // ��������� � ������ ������� (������� ����� ������ ����� ����������)
  // ���� ��� ������ ��������� ������� ������ , �� ������������ ������������
  void enqueue(MsgIdType msgId, const CommandType& command)
  {
  __synchronized__
    __trace2__("enqueue:cmd=%d, msgId=%lld, prio=%d",command->get_commandId(),msgId,command->get_priority());
    Locker* locker = hash.get(msgId);

    if ( !locker )
    {
      locker = new Locker;
      locker->state = StateTypeValue::UNKNOWN_STATE;
      locker->msgId = msgId;

      hash.put(msgId,locker);
    }
    locker->push_back(command);
    if(!locker->locked && !locker->enqueued &&
       StateChecker::commandIsValid(locker->state,command))
    {
      locker->enqueued=true;
      queue.Push(locker,command->get_priority());
      event.Signal();
    }
  }

  typedef std::vector<std::pair<MsgIdType,CommandType> > EnqueueVector;
  void enqueueEx(EnqueueVector& in)
  {
    __synchronized__
    for(EnqueueVector::iterator it=in.begin();it!=in.end();it++)
    {
      MsgIdType msgId=it->first;
      CommandType& command=it->second;

      __trace2__("enqueue:cmd=%d, msgId=%lld, prio=%d",command->get_commandId(),msgId,command->get_priority());
      Locker* locker = hash.get(msgId);

      if ( !locker )
      {
        locker = new Locker;
        locker->state = StateTypeValue::UNKNOWN_STATE;
        locker->msgId = msgId;

        hash.put(msgId,locker);
      }
      locker->push_back(command);
      if(!locker->locked && !locker->enqueued &&
         StateChecker::commandIsValid(locker->state,command))
      {
        locker->enqueued=true;
        queue.Push(locker,command->get_priority());
        event.Signal();
      }
    }
  }


  // ������������ ������ �������� �������
  // ������ � ����� �� ��������� ��������� ��� ���������� ������ ����������
  // ��� ������� ������� ������� �������� ��������� ��� �������� ��������� �������
  // ���� ��� ������� � ���������� ��������� ������� �����������
  void selectAndDequeue(Tuple& result,volatile bool* quitting)
  {
    for(;;)
    {
      {
        __synchronized__
        if(queue.Count()>0)
        {
          Locker *l;
          queue.Pop(l);
          l->locked=true;
          l->enqueued=false;
          l->getNextCommand(result.command);
          result.msgId=l->msgId;
          result.state=l->state;
          return;
        }
      }
      event.Wait();
      if(*quitting)return;
    }
  }

  // �������� ���������, ������� ��� � ��������� � ������ ��������
  // ����� ���� ������������ ������������
  void changeState(MsgIdType msgId,StateType state)
  {
  __synchronized__
    Locker* locker = hash.get(msgId);
    if ( !locker ) throw runtime_error("incorrect msgid");
    if ( !locker->locked ) throw runtime_error("locker is not locked, can't change state");
    locker->state = state;
    //locker->lastCommand=UNKNOWN;

    // ������������ ������ � ��������� � ������ ��������
    locker->locked = false;


    if ( StateChecker::stateIsFinal(state) )
    {
      ++counter;
      if(locker->cmds.empty())
      {
        hash.remove(locker->msgId);
        delete locker;
        return;
      }
    }

    if(StateChecker::stateIsSuperFinal(state))
    {
      hash.remove(locker->msgId);
      delete locker;
      return;
    }

    CommandType cmd;
    if(locker->getNextCommand(cmd,false))
    {
      locker->enqueued=true;
      queue.Push(locker,cmd->get_priority());
    }

    event.Signal();
  }
#undef __synchronized__
};

#undef DISABLE_LIST_DUMP


} // namespace system
} // namespace cmsc


#endif
