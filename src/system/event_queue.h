/*
  $Id$
*/


#if !defined (__Cxx_Header__EventQueue_h__)
#define __Cxx_Header__EventQueue_h__

#include "core/synchronization/Event.hpp"
#include "core/synchronization/Mutex.hpp"
#include "smeman/smsccmd.h"
#include "system/state_checker.hpp"
//#include <stdexcept>
#include <inttypes.h>
//#include <stdint.h>
#include <string.h>

#define DISABLE_LIST_DUMP

namespace smsc {
namespace system {

using namespace smsc::smeman;
using namespace smsc::core::synchronization;

typedef uint64_t MsgIdType;
//typedef uint64_t StateType;
//typedef uint64_t CommandType;

const int MAX_COMMAND_PROCESSED = 2000;
const int LOCK_LIFE_LENGTH = 8;

const int PRIORITIES_COUNT=32;
const int MAX_PROCESSED_LOCKERS=1000;

using std::runtime_error;

struct Tuple
{
  CommandType command;
  MsgIdType msgId;
  StateType state;
};

class EventQueue
{
  uint64_t counter;

  // запись списка команд
  struct CmdRecord
  {
    CommandType command;
    unsigned long timeout;
    CmdRecord(CommandType cmd) : command(cmd) {timeout = time(0)+LOCK_LIFE_LENGTH;}
    CmdRecord* next;
  };

  // запись таблицы блокировок
  struct Locker
  {
    bool locked;
    Locker* next_hash;
    MsgIdType msgId;
    StateType state;
    CmdRecord* cmds;
    int priority;
    Locker() : locked(false), cmds(0) {}
    ~Locker()
    {
      while ( cmds )
      {
        CmdRecord* n = cmds->next;
        delete cmds;
        cmds = n;
      }
    }

    void push_back(CmdRecord* cmd)
    {
      CmdRecord** iter = &cmds;
      for ( ; *iter != 0; iter = &(*iter)->next );
      *iter = cmd;
      cmd->next = 0;
    }

