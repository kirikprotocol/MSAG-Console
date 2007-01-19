#include "InfoSme_T_DBEntityStorage.hpp"

#include <memory>
#include "Exceptions.hpp"

InfoSme_T_DBEntityStorage::InfoSme_T_DBEntityStorage(DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>* storage) : _storage(storage), _currentIdSeqNum(0)
{
  InfoSme_T_Entity_Adapter record;
  typename DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::rid_t rid, nextRid;

  typename DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::operation_status_t
    opResult =_storage->extractFirstRecord(&record, &rid, &nextRid);

  while ( opResult == DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::OPERATION_OK ) {
    if ( ! _uniq_index_by_id_key.insertIndexedValue(InfoSme_T_Entity::Id_Key(record.getAdaptedObjRef()),
                                                    rid) ||
         ! _nonuniq_index_by_state_and_sdate_key.insertIndexedValue(InfoSme_T_Entity::StateANDSDate_key(record.getAdaptedObjRef()),
                                                                   rid) )
      throw smsc::db::SQLException("InfoSme_T_DBEntityStorage::InfoSme_T_DBEntityStorage::: can't load storage data (index update was failed)");
    else {
      _currentIdSeqNum = std::max(_currentIdSeqNum, record.getAdaptedObjRef().getId());
      opResult =_storage->extractNextRecord(&record, &rid, &nextRid);
    }
  }

  if ( opResult != DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::NO_RECORD_FOUND &&
       opResult != DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::RECORD_DELETED )
    throw smsc::db::SQLException("InfoSme_T_DBEntityStorage::InfoSme_T_DBEntityStorage::: can't load storage data", opResult, 0);
}

uint64_t
InfoSme_T_DBEntityStorage::getNextIdSequenceNumber()
{
  return ++_currentIdSeqNum;
}

bool
InfoSme_T_DBEntityStorage::putValue(const InfoSme_T_Entity& value)
{
  smsc::core::synchronization::MutexGuard lockGuard(_storageLock);
  InfoSme_T_Entity::Id_Key primaryKey(value);

  if ( !_uniq_index_by_id_key.isExist(primaryKey) ) {
    typename DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::rid_t rid;
    if ( _storage->addRecord(value, &rid) != 
         DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::OPERATION_OK )
      return false;

    _uniq_index_by_id_key.insertIndexedValue(primaryKey, rid);
    _nonuniq_index_by_state_and_sdate_key.insertIndexedValue(InfoSme_T_Entity::StateANDSDate_key(value), rid);
    return true;
  } else return false;
}

DbIterator<InfoSme_T_Entity>*
InfoSme_T_DBEntityStorage::getIterator()
{
  return new InfoSme_T_DbIterator(_storage, _storageLock);
}

bool
InfoSme_T_DBEntityStorage::findValue(const InfoSme_T_Entity::Id_Key& key,
                                       InfoSme_T_Entity* result)
{
  smsc::core::synchronization::MutexGuard lockGuard(_storageLock);
  typename DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::rid_t rid;

  InfoSme_T_Entity_Adapter record;
  if ( !_uniq_index_by_id_key.findIndexedValueByKey(key, &rid) ||
       _storage->extractRecord(&record, rid) !=
       DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::OPERATION_OK )
    return false;
  else {
    *result = record.getAdaptedObjRef();
    return true;
  }
}

bool
InfoSme_T_DBEntityStorage::findFirstValue(const InfoSme_T_Entity::StateANDSDate_key& key, InfoSme_T_Entity* resultValue)
{
  smsc::core::synchronization::MutexGuard lockGuard(_storageLock);
  typename DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::rid_t rid;
  InfoSme_T_Entity_Adapter record;
  if ( _nonuniq_index_by_state_and_sdate_key.findFirstIndexedValueByKey(key, &rid) &&
       _storage->extractRecord(&record, rid) ==
       DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::OPERATION_OK ) {
    *resultValue = record.getAdaptedObjRef();
    return true;
  } else
    return false;
}

