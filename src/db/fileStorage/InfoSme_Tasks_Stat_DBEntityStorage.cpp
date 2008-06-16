#include "InfoSme_Tasks_Stat_DBEntityStorage.hpp"

#include <memory>
#include "Exceptions.hpp"
#include <logger/Logger.h>

InfoSme_Tasks_Stat_DBEntityStorage::InfoSme_Tasks_Stat_DBEntityStorage (DataStorage_FileDispatcher<InfoSme_Tasks_Stat_Entity_Adapter>* storage)
  : _storage(storage)
{
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("dbgStrg");
  smsc_log_debug(logger, "InfoSme_Tasks_Stat_DBEntityStorage::InfoSme_Tasks_Stat_DBEntityStorage::: It is creating an index");
  smsc::core::synchronization::MutexGuard lockGuard(_storageLock);
  InfoSme_Tasks_Stat_Entity_Adapter record;
  DataStorage_FileDispatcher<InfoSme_Tasks_Stat_Entity_Adapter>::rid_t rid, nextRid;

  DataStorage_FileDispatcher<InfoSme_Tasks_Stat_Entity_Adapter>::operation_status_t
    opResult =_storage->extractFirstRecord(&record, &rid, &nextRid);

  while ( opResult == DataStorage_FileDispatcher<InfoSme_Tasks_Stat_Entity_Adapter>::OPERATION_OK ) {
    if ( ! _nonuniq_index_by_taskid_and_period_key.insertIndexedValue(InfoSme_Tasks_Stat_Entity::TaskId_Period_Key(record.getAdaptedObjRef()),
                                                                      rid) )
      throw smsc::db::SQLException("InfoSme_Tasks_Stat_DBEntityStorage::InfoSme_Tasks_Stat_DBEntityStorage::: can't load storage data");
    else
      opResult =_storage->extractNextRecord(&record, &rid, &nextRid);
  }

  if ( opResult != DataStorage_FileDispatcher<InfoSme_Tasks_Stat_Entity_Adapter>::NO_RECORD_FOUND &&
       opResult != DataStorage_FileDispatcher<InfoSme_Tasks_Stat_Entity_Adapter>::RECORD_DELETED )
    throw smsc::db::SQLException("InfoSme_Tasks_Stat_DBEntityStorage::InfoSme_Tasks_Stat_DBEntityStorage::: can't load storage data");

  smsc_log_debug(logger, "InfoSme_Tasks_Stat_DBEntityStorage::InfoSme_Tasks_Stat_DBEntityStorage::: Index was created");
}

bool
InfoSme_Tasks_Stat_DBEntityStorage::findFirstValue(const InfoSme_Tasks_Stat_Entity::TaskId_Period_Key& key, InfoSme_Tasks_Stat_Entity* resultValue)
{
  smsc::core::synchronization::MutexGuard lockGuard(_storageLock);
  DataStorage_FileDispatcher<InfoSme_Tasks_Stat_Entity_Adapter>::rid_t rid;
  InfoSme_Tasks_Stat_Entity_Adapter record;
  if ( _nonuniq_index_by_taskid_and_period_key.findFirstIndexedValueByKey(key, &rid) &&
       _storage->extractRecord(&record, rid) ==
       DataStorage_FileDispatcher<InfoSme_Tasks_Stat_Entity_Adapter>::OPERATION_OK ) {
    *resultValue = record.getAdaptedObjRef();
    return true;
  } else
    return false;
}

bool
InfoSme_Tasks_Stat_DBEntityStorage::findNextValue(const InfoSme_Tasks_Stat_Entity::TaskId_Period_Key& key, InfoSme_Tasks_Stat_Entity* resultValue)
{
  smsc::core::synchronization::MutexGuard lockGuard(_storageLock);
  DataStorage_FileDispatcher<InfoSme_Tasks_Stat_Entity_Adapter>::rid_t rid;
  InfoSme_Tasks_Stat_Entity_Adapter record;
  if ( _nonuniq_index_by_taskid_and_period_key.findNextIndexedValueByKey(key, &rid) &&
       _storage->extractRecord(&record, rid) ==
       DataStorage_FileDispatcher<InfoSme_Tasks_Stat_Entity_Adapter>::OPERATION_OK ) {
    *resultValue = record.getAdaptedObjRef();
    return true;
  } else
    return false;

}

