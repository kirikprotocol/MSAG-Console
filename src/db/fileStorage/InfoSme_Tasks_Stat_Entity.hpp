#ifndef __DBENTITYSTORAGE_INFOSME_TASKS_STAT_ENTITY_HPP__
# define __DBENTITYSTORAGE_INFOSME_TASKS_STAT_ENTITY_HPP__ 1

# include <sys/types.h>
# include <time.h>
# include <string>
# include <sstream>
# include "EntityKey.hpp"

class InfoSme_Tasks_Stat_Entity {
public:
  InfoSme_Tasks_Stat_Entity()
    : _taskId(""), _period(0), _generated(0), _delivered(0),
      _retried(0), _failed(0) {}

  InfoSme_Tasks_Stat_Entity(const std::string& taskId,
                            uint32_t period,
                            uint32_t generated,
                            uint32_t delivered,
                            uint32_t retried,
                            uint32_t failed ) :
    _taskId(taskId), _period(period), _generated(generated), _delivered(delivered), _retried(retried), _failed(failed) {}

  const std::string& getTaskId() const { return _taskId; }
  uint32_t getPeriod() const { return _period; }
  uint32_t getGenerated() const { return _generated; }
  uint32_t getDelivered() const { return _delivered; }
  uint32_t getRetried() const { return _retried; }
  uint32_t getFailed() const { return _failed; }
  std::string toString() const {
    std::ostringstream obuf;
    obuf << "taskId=[" << _taskId
         << "],period=[" << _period
         << "],generated=[" << _generated
         << "],delivered=[" << _delivered
         << "],retried=[" << _retried
         << "],failed=[" << _failed
         << "]";
    return obuf.str();
  }

  class TaskId_Period_Key;

  class TaskId_Key {
  public:
    TaskId_Key(const std::string& taskId) : _taskId(taskId) {}
    // It's constructor for making TaskId_Period_Key object from data of
    // InfoSme_Tasks_Stat_Entity object.
    TaskId_Key(const InfoSme_Tasks_Stat_Entity& dataValue) : _taskId(dataValue.getTaskId()) {}

    virtual bool operator== (const TaskId_Key& rhs) const {
      if ( _taskId == rhs._taskId ) return true;
      else return false;
    }

    virtual bool operator< (const TaskId_Key& rhs) const {
      if ( _taskId < rhs._taskId ) return true;
      else return false;
    }

    virtual std::string toString() const {
      std::ostringstream obuf;
      obuf << "taskId=[" << _taskId
           << "]";
      return obuf.str();
    }
  private:
    std::string _taskId;

    friend class TaskId_Period_Key;
  };

  // entity key definitions
  class TaskId_Period_Key
    : public EntityKey<key_traits<TaskId_Period_Key> > {
  public:
    TaskId_Period_Key(const std::string& taskId, uint32_t period) : _taskId(taskId), _period(period) {}
    // It's constructor for making TaskId_Period_Key object from data of
    // InfoSme_Tasks_Stat_Entity object.
    TaskId_Period_Key(const InfoSme_Tasks_Stat_Entity& dataValue) : _taskId(dataValue.getTaskId()), _period(dataValue.getPeriod()) {}
    /*
    ** этот конструктор используется при удалении объектов из хранилища на основании 
    ** ключа TaskId_Key, который является частью ключа TaskId_Period_Key
    */
    TaskId_Period_Key(const TaskId_Key& taskIdKey, uint32_t period) : _taskId(taskIdKey._taskId), _period(period) {}

    virtual bool operator== (const TaskId_Period_Key& rhs) const {
      if ( _taskId == rhs._taskId && 
           _period == rhs._period) return true;
      else return false;
    }

    virtual bool operator< (const TaskId_Period_Key& rhs) const {
      if ( _taskId < rhs._taskId )
        return true;
      else if ( _taskId == rhs._taskId && 
                _period < rhs._period ) return true;
      else return false;
      /*      if ( _taskId <= rhs._taskId && 
           _period < rhs._period) return true;
      else return false;*/
    }

    virtual std::string toString() const {
      std::ostringstream obuf;
      obuf << "taskId=[" << _taskId
           << "],period=[" <<_period
           << "]";
      return obuf.str();
    }
  private:
    std::string _taskId;
    uint32_t _period;
  };

private:
  std::string _taskId;
  uint32_t _period;
  uint32_t _generated;
  uint32_t _delivered;
  uint32_t _retried;
  uint32_t _failed;
};


#endif
