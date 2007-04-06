#ifndef __DBENTITYSTORAGE_INFOSME_TASKS_STAT_DBENTITYSTORAGE_HPP__
# define __DBENTITYSTORAGE_INFOSME_TASKS_STAT_DBENTITYSTORAGE_HPP__ 1

# include "EntityKey.hpp"

# include "DBEntityStorageIface.hpp"
# include "InfoSme_Tasks_Stat_Entity.hpp"

# include "DiskDataStorage.hpp"

# include "StorageIndex.hpp"
# include "InfoSme_Tasks_Stat_Entity_Adapter.hpp"
# include <core/synchronization/Mutex.hpp>

class InfoSme_Tasks_Stat_DBEntityStorage
  : public DbEntity_NonUniqueSearchIface<InfoSme_Tasks_Stat_Entity::TaskId_Period_Key,
                                         InfoSme_Tasks_Stat_Entity>,
    public DbEntity_StorageIface<InfoSme_Tasks_Stat_Entity>,
    public DbEntity_EraseIface<InfoSme_Tasks_Stat_Entity::TaskId_Key>,
    public DbEntity_FullTableScanIface<InfoSme_Tasks_Stat_Entity> {
public:
  InfoSme_Tasks_Stat_DBEntityStorage(DataStorage_FileDispatcher<InfoSme_Tasks_Stat_Entity_Adapter>* storage);

  virtual bool findFirstValue(const InfoSme_Tasks_Stat_Entity::TaskId_Period_Key& key, InfoSme_Tasks_Stat_Entity* resultValue);
  virtual bool findNextValue(const InfoSme_Tasks_Stat_Entity::TaskId_Period_Key& key, InfoSme_Tasks_Stat_Entity* resultValue);
  virtual bool putValue(const InfoSme_Tasks_Stat_Entity& value);

  virtual DbIterator<InfoSme_Tasks_Stat_Entity>* getIterator();

  virtual int eraseValue(const InfoSme_Tasks_Stat_Entity::TaskId_Key& key);
private:
  DataStorage_FileDispatcher<InfoSme_Tasks_Stat_Entity_Adapter>* _storage;

  NonUniqueStorageIndex<InfoSme_Tasks_Stat_Entity::TaskId_Period_Key,
                        DataStorage_FileDispatcher<InfoSme_Tasks_Stat_Entity>::rid_t> _nonuniq_index_by_taskid_and_period_key;
  smsc::core::synchronization::Mutex _storageLock;

  class InfoSme_Tasks_Stat_DbIterator : public DbIterator<InfoSme_Tasks_Stat_Entity> {
  public:
    InfoSme_Tasks_Stat_DbIterator(DataStorage_FileDispatcher<InfoSme_Tasks_Stat_Entity_Adapter>* storage,
                                  smsc::core::synchronization::Mutex& storageLock);
    virtual bool nextValue(InfoSme_Tasks_Stat_Entity* resultValue);
  private:
    DataStorage_FileDispatcher<InfoSme_Tasks_Stat_Entity_Adapter>* _storage;
    smsc::core::synchronization::Mutex& _storageLock;
    DataStorage_FileDispatcher<InfoSme_Tasks_Stat_Entity>::rid_t _ridForSequentialBypass;
    bool _beginIteration;
  };

  friend class InfoSme_Tasks_Stat_DbIterator;

  // disable copy
  InfoSme_Tasks_Stat_DBEntityStorage(const InfoSme_Tasks_Stat_DBEntityStorage& rhs);
  InfoSme_Tasks_Stat_DBEntityStorage& operator=(const InfoSme_Tasks_Stat_DBEntityStorage& rhs);
};

#endif
