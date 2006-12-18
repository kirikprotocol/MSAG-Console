#ifndef __DBENTITYSTORAGE_INFOSME_GENERATING_TASKS_ENTITY_HPP__
# define __DBENTITYSTORAGE_INFOSME_GENERATING_TASKS_ENTITY_HPP__ 1

# include <sys/types.h>
# include <time.h>
# include <string>
# include <sstream>
# include "EntityKey.hpp"

class InfoSme_Generating_Tasks_Entity {
public:
  explicit InfoSme_Generating_Tasks_Entity(const std::string& taskId="") :
    _taskId(taskId) {}
  const std::string& getTaskId() const { return _taskId; }
  std::string toString() const {
    return "taskId=[" + _taskId + "]";
  }

  // entity key definitions
  class TaskId_Key
    : public EntityKey<key_traits<TaskId_Key> > {
  public:
    TaskId_Key(const std::string& idValue) : _idValue(idValue) {}
    // It's constructor for making Id_Key object from data of
    // InfoSme_Generating_Tasks_Entity object.
    TaskId_Key(const InfoSme_Generating_Tasks_Entity& dataValue) : _idValue (dataValue.getTaskId()) {}

    virtual bool operator== (const TaskId_Key& rhs) const {
      if ( _idValue == rhs._idValue ) return true;
      else return false;
    }

    virtual bool operator< (const TaskId_Key& rhs) const {
      if ( _idValue < rhs._idValue ) return true;
      else return false;
    }

    virtual std::string toString() const {
      return "idValue=[" + _idValue + "]";
    }
  private:
    std::string _idValue;
  };

private:
  std::string _taskId;
};

#endif
