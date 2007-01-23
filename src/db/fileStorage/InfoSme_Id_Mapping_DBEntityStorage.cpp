#include "InfoSme_Id_Mapping_DBEntityStorage.hpp"
#include "Exceptions.hpp"

InfoSme_Id_Mapping_DBEntityStorage::InfoSme_Id_Mapping_DBEntityStorage(DataStorage_FileDispatcher<InfoSme_Id_Mapping_Entity_Adapter>* storage) : _storage(storage)
{
  InfoSme_Id_Mapping_Entity_Adapter record;
  typename DataStorage_FileDispatcher<InfoSme_Id_Mapping_Entity_Adapter>::rid_t rid, nextRid;

  typename DataStorage_FileDispatcher<InfoSme_Id_Mapping_Entity_Adapter>::operation_status_t
    opResult =_storage->extractFirstRecord(&record, &rid, &nextRid);

  while ( opResult == DataStorage_FileDispatcher<InfoSme_Id_Mapping_Entity_Adapter>::OPERATION_OK ) {
    if ( ! _uniq_index_by_id_key.insertIndexedValue(InfoSme_Id_Mapping_Entity::Id_Key(record.getAdaptedObjRef()),
                                                    rid) ||
         ! _nonuniq_index_by_smscid_key.insertIndexedValue(InfoSme_Id_Mapping_Entity::SmscId_Key(record.getAdaptedObjRef()),
                                                           rid) )
      throw smsc::db::SQLException("InfoSme_Id_Mapping_DBEntityStorage::InfoSme_Id_Mapping_DBEntityStorage::: can't load storage data");
    else {
      InfoSme_Id_Mapping_Entity::SmscId_Key smscIdKey(record.getAdaptedObjRef());

      smsc::core::buffers::RefPtr<smsc::core::synchronization::RecursiveMutex,
                                  smsc::core::synchronization::Mutex> mutex =
        InfoSme_Id_Mapping_Entity::_mutexRegistry_ForSmscIdExAccess.getObject(smscIdKey);
      if ( !mutex.Get() ) {
        InfoSme_Id_Mapping_Entity::_mutexRegistry_ForSmscIdExAccess.toRegisterObject
          (smsc::core::buffers::RefPtr<smsc::core::synchronization::RecursiveMutex,
                                       smsc::core::synchronization::Mutex>(new smsc::core::synchronization::RecursiveMutex()), smscIdKey);
      }

      opResult =_storage->extractNextRecord(&record, &rid, &nextRid);
    }
  }

  if ( opResult != DataStorage_FileDispatcher<InfoSme_Id_Mapping_Entity_Adapter>::NO_RECORD_FOUND &&
       opResult != DataStorage_FileDispatcher<InfoSme_Id_Mapping_Entity_Adapter>::RECORD_DELETED )
    throw smsc::db::SQLException("InfoSme_Id_Mapping_DBEntityStorage::InfoSme_Id_Mapping_DBEntityStorage::: can't load storage data");
}

bool
InfoSme_Id_Mapping_DBEntityStorage::findValue(const InfoSme_Id_Mapping_Entity::Id_Key& key, InfoSme_Id_Mapping_Entity* result)
{
  smsc::core::synchronization::MutexGuard lockGuard(_storageLock);
  typename DataStorage_FileDispatcher<InfoSme_Id_Mapping_Entity>::rid_t rid;

  InfoSme_Id_Mapping_Entity_Adapter record;
  if ( !_uniq_index_by_id_key.findIndexedValueByKey(key, &rid) ||
       _storage->extractRecord(&record, rid) !=
       DataStorage_FileDispatcher<InfoSme_Id_Mapping_Entity>::OPERATION_OK )
    return false;
  else {
    *result = record.getAdaptedObjRef();
    return true;
  }
}

bool
InfoSme_Id_Mapping_DBEntityStorage::findFirstValue(const InfoSme_Id_Mapping_Entity::SmscId_Key& key, InfoSme_Id_Mapping_Entity* resultValue)
{
  smsc::core::synchronization::MutexGuard lockGuard(_storageLock);
  typename DataStorage_FileDispatcher<InfoSme_Id_Mapping_Entity>::rid_t rid;
  InfoSme_Id_Mapping_Entity_Adapter record;

  InfoSme_Id_Mapping_Entity::SmscId_Key foundKey(key);
  if ( _nonuniq_index_by_smscid_key.findFirstIndexedValueByKey(key, &foundKey, &rid) &&
       foundKey == key &&
       _storage->extractRecord(&record, rid) ==
       DataStorage_FileDispatcher<InfoSme_Id_Mapping_Entity_Adapter>::OPERATION_OK ) {
    *resultValue = record.getAdaptedObjRef();
    return true;
  } else
    return false;
}

