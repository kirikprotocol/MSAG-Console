/*
  $Id$
*/

#if !defined (__Cxx_Header__EventQueue_h__)
#define __Cxx_Header__EventQueue_h__

#include "core/synchronization/Event.hpp"
#include "core/synchronization/Mutex.hpp"
#include "smeman/smsccmd.h"
//#include <stdexcept>
#include <inttypes.h>
#include <stdint.h>


namespace smsc {
namespace system {

using namespace smsc::smeman;
using namespace smsc::core::synchronization;

typedef uint64_t MsgIdType;
typedef uint64_t StateType;
typedef SmscCommand CommandType;
//typedef uint64_t CommandType;

const int MAX_COMMAND_PROCESSED = 200;
const int LOCK_LIFE_LENGTH = 8;

using std::runtime_error;

namespace StateTypeValue
{
  const int UNKNOWN_STATE = 0;
};

class StateChecker
{
public:
  static bool commandIsValid(StateType state,CommandType cmd)
  {
    return true;
  }
  static bool commandHasTimeout(CommandType cmd)
  {
    return true;
  }
  static bool stateIsFinal(StateType state)
  {
    return false;
  }
  static bool stateIsSuperFinal(StateType state)
  {
    return false;
  }
};

struct Tuple
{
  CommandType command;
  MsgIdType msgId;
  StateType state;
};

class EventQueue
{
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
    ~Locker() { while ( cmds ) { CmdRecord* n = cmds->next; delete cmds; cmds = n;} }
    
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
    loop:
      if ( StateChecker::commandHasTimeout((*cmd)->command) )
      {
        if ( t > ((*cmd)->timeout) )
        {
          // remove command
          CmdRecord* tmp = *cmd;
          *cmd = (*cmd)->next;
          delete tmp;
          goto loop;
        }
      }
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
      if ( *cmd  ) goto loop;
      return false;
    }
  };

  class HashTable
  {
    static const int TABLE_SIZE = MAX_COMMAND_PROCESSED*LOCK_LIFE_LENGTH;
    Locker* table[TABLE_SIZE];
  public:
    void put(MsgIdType key,Locker* value)
    {
      int idx = key%(TABLE_SIZE);
      if ( table[idx] ) 
      {
        value->next_hash = table[idx];
      }
      else value->next_hash = 0;
      table[idx] = value;
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
      for ( Locker** l = table+idx; *l!=0; l = &(*l)->next_hash)
      {
        if ( (*l)->msgId == key )
        {
          *l = (*l)->next_hash;
        }
      }
    }
  } hash;
  
  Event event;
  Mutex mutex;
  
  Locker* first_unlocked;
  Locker* last_unlocked;

public:
#define __synchronized__ MutexGuard mguard(mutex);

  // добавляет в запись команду (создает новую запись приее отсутствии)
  // если для записи допустима выборка команд , то нотифицирует исполнителей
  void enqueue(MsgIdType msgId, const CommandType& command)
  {
  __synchronized__
    Locker* locker = hash.get(msgId);
    if ( !locker )
    {
      locker = new Locker;
      hash.put(msgId,locker);
      if ( last_unlocked )
        last_unlocked->next_unlocked = locker;
      else first_unlocked = last_unlocked = locker;
    }
    locker->state = StateTypeValue::UNKNOWN_STATE;
    locker->msgId = msgId;
    locker->push_back(new CmdRecord(command));
    if ( !locker->locked )
      event.Signal();
  }
  
  
  // просматривет список активных записей
  // записи в одном из финальных состояний при отсутствии команд удаляуются
  // для записей имеющих команды выберает доступную для текущего состояния команду
  // если нет записей с доступными командами ожидает нотификации
  void selectAndDequeue(Tuple& result)
  {
  wait_event:
    event.Wait();
    {
    __synchronized__
      Locker* locker = 0;
      Locker* prev = 0;
      unsigned long t = time(0);
      for (Locker* iter = first_unlocked; 
           iter != 0; prev = iter, iter = iter->next_unlocked )
      {
        bool success = iter->getNextCommand(result.command);
        if ( success || !iter->cmds )
        {
          if ( iter == last_unlocked ) last_unlocked = prev;
          if ( prev ) prev->next_unlocked = iter->next_unlocked;
          else first_unlocked = iter->next_unlocked;
          if ( success ) 
          {
            result.msgId = iter->msgId;
            result.state = iter->state;
            iter->locked = true;
            return;
          }
          else //( !iter->cmds ) 
          {
            if ( StateChecker::stateIsFinal(iter->state) )
            {
              hash.remove(iter->msgId);
              delete locker;
            }
          }
        }
        else // есть только ожидающие команды
        {
          // none
        }
      }
      goto wait_event;
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
    locker->locked = false;
    if ( last_unlocked )
      last_unlocked->next_unlocked = locker;
    else first_unlocked = last_unlocked = locker;
    event.Signal();
  }
#undef __synchronized__
};



}; // namespace system
}; // namespace cmsc


#endif