    // выберает следующую допустимую команду
    // возвращает true если команда найдена - false в обраном случае
    // удаляет все команды для которых возможен таймаут и он истек
    bool getNextCommand(CommandType& c)
    {
      CmdRecord** cmd = &cmds;
      unsigned long t = time(0);
      //!__trace2__("try to select command for %p(%lld)",this,msgId);
      while(*cmd)
      {
        if ( StateChecker::commandHasTimeout((*cmd)->command) )
        {
          if ( t > ((*cmd)->timeout) )
          {
            // remove command
            //!__trace2__("delete command:%lld (%lu/%lu)",msgId,t,(*cmd)->timeout);
            CmdRecord* tmp = *cmd;
            *cmd = (*cmd)->next;
            delete tmp;
            continue;
          }
        }
        //!__trace2__("getnextcommand(%lld): %d,%d",msgId,state,(*cmd)->command->get_commandId());
        if ( StateChecker::commandIsValid(state,(*cmd)->command) )
        {
          c = (*cmd)->command;
          // remove command
          CmdRecord* tmp = *cmd;
          *cmd = (*cmd)->next;
          delete tmp;
          return true;
        }
        cmd = &(*cmd)->next;
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
      int idx = key%(TABLE_SIZE);
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
      int idx = key%(TABLE_SIZE);
      for ( Locker* l = table[idx]; l!=0; l = l->next_hash)
      {
        if ( l->msgId == key ) return l;
      }
      return 0;
    }
    void remove(MsgIdType key)
    {
      int idx = key%(TABLE_SIZE);
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
  } hash;

  Event event;
  Mutex mutex;

  typedef std::multimap<int,Locker*> LockerQueue;
  typedef std::pair<int,Locker*> LockerPair;
  LockerQueue unlocked;
  int counts[PRIORITIES_COUNT];
  int processed;

public:
#define __synchronized__ MutexGuard mguard(mutex);

  EventQueue() : counter(0)
  {
    for(int i=0;i<PRIORITIES_COUNT;i++)counts[i]=0;
    processed=0;
  }

  ~EventQueue() {}

  uint64_t getCounter()
  {
  __synchronized__
    return counter;
  }

  void getStats(int& hcnt,int& ucnt)
  {
    __synchronized__
    hcnt=hash.getCount();
    ucnt=unlocked.size();
  }

  int calcWeight(int prio)
  {
    if(prio>PRIORITIES_COUNT)prio=PRIORITIES_COUNT-1;
    if(prio<0)prio=0;
    counts[prio]++;
    processed++;
    if(processed>MAX_PROCESSED_LOCKERS)
    {
      processed=0;
      for(int i=0;i<PRIORITIES_COUNT;i++)counts[i]=0;
    }
    return counts[prio]*10000/PRIORITIES_COUNT;
  }

  // добавляет в запись команду (создает новую запись приее отсутствии)
  // если для записи допустима выборка команд , то нотифицирует исполнителей
  void enqueue(MsgIdType msgId, const CommandType& command)
  {
  __synchronized__
    __trace2__("enqueue:cmd=%d, msgId=%lld",command->get_commandId(),msgId);
    Locker* locker = hash.get(msgId);
    //!__trace2__("enq: first=%p, last=%p, lock=%p",
    //!  first_unlocked,last_unlocked,locker);
    if ( !locker )
    {
      locker = new Locker;
      hash.put(msgId,locker);
      locker->state = StateTypeValue::UNKNOWN_STATE;
      locker->priority=command->get_priority();
      int weight=calcWeight(locker->priority);
      __trace2__("enqueue: prio=%d, weight=%d",locker->priority,weight);
      unlocked.insert(LockerPair(weight,locker));
    }
    locker->msgId = msgId;
    locker->push_back(new CmdRecord(command));
    //!__trace2__("enqueue: last unlocked=%p",last_unlocked);
    event.Signal();
  }


  // просматривет список активных записей
  // записи в одном из финальных состояний при отсутствии команд удаляуются
  // для записей имеющих команды выберает доступную для текущего состояния команду
  // если нет записей с доступными командами ожидает нотификации
  void selectAndDequeue(Tuple& result,volatile bool* quitting)
  {
    //!__trace__("enter selectAndDequeue");
    for(;;)
    {
      {
      __synchronized__
        for (LockerQueue::iterator iter = unlocked.begin();iter!=unlocked.end();)
        {
          bool success = (*iter).second->getNextCommand(result.command);
          if ( success || !(*iter).second->cmds ||
               StateChecker::stateIsSuperFinal((*iter).second->state))
          {
            Locker* locker = (*iter).second;
            __trace2__("selectAndDequeue: weight=%d, msgId=%d",(*iter).first,locker->msgId);

            __require__(!locker->locked);

            if ( success ) // получена доступная команда
            {
              unlocked.erase(iter);

              locker->locked = true;
              result.msgId = locker->msgId;
              result.state = locker->state;
              return;
            }
            else if( !locker->cmds || StateChecker::stateIsSuperFinal(locker->state)) //вообще нет команд
            {
              if ( StateChecker::stateIsFinal(locker->state) )
              {
                LockerQueue::iterator tmp=iter;
                tmp++;
                unlocked.erase(iter);
                iter=tmp;
                hash.remove(locker->msgId);
                delete locker;
                continue;
              }
            }
          }
          iter++;
        }
      }
      event.Wait();
      if(*quitting)return;
    }
  }

  // изменяет состояние, снимает лок и добавляет в список активных
  // после чего нотифицирует исполнителей
  void changeState(MsgIdType msgId,StateType state)
  {
  __synchronized__
    Locker* locker = hash.get(msgId);
    if ( !locker ) throw runtime_error("incorrect msgid");
    if ( !locker->locked ) throw runtime_error("locker is not locked, can't change state");
    locker->state = state;

    // разблокируем запись и добавляем в список активных
    locker->locked = false;

    if ( StateChecker::stateIsFinal(state) )
      ++counter;

    unlocked.insert(LockerPair(calcWeight(locker->priority),locker));

    event.Signal();
  }
#undef __synchronized__
};

#undef DISABLE_LIST_DUMP


}; // namespace system
}; // namespace cmsc


#endif
