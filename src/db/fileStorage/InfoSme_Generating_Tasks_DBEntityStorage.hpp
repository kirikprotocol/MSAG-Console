#ifndef __DBENTITYSTORAGE_INFOSME_GENERATING_TASKS_DBENTITYSTORAGE_HPP__
# define __DBENTITYSTORAGE_INFOSME_GENERATING_TASKS_DBENTITYSTORAGE_HPP__ 1

# include "EntityKey.hpp"

# include "DBEntityStorageIface.hpp"
# include "InfoSme_Generating_Tasks_Entity.hpp"
# include "DiskDataStorage.hpp"

# include "StorageIndex.hpp"
# include "InfoSme_Generating_Tasks_Entity_Adapter.hpp"
# include <core/synchronization/Mutex.hpp>

class InfoSme_Generating_Tasks_DBEntityStorage
  : public DbEntity_SearchIface<InfoSme_Generating_Tasks_Entity::TaskId_Key,
                                InfoSme_Generating_Tasks_Entity>,
    public DbEntity_StorageIface<InfoSme_Generating_Tasks_Entity>,
    public DbEntity_EraseIface<InfoSme_Generating_Tasks_Entity::TaskId_Key> {
public:
  InfoSme_Generating_Tasks_DBEntityStorage(DataStorage_FileDispatcher<InfoSme_Generating_Tasks_Entity_Adapter>* storage);
  virtual bool findValue(const InfoSme_Generating_Tasks_Entity::TaskId_Key& key, InfoSme_Generating_Tasks_Entity* result);
  virtual bool putValue(const InfoSme_Generating_Tasks_Entity& value);
  virtual int eraseValue(const InfoSme_Generating_Tasks_Entity::TaskId_Key& key);
private:
  DataStorage_FileDispatcher<InfoSme_Generating_Tasks_Entity_Adapter>* _storage;
  UniqueStorageIndex<InfoSme_Generating_Tasks_Entity::TaskId_Key,
                     DataStorage_FileDispatcher<InfoSme_Generating_Tasks_Entity>::rid_t> _uniq_index_by_taskid_key;
  smsc::core::synchronization::Mutex _storageLock;
};

#endif
