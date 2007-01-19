#ifndef __DBENTITYSTORAGE_INFOSME_ID_MAPPING_ENTITY_HPP__
# define __DBENTITYSTORAGE_INFOSME_ID_MAPPING_ENTITY_HPP__ 1

# include <stdio.h>
# include <sys/types.h>
# include <time.h>
# include <string>
# include <sstream>
# include "EntityKey.hpp"
# include <util/ObjectRegistry.hpp>
# include <core/synchronization/Mutex.hpp>
# include <core/synchronization/RecursiveMutex.hpp>
# include <core/buffers/RefPtr.hpp>

class SelectInfoSme_Id_Mapping_SmscId_criterion;
class Insert_into_InfoSme_Id_Mapping;
class Delete_from_InfoSme_Id_Mapping_By_SmscId;
class InfoSme_Id_Mapping_DBEntityStorage;

class InfoSme_Id_Mapping_Entity {
public:
  InfoSme_Id_Mapping_Entity() : _id(0), _smsc_id(""), _task_id("") {}

  InfoSme_Id_Mapping_Entity(uint64_t id,
                            const std::string& smsc_id,
                            const std::string& task_id) :
    _id(id), _smsc_id(smsc_id), _task_id(task_id) {}
  uint64_t getId() const { return _id; }
  const std::string& getSmscId() const { return _smsc_id; }
  const std::string& getTaskId() const { return _task_id; }
  std::string toString() const {
    std::ostringstream obuf;
    obuf << "id=[" << _id
         << "],smsc_id=[" << _smsc_id
         << "],task_id=[" << _task_id
         << "]";
    return obuf.str();
  }

  // entity key definitions
  class Id_Key
    : public EntityKey<key_traits<Id_Key> > {
  public:
    Id_Key(uint64_t idValue) : _idValue(idValue) {}
    // It's constructor for making Id_Key object from data of
    // InfoSme_Id_Mapping_Entity object.
    Id_Key(const InfoSme_Id_Mapping_Entity& dataValue) : _idValue (dataValue.getId()) {}

    virtual bool operator== (const Id_Key& rhs) const {
      if ( _idValue == rhs._idValue ) return true;
      else return false;
    }

    virtual bool operator< (const Id_Key& rhs) const {
      if ( _idValue < rhs._idValue ) return true;
      else return false;
    }

    virtual std::string toString() const {
      char str[64];
      snprintf(str,sizeof(str),"idValue=[%lld]",_idValue);
      return std::string(str);
    }
  private:
    uint64_t _idValue;
  };

  // entity key definitions
  class SmscId_Key
    : public EntityKey<key_traits<SmscId_Key> > {
  public:
    SmscId_Key(const std::string& smscIdValue) : _smscIdValue(smscIdValue) {}
    // It's constructor for making Id_Key object from data of
    // InfoSme_Id_Mapping_Entity object.
    SmscId_Key(const InfoSme_Id_Mapping_Entity& dataValue) : _smscIdValue (dataValue.getSmscId()) {}

    virtual bool operator== (const SmscId_Key& rhs) const {
      if ( _smscIdValue == rhs._smscIdValue ) return true;
      else return false;
    }

    virtual bool operator< (const SmscId_Key& rhs) const {
      if ( _smscIdValue < rhs._smscIdValue ) return true;
      else return false;
    }

    virtual std::string toString() const {
      return "smscIdValue=[" + _smscIdValue + "]";
    }
  private:
    std::string _smscIdValue;
  };

private:
  uint64_t _id;
  std::string _smsc_id;
  std::string _task_id;

  friend class SelectInfoSme_Id_Mapping_SmscId_criterion;
  friend class Insert_into_InfoSme_Id_Mapping;
  friend class Delete_from_InfoSme_Id_Mapping_By_SmscId;
  friend class InfoSme_Id_Mapping_DBEntityStorage;

  //static smsc::util::ObjectRegistry<smsc::core::buffers::RefPtr<smsc::core::synchronization::RecursiveMutex,
  static smsc::util::RefObjectRegistry<smsc::core::synchronization::RecursiveMutex,
                                       SmscId_Key> _mutexRegistry_ForSmscIdExAccess;
  static smsc::core::synchronization::Mutex _mutexRegistryLock_ForSmscIdExAccess;
};
#endif
