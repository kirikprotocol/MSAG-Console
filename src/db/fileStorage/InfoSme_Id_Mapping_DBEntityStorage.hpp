#ifndef __DBENTITYSTORAGE_INFOSME_ID_MAPPING_DBENTITYSTORAGE_HPP__
# define __DBENTITYSTORAGE_INFOSME_ID_MAPPING_DBENTITYSTORAGE_HPP__ 1

# include "EntityKey.hpp"

# include "DBEntityStorageIface.hpp"
# include "InfoSme_Id_Mapping_Entity.hpp"

# include "DiskDataStorage.hpp"

# include "StorageIndex.hpp"
# include "InfoSme_Id_Mapping_Entity_Adapter.hpp"
# include <core/synchronization/Mutex.hpp>

class InfoSme_Id_Mapping_DBEntityStorage
  : public DbEntity_SearchIface<InfoSme_Id_Mapping_Entity::Id_Key, InfoSme_Id_Mapping_Entity>,
    public DbEntity_NonUniqueSearchIface<InfoSme_Id_Mapping_Entity::SmscId_Key, InfoSme_Id_Mapping_Entity>,
    public DbEntity_StorageIface<InfoSme_Id_Mapping_Entity>,
    public DbEntity_FullTableScanIface<InfoSme_Id_Mapping_Entity>,
    public DbEntity_EraseIface<InfoSme_Id_Mapping_Entity::Id_Key> {
public:
  InfoSme_Id_Mapping_DBEntityStorage(DataStorage_FileDispatcher<InfoSme_Id_Mapping_Entity_Adapter>* storage);
  virtual bool findValue(const InfoSme_Id_Mapping_Entity::Id_Key& key, InfoSme_Id_Mapping_Entity* result);
  virtual bool findFirstValue(const InfoSme_Id_Mapping_Entity::SmscId_Key& key, InfoSme_Id_Mapping_Entity* resultValue);
  virtual bool findNextValue(const InfoSme_Id_Mapping_Entity::SmscId_Key& key, InfoSme_Id_Mapping_Entity* resultValue);

  virtual bool putValue(const InfoSme_Id_Mapping_Entity& value);

  virtual DbIterator<InfoSme_Id_Mapping_Entity>* getIterator();

  virtual int eraseValue(const InfoSme_Id_Mapping_Entity::Id_Key& key);
private:
  DataStorage_FileDispatcher<InfoSme_Id_Mapping_Entity_Adapter>* _storage;
  UniqueStorageIndex<InfoSme_Id_Mapping_Entity::Id_Key,
                     DataStorage_FileDispatcher<InfoSme_Id_Mapping_Entity>::rid_t> _uniq_index_by_id_key;
  NonUniqueStorageIndex<InfoSme_Id_Mapping_Entity::SmscId_Key,
                        DataStorage_FileDispatcher<InfoSme_Id_Mapping_Entity>::rid_t> _nonuniq_index_by_smscid_key;
  smsc::core::synchronization::Mutex _storageLock;

  class InfoSme_Id_Mapping_DbIterator : public DbIterator<InfoSme_Id_Mapping_Entity> {
  public:
    InfoSme_Id_Mapping_DbIterator(DataStorage_FileDispatcher<InfoSme_Id_Mapping_Entity_Adapter>* storage,
                                  smsc::core::synchronization::Mutex& storageLock);
    virtual bool nextValue(InfoSme_Id_Mapping_Entity* resultValue);
  private:
    DataStorage_FileDispatcher<InfoSme_Id_Mapping_Entity_Adapter>* _storage;
    smsc::core::synchronization::Mutex& _storageLock;
    DataStorage_FileDispatcher<InfoSme_Id_Mapping_Entity>::rid_t _ridForSequentialBypass;
    bool _beginIteration;
  };

  friend class InfoSme_Id_Mapping_DbIterator;
};

#endif