bool
InfoSme_Id_Mapping_DBEntityStorage::findNextValue(const InfoSme_Id_Mapping_Entity::SmscId_Key& key, InfoSme_Id_Mapping_Entity* resultValue)
{
  smsc::core::synchronization::MutexGuard lockGuard(_storageLock);
  typename DataStorage_FileDispatcher<InfoSme_Id_Mapping_Entity>::rid_t rid;
  InfoSme_Id_Mapping_Entity_Adapter record;

  InfoSme_Id_Mapping_Entity::SmscId_Key foundKey(key);
  if ( _nonuniq_index_by_smscid_key.findNextIndexedValueByKey(key, &rid) &&
       foundKey == key &&
       _storage->extractRecord(&record, rid) ==
       DataStorage_FileDispatcher<InfoSme_Id_Mapping_Entity_Adapter>::OPERATION_OK ) {
    *resultValue = record.getAdaptedObjRef();
    return true;
  } else
    return false;

}

bool
InfoSme_Id_Mapping_DBEntityStorage::putValue(const InfoSme_Id_Mapping_Entity& value)
{
  smsc::core::synchronization::MutexGuard lockGuard(_storageLock);
  InfoSme_Id_Mapping_Entity::Id_Key primaryKey(value);

  if ( !_uniq_index_by_id_key.isExist(primaryKey ) ) {
    typename DataStorage_FileDispatcher<InfoSme_Id_Mapping_Entity>::rid_t rid;
    _storage->addRecord(value, &rid);

    _uniq_index_by_id_key.insertIndexedValue(primaryKey, rid);
    _nonuniq_index_by_smscid_key.insertIndexedValue(InfoSme_Id_Mapping_Entity::SmscId_Key(value), rid);

    return true;
  } else return false;
}

DbIterator<InfoSme_Id_Mapping_Entity>*
InfoSme_Id_Mapping_DBEntityStorage::getIterator()
{
  return new InfoSme_Id_Mapping_DbIterator(_storage, _storageLock);
}

int
InfoSme_Id_Mapping_DBEntityStorage::eraseValue(const InfoSme_Id_Mapping_Entity::Id_Key& key)
{
  typename DataStorage_FileDispatcher<InfoSme_Id_Mapping_Entity>::rid_t rid;

  smsc::core::synchronization::MutexGuard lockGuard(_storageLock);
  if ( _uniq_index_by_id_key.findIndexedValueByKey(key, &rid) ) {
    InfoSme_Id_Mapping_Entity_Adapter record;
    if ( _storage->extractRecord(&record, rid) ==
         DataStorage_FileDispatcher<InfoSme_Id_Mapping_Entity_Adapter>::OPERATION_OK ) {
      _nonuniq_index_by_smscid_key.eraseIndexedValue(InfoSme_Id_Mapping_Entity::SmscId_Key(record.getAdaptedObjRef()), rid);
    }
    _uniq_index_by_id_key.eraseIndexedValue(key);
    _storage->deleteRecord(rid);
    return 1;
  } else return 0;
}

InfoSme_Id_Mapping_DBEntityStorage::InfoSme_Id_Mapping_DbIterator::InfoSme_Id_Mapping_DbIterator(DataStorage_FileDispatcher<InfoSme_Id_Mapping_Entity_Adapter>* storage, smsc::core::synchronization::Mutex& storageLock)
  : _storage(storage), _storageLock(storageLock), _ridForSequentialBypass(0), _beginIteration(true) {}

bool
InfoSme_Id_Mapping_DBEntityStorage::InfoSme_Id_Mapping_DbIterator::nextValue(InfoSme_Id_Mapping_Entity* resultValue)
{
  smsc::core::synchronization::MutexGuard lockGuard(_storageLock);
  DataStorage_FileDispatcher<InfoSme_Id_Mapping_Entity_Adapter>::operation_status_t status;

  typename DataStorage_FileDispatcher<InfoSme_Id_Mapping_Entity_Adapter>::rid_t rid;
  InfoSme_Id_Mapping_Entity_Adapter record;
  if ( _beginIteration ) {
    status = _storage->extractFirstRecord(&record, &rid, &_ridForSequentialBypass);
    _beginIteration = false;
  } else
    status = _storage->extractNextRecord(&record, &rid, &_ridForSequentialBypass);

  if ( status == DataStorage_FileDispatcher<InfoSme_Id_Mapping_Entity_Adapter>::OPERATION_OK ) {
    *resultValue = record.getAdaptedObjRef();
    return true;
  } else
    return false;
}