bool
InfoSme_T_DBEntityStorage::findNextValue(const InfoSme_T_Entity::StateANDSDate_key& key, InfoSme_T_Entity* resultValue)
{
  smsc::core::synchronization::MutexGuard lockGuard(_storageLock);
  typename DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::rid_t rid;
  InfoSme_T_Entity_Adapter record;
  if ( _nonuniq_index_by_state_and_sdate_key.findNextIndexedValueByKey(key, &rid) &&
       _storage->extractRecord(&record, rid) ==
       DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::OPERATION_OK ) {
    *resultValue = record.getAdaptedObjRef();
    return true;
  } else
    return false;
}

int
InfoSme_T_DBEntityStorage::eraseValue(const InfoSme_T_Entity::Id_Key& key)
{
  smsc::core::synchronization::MutexGuard lockGuard(_storageLock);
  typename DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::rid_t rid;

  if ( _uniq_index_by_id_key.findIndexedValueByKey(key, &rid) ) {
    InfoSme_T_Entity_Adapter record;
    if ( _storage->extractRecord(&record, rid) ==
         DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::OPERATION_OK ) {
      _nonuniq_index_by_state_and_sdate_key.eraseIndexedValue(InfoSme_T_Entity::StateANDSDate_key(record.getAdaptedObjRef()), rid);
    }
    _uniq_index_by_id_key.eraseIndexedValue(key);
    _storage->deleteRecord(rid);
    return 1;
  } else return 0;
}

int
InfoSme_T_DBEntityStorage::updateValue(const InfoSme_T_Entity::Id_Key& key, const InfoSme_T_Entity& oldValue , const InfoSme_T_Entity& newValue)
{
  smsc::core::synchronization::MutexGuard lockGuard(_storageLock);
  typename DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::rid_t rid;

  InfoSme_T_Entity_Adapter* record;
  if ( _uniq_index_by_id_key.findIndexedValueByKey(key, &rid) ) {
    if ( _storage->replaceRecord(newValue, rid) ==
         DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::OPERATION_OK ) {
      InfoSme_T_Entity::StateANDSDate_key oldKey(oldValue);
      InfoSme_T_Entity::StateANDSDate_key newKey(newValue);
      if ( ! (oldKey == newKey) ) { 
        // удаляем из неуникального индекса запись со старым значением неуникального
        // ключа и rid'ом, который вернул вызов _uniq_index_by_id_key.findIndexedValueByKey
        _nonuniq_index_by_state_and_sdate_key.eraseIndexedValue(oldKey, rid);

        // и создаем новую прекрасную запись с значением rid, указывающим на 
        // запись в хранилище с обновленным значением
        _nonuniq_index_by_state_and_sdate_key.insertIndexedValue(newKey, rid);
      }
      return 1;
    } else return -1;
  } else return 0;
}

InfoSme_T_DBEntityStorage::InfoSme_T_DbIterator::InfoSme_T_DbIterator(DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>* storage, smsc::core::synchronization::Mutex& storageLock)
  : _storage(storage), _storageLock(storageLock), _ridForSequentialBypass(0), _beginIteration(true) {}

bool
InfoSme_T_DBEntityStorage::InfoSme_T_DbIterator::nextValue(InfoSme_T_Entity* resultValue)
{
  smsc::core::synchronization::MutexGuard lockGuard(_storageLock);
  DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::operation_status_t status;

  typename DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::rid_t rid;
  InfoSme_T_Entity_Adapter record;
  if ( _beginIteration ) {
    status = _storage->extractFirstRecord(&record, &rid, &_ridForSequentialBypass);
    _beginIteration = false;
  } else
    status = _storage->extractNextRecord(&record, &rid, &_ridForSequentialBypass);

  if ( status == DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::OPERATION_OK ) {
    *resultValue = record.getAdaptedObjRef();
    return true;
  } else
    return false;
}
