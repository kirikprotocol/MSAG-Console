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

const int MAX_COMMAND_PROCESSED = 200;
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
    Locker* next_unlocked;
    CmdRecord* cmds;
    Locker() : locked(false), next_unlocked(0), cmds(0) {}
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
      __trace2__("try to select command for %p(%lld)",this,msgId);
      while(*cmd)
      {
        if ( StateChecker::commandHasTimeout((*cmd)->command) )
        {
          if ( t > ((*cmd)->timeout) )
          {
            // remove command
            __trace2__("delete command:%lld (%lu/%lu)",msgId,t,(*cmd)->timeout);
            CmdRecord* tmp = *cmd;
            *cmd = (*cmd)->next;
            delete tmp;
            continue;
          }
        }
        __trace2__("getnextcommand(%lld): %d,%d",msgId,state,(*cmd)->command->get_commandId());
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
      __trace2__("put locker %p(key=%lld) hash count:%d",value,key,count);
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
          __trace2__("delete locker(%lld) hash count:%d",key,count);
          return;
        }
      }
      __trace2__("deleting non-existent locker:%lld",key);
    }
    int getCount(){return count;}
  } hash;

  Event event;
  Mutex mutex;

  Locker* first_unlocked;
  Locker* last_unlocked;

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

  // добавляет в запись команду (создает новую запись приее отсутствии)
  // если для записи допустима выборка команд , то нотифицирует исполнителей
  void enqueue(MsgIdType msgId, const CommandType& command)
  {
  __synchronized__
    __trace2__("enqueue:%lld",msgId);
    Locker* locker = hash.get(msgId);
    __trace2__("enq: first=%p, last=%p, lock=%p",
      first_unlocked,last_unlocked,locker);
    if ( !locker )
    {
      locker = new Locker;
      hash.put(msgId,locker);
      locker->state = StateTypeValue::UNKNOWN_STATE;
      if ( last_unlocked )
      {
        last_unlocked->next_unlocked = locker;
        last_unlocked=locker;
        locker->next_unlocked=0;
      }
      else
      {
        __require__(first_unlocked == 0);
        first_unlocked = last_unlocked = locker;
      }
    }
    locker->msgId = msgId;
    locker->push_back(new CmdRecord(command));
    __trace2__("enqueue: last unlocked=%p",last_unlocked);
    event.Signal();
  }


  // просматривет список активных записей
  // записи в одном из финальных состояний при отсутствии команд удаляуются
  // для записей имеющих команды выберает доступную для текущего состояния команду
  // если нет записей с доступными командами ожидает нотификации
  void selectAndDequeue(Tuple& result,volatile bool* quitting)
  {
    __trace__("enter selectAndDequeue");
    for(;;)
    {
      {
        trace("selanddeq: enter synchronized block");
      __synchronized__
        trace("selanddeq: got mutex");
        Locker* prev = 0;
#if !defined ( DISABLE_ANY_CHECKS ) || defined(DISABLE_LIST_DUMP)
        {
          Locker *iter1,*iter2;
          int i;
          for(i=0;i<HashTable::TABLE_SIZE;i++)
          {
            iter1=hash.table[i];
            while(iter1)
            {
              if(!iter1->locked)
              {
                int ok=0;
                iter2=first_unlocked;
                while(iter2)
                {
                  if(iter2==iter1)
                  {
                    ok=1;
                    break;
                  }
                  iter2=iter2->next_unlocked;
                }
                if(!ok)
                {
                  __watch__(iter1);
                  iter2=first_unlocked;
                  while(iter2)
                  {
                    __trace2__("ptr:%p, id:%lld",iter2,iter2->msgId);
                    iter2=iter2->next_unlocked;
                  }
                  __require__(ok);
                }
              }
              iter1=iter1->next_hash;
            }
          }
        }
#endif // LIST TEST        
        __watch__(first_unlocked);
        __watch__(last_unlocked);
        for (Locker* iter = first_unlocked;
             iter != 0; ) //iter = iter->next_unlocked
        {
          __trace2__("iterate unlocked lockers:%lld",iter->msgId);
          bool success = iter->getNextCommand(result.command);
          if ( success || !iter->cmds )
          {
            Locker* locker = iter;
            __watch__(locker);
            iter = iter->next_unlocked;// prev не изменяется
            __require__(!locker->locked);


            if ( success ) // получена доступная команда
            {
              // удаляем из списка активных
#if !defined (DISABLE_ANY_CHECKS) || defined(DISABLE_LIST_DUMP)              
              {
              __trace__("dump list before");
              Locker *iter2=first_unlocked;
              while(iter2)
              {
                __trace2__("ptr:%p, id:%lld",iter2,iter2->msgId);
                iter2=iter2->next_unlocked;
              }
              }
#endif // DUMP LIST
              __trace2__("success:%p",locker);
              if ( locker == last_unlocked ) last_unlocked = prev;
              if ( prev )
                 prev->next_unlocked = locker->next_unlocked;
              else
              {
                __require__( locker == first_unlocked );
                __watch__(locker->next_unlocked);
                first_unlocked = locker->next_unlocked;
              }

#if !defined (DISABLE_ANY_CHECKS) || defined(DISABLE_LIST_DUMP)              
              {
              __trace__("dump list after");
              Locker *iter2=first_unlocked;
              while(iter2)
              {
                __trace2__("ptr:%p, id:%lld",iter2,iter2->msgId);
                iter2=iter2->next_unlocked;
              }
              }
#endif // DUMP LIST

              locker->next_unlocked = 0;
              locker->locked = true;
              result.msgId = locker->msgId;
              result.state = locker->state;
              __watch__(first_unlocked);
              __watch__(last_unlocked);
              __trace__("returning from selectAndDequeue");
              return;
            }
            else //( !iter->cmds ) вообще нет команд
            {
              if ( StateChecker::stateIsFinal(locker->state) )
              {
                // удаляем из списка активных
                if ( locker == last_unlocked ) last_unlocked = prev;
                if ( prev )
                   prev->next_unlocked = locker->next_unlocked;
                else
                {
                  __require__( locker == first_unlocked );
                  __watch__(locker->next_unlocked);
                  first_unlocked = locker->next_unlocked;
                }

                locker->next_unlocked = 0;
                hash.remove(locker->msgId);
                delete locker;
              }
              else
              {
                //locker->locked=true;
                prev = locker;
              }
            }
          }
          else // есть только ожидающие команды
          {
            // выбераем следующую запись
            prev = iter;
            iter = iter->next_unlocked;
            // none
          }
        }
      }
      __watch__(first_unlocked);
      __watch__(last_unlocked);
      __trace__("selanddeq:wait");
      event.Wait();
      __trace__("selanddeq:wait finished");
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


#if !defined (DISABLE_ANY_CHECKS) || defined(DISABLE_LIST_DUMP)              
    {Locker *iter2=first_unlocked;
    __trace__("change state: list before");
    while(iter2)
    {
      __trace2__("ptr:%p, id:%lld",iter2,iter2->msgId);
      iter2=iter2->next_unlocked;
    }}
#endif // DUMP LIST

    if ( last_unlocked )
    {
      last_unlocked->next_unlocked = locker;
      last_unlocked = locker;
      locker->next_unlocked=0;
    }
    else
    {
      __require__(first_unlocked == 0);
      first_unlocked = last_unlocked = locker;
    }

#if !defined (DISABLE_ANY_CHECKS) || defined(DISABLE_LIST_DUMP)              
    {Locker *iter2=first_unlocked;
    __trace__("change state: list after");
    while(iter2)
    {
      __trace2__("ptr:%p, id:%lld",iter2,iter2->msgId);
      iter2=iter2->next_unlocked;
    }}
#endif // DUMP LIST    

    __watch__(first_unlocked);
    __watch__(last_unlocked);
    event.Signal();
  }
#undef __synchronized__
};

#undef DISABLE_LIST_DUMP


}; // namespace system
}; // namespace cmsc


#endif


