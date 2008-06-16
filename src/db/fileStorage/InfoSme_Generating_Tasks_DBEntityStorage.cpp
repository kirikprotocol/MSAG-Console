#include "InfoSme_Generating_Tasks_DBEntityStorage.hpp"

#include <memory>
#include "Exceptions.hpp"

InfoSme_Generating_Tasks_DBEntityStorage::InfoSme_Generating_Tasks_DBEntityStorage(DataStorage_FileDispatcher<InfoSme_Generating_Tasks_Entity_Adapter>* storage)
  : _storage(storage)
{
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("dbgStrg");
  smsc_log_debug(logger, "InfoSme_Generating_Tasks_DBEntityStorage::InfoSme_Generating_Tasks_DBEntityStorage::: It is creating an index");

  InfoSme_Generating_Tasks_Entity_Adapter record;
  DataStorage_FileDispatcher<InfoSme_Generating_Tasks_Entity_Adapter>::rid_t rid, nextRid;

  DataStorage_FileDispatcher<InfoSme_Generating_Tasks_Entity_Adapter>::operation_status_t
    opResult =_storage->extractFirstRecord(&record, &rid, &nextRid);

  while ( opResult == DataStorage_FileDispatcher<InfoSme_Generating_Tasks_Entity_Adapter>::OPERATION_OK ) {
    if ( ! _uniq_index_by_taskid_key.insertIndexedValue(InfoSme_Generating_Tasks_Entity::TaskId_Key(record.getAdaptedObjRef()),
                                                    rid) )
      throw smsc::db::SQLException("InfoSme_Generating_Tasks_DBEntityStorage::InfoSme_Generating_Tasks_DBEntityStorage::: cann't load storage data");
    else
      opResult =_storage->extractNextRecord(&record, &rid, &nextRid);
  }

  if ( opResult != DataStorage_FileDispatcher<InfoSme_Generating_Tasks_Entity_Adapter>::NO_RECORD_FOUND &&
       opResult != DataStorage_FileDispatcher<InfoSme_Generating_Tasks_Entity_Adapter>::RECORD_DELETED )
    throw smsc::db::SQLException("InfoSme_Generating_Tasks_DBEntityStorage::InfoSme_Generating_Tasks_DBEntityStorage::: can't load storage data");

  smsc_log_debug(logger, "InfoSme_Generating_Tasks_DBEntityStorage::InfoSme_Generating_Tasks_DBEntityStorage::: Indexe was created");
}

bool
InfoSme_Generating_Tasks_DBEntityStorage::findValue(const InfoSme_Generating_Tasks_Entity::TaskId_Key& key, InfoSme_Generating_Tasks_Entity* result)
{
  smsc::core::synchronization::MutexGuard lockGuard(_storageLock);
  DataStorage_FileDispatcher<InfoSme_Generating_Tasks_Entity_Adapter>::rid_t rid;

  InfoSme_Generating_Tasks_Entity_Adapter record;
  if ( !_uniq_index_by_taskid_key.findIndexedValueByKey(key, &rid) ||
       _storage->extractRecord(&record, rid) !=
       DataStorage_FileDispatcher<InfoSme_Generating_Tasks_Entity_Adapter>::OPERATION_OK )
    return false;
  else {
    *result = record.getAdaptedObjRef();
    return true;
  }
}

bool
InfoSme_Generating_Tasks_DBEntityStorage::putValue(const InfoSme_Generating_Tasks_Entity& value)
{
  smsc::core::synchronization::MutexGuard lockGuard(_storageLock);
  InfoSme_Generating_Tasks_Entity::TaskId_Key primaryKey(value);

  if ( !_uniq_index_by_taskid_key.isExist(primaryKey) ) {
    DataStorage_FileDispatcher<InfoSme_Generating_Tasks_Entity_Adapter>::rid_t rid;
    _storage->addRecord(value, &rid);

    _uniq_index_by_taskid_key.insertIndexedValue(primaryKey, rid);

    return true;
  } else return false;
}

int
InfoSme_Generating_Tasks_DBEntityStorage::eraseValue(const InfoSme_Generating_Tasks_Entity::TaskId_Key& key)
{
  smsc::core::synchronization::MutexGuard lockGuard(_storageLock);
  DataStorage_FileDispatcher<InfoSme_Generating_Tasks_Entity_Adapter>::rid_t rid;

  if ( _uniq_index_by_taskid_key.findIndexedValueByKey(key, &rid) ) {
    _uniq_index_by_taskid_key.eraseIndexedValue(key);
    _storage->deleteRecord(rid);
    return 1;
  } else return 0;
}
