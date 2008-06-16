#ifndef __DBENTITYSTORAGE_INFOSME_T_DBENTITYSTORAGE_HPP__
# define __DBENTITYSTORAGE_INFOSME_T_DBENTITYSTORAGE_HPP__ 1

# include "EntityKey.hpp"

# include "DBEntityStorageIface.hpp"
# include "InfoSme_T_Entity.hpp"

# include "DiskDataStorage.hpp"

# include "StorageIndex.hpp"
# include "InfoSme_T_Entity_Adapter.hpp"
# include <core/synchronization/Mutex.hpp>

/*
** Класс предоставляет интерфейс для сохранения элементов данных типа InfoSme_T_Entity,
** выполнения операций поиска сохраненных данных по заданному ключу, последовательного
** доступа к хранимым элементам  и удаления хранимого элемента на основании
** заданного значения ключа.
*/
class InfoSme_T_DBEntityStorage
  : public DbEntity_SearchIface<InfoSme_T_Entity::Id_Key, InfoSme_T_Entity>,
    //    public DbEntity_NonUniqueSearchIface<InfoSme_T_Entity::StateANDSDate_key, InfoSme_T_Entity>,
    public DbEntity_StorageIface<InfoSme_T_Entity>,
    public DbEntity_FullTableScanIface<InfoSme_T_Entity>,
    public DbEntity_EraseIface<InfoSme_T_Entity::Id_Key>,
    public DbEntity_UpdateIface<InfoSme_T_Entity::Id_Key, InfoSme_T_Entity> {
public:
  InfoSme_T_DBEntityStorage(DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>* storage,
                            bool buildIndex=true);
  ~InfoSme_T_DBEntityStorage();
  virtual bool findValue(const InfoSme_T_Entity::Id_Key& key, InfoSme_T_Entity* resultValue);

  virtual bool putValue(const InfoSme_T_Entity& value);

  virtual int eraseValue(const InfoSme_T_Entity::Id_Key& key);
  virtual int updateValue(const InfoSme_T_Entity::Id_Key& key, const InfoSme_T_Entity& oldValue , const InfoSme_T_Entity& newValue);

  uint64_t getNextIdSequenceNumber();

  class InfoSme_T_DbIterator : public DbIterator<InfoSme_T_Entity> {
  public:
    InfoSme_T_DbIterator(InfoSme_T_DBEntityStorage* dbIteratorCreator);

    virtual bool nextValue(InfoSme_T_Entity* resultValue);
    void setIndexSearchCrit(const InfoSme_T_Entity::StateANDSDate_key& fromKey,
                            const InfoSme_T_Entity::StateANDSDate_key& toKey);
  private:
    bool sequentialBypass(InfoSme_T_Entity* resultValue);
    bool indexedBypass(InfoSme_T_Entity* resultValue);

    InfoSme_T_DBEntityStorage* _dbIteratorCreator;
    //smsc::core::synchronization::Mutex& _storageLock;
    DataStorage_FileDispatcher<InfoSme_T_Entity>::rid_t _ridForSequentialBypass;
    bool _beginIteration, _searchStateCriterionIsSet;
    InfoSme_T_Entity::StateANDSDate_key _fromKey, _toKey;

    NonUniqueStorageIndex<InfoSme_T_Entity::StateANDSDate_key,
                          DataStorage_FileDispatcher<InfoSme_T_Entity>::rid_t>::IndexPosition indexPos;
  };

  virtual InfoSme_T_DbIterator* getIterator();
private:
  DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>* _storage;
  UniqueStorageIndex<InfoSme_T_Entity::Id_Key,
                     DataStorage_FileDispatcher<InfoSme_T_Entity>::rid_t> _uniq_index_by_id_key;
  NonUniqueStorageIndex<InfoSme_T_Entity::StateANDSDate_key,
                        DataStorage_FileDispatcher<InfoSme_T_Entity>::rid_t> _nonuniq_index_by_state_and_sdate_key;
  smsc::core::synchronization::Mutex _storageLock;

  
  friend class InfoSme_T_DbIterator;
};

#endif