bool
InfoSme_Tasks_Stat_DBEntityStorage::putValue(const InfoSme_Tasks_Stat_Entity& value)
{
  smsc::core::synchronization::MutexGuard lockGuard(_storageLock);
  InfoSme_Tasks_Stat_Entity::TaskId_Period_Key nonuniqKey(value);

  DataStorage_FileDispatcher<InfoSme_Tasks_Stat_Entity>::rid_t rid;
  _storage->addRecord(value, &rid);

  _nonuniq_index_by_taskid_and_period_key.insertIndexedValue(nonuniqKey, rid);

  return true;
}

DbIterator<InfoSme_Tasks_Stat_Entity>*
InfoSme_Tasks_Stat_DBEntityStorage::getIterator()
{
  return new InfoSme_Tasks_Stat_DbIterator(_storage, _storageLock);
}

int
InfoSme_Tasks_Stat_DBEntityStorage::eraseValue(const InfoSme_Tasks_Stat_Entity::TaskId_Key& key)
{
  smsc::core::synchronization::MutexGuard lockGuard(_storageLock);
  DataStorage_FileDispatcher<InfoSme_Tasks_Stat_Entity>::rid_t rid;
  InfoSme_Tasks_Stat_Entity::TaskId_Period_Key minKeyValue(key,0);
  InfoSme_Tasks_Stat_Entity::TaskId_Period_Key maxKeyValue(key,0xFFFFFFFF);
  std::vector<DataStorage_FileDispatcher<InfoSme_Tasks_Stat_Entity>::rid_t> erasedValues;
  _nonuniq_index_by_taskid_and_period_key.eraseIndexedIntervalValue(minKeyValue, maxKeyValue, &erasedValues);

  for(std::vector<DataStorage_FileDispatcher<InfoSme_Tasks_Stat_Entity>::rid_t>::iterator iter=erasedValues.begin(); iter != erasedValues.end(); ++iter)
    _storage->deleteRecord(*iter);
  return erasedValues.size();
}

InfoSme_Tasks_Stat_DBEntityStorage::InfoSme_Tasks_Stat_DbIterator::InfoSme_Tasks_Stat_DbIterator(DataStorage_FileDispatcher<InfoSme_Tasks_Stat_Entity_Adapter>* storage, smsc::core::synchronization::Mutex& storageLock)
  : _storage(storage), _storageLock(storageLock), _ridForSequentialBypass(0), _beginIteration(true) {}

bool
InfoSme_Tasks_Stat_DBEntityStorage::InfoSme_Tasks_Stat_DbIterator::nextValue(InfoSme_Tasks_Stat_Entity* resultValue)
{
  smsc::core::synchronization::MutexGuard lockGuard(_storageLock);
  DataStorage_FileDispatcher<InfoSme_Tasks_Stat_Entity_Adapter>::operation_status_t status;

  DataStorage_FileDispatcher<InfoSme_Tasks_Stat_Entity_Adapter>::rid_t rid;
  InfoSme_Tasks_Stat_Entity_Adapter record;
  if ( _beginIteration ) {
    status = _storage->extractFirstRecord(&record, &rid, &_ridForSequentialBypass);
    _beginIteration = false;
  } else
    status = _storage->extractNextRecord(&record, &rid, &_ridForSequentialBypass);

  if ( status == DataStorage_FileDispatcher<InfoSme_Tasks_Stat_Entity_Adapter>::OPERATION_OK ) {
    *resultValue = record.getAdaptedObjRef();
    return true;
  } else
    return false;
}
