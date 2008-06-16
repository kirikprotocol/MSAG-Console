#include "InfoSme_T_DBEntityStorage.hpp"

#include <memory>
#include "Exceptions.hpp"
#include "SequenceNumber.hpp"
#include <logger/Logger.h>

InfoSme_T_DBEntityStorage::InfoSme_T_DBEntityStorage(DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>* storage, bool buildIndex) : _storage(storage)
{
  if ( buildIndex) {
    smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("dbgStrg");
    smsc_log_debug(logger, "InfoSme_T_DBEntityStorage::InfoSme_T_DBEntityStorage::: It is creating indexes");

    InfoSme_T_Entity_Adapter record;
    DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::rid_t rid, nextRid;

    DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::operation_status_t
      opResult =_storage->extractFirstRecord(&record, &rid, &nextRid);

    while ( opResult == DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::OPERATION_OK ) {
      if ( ! _uniq_index_by_id_key.insertIndexedValue(InfoSme_T_Entity::Id_Key(record.getAdaptedObjRef()),
                                                      rid) ||
           ! _nonuniq_index_by_state_and_sdate_key.insertIndexedValue(InfoSme_T_Entity::StateANDSDate_key(record.getAdaptedObjRef()),
                                                                      rid) )
        throw smsc::db::SQLException("InfoSme_T_DBEntityStorage::InfoSme_T_DBEntityStorage::: can't load storage data (index update was failed)");
      else {
        opResult =_storage->extractNextRecord(&record, &rid, &nextRid);
      }
    }

    if ( opResult != DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::NO_RECORD_FOUND &&
         opResult != DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::RECORD_DELETED )
      throw smsc::db::SQLException("InfoSme_T_DBEntityStorage::InfoSme_T_DBEntityStorage::: can't load storage data", opResult, 0);

    smsc_log_debug(logger, "InfoSme_T_DBEntityStorage::InfoSme_T_DBEntityStorage::: Indexes was created");
  }
}

InfoSme_T_DBEntityStorage::~InfoSme_T_DBEntityStorage()
{
  delete _storage;
}

uint64_t
InfoSme_T_DBEntityStorage::getNextIdSequenceNumber()
{
  return SequenceNumber::getInstance().getNextSequenceNumber();
}

bool
InfoSme_T_DBEntityStorage::putValue(const InfoSme_T_Entity& value)
{
  smsc::core::synchronization::MutexGuard lockGuard(_storageLock);
  InfoSme_T_Entity::Id_Key primaryKey(value);

  if ( !_uniq_index_by_id_key.isExist(primaryKey) ) {
    DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::rid_t rid;
    if ( _storage->addRecord(value, &rid) != 
         DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::OPERATION_OK )
      return false;

    _uniq_index_by_id_key.insertIndexedValue(primaryKey, rid);
    _nonuniq_index_by_state_and_sdate_key.insertIndexedValue(InfoSme_T_Entity::StateANDSDate_key(value), rid);
    return true;
  } else return false;
}

InfoSme_T_DBEntityStorage::InfoSme_T_DbIterator*
InfoSme_T_DBEntityStorage::getIterator()
{
  return new InfoSme_T_DbIterator(this);
}

bool
InfoSme_T_DBEntityStorage::findValue(const InfoSme_T_Entity::Id_Key& key,
                                       InfoSme_T_Entity* result)
{
  smsc::core::synchronization::MutexGuard lockGuard(_storageLock);
  DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::rid_t rid;

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

int
InfoSme_T_DBEntityStorage::eraseValue(const InfoSme_T_Entity::Id_Key& key)
{
  smsc::core::synchronization::MutexGuard lockGuard(_storageLock);
  DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::rid_t rid;

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
  DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::rid_t rid;

  InfoSme_T_Entity_Adapter* record;
  if ( _uniq_index_by_id_key.findIndexedValueByKey(key, &rid) ) {
    smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("dbstrg");
    //    smsc_log_debug(logger, "InfoSme_T_DBEntityStorage::updateValue::: call _storage->replaceRecord(newValue=[%s],rid=[%d]",newValue.toString().c_str(),rid);
    if ( _storage->replaceRecord(newValue, rid) ==
         DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::OPERATION_OK ) {
      //      smsc_log_debug(logger, "InfoSme_T_DBEntityStorage::updateValue::: _storage->replaceRecord returned ok");
      InfoSme_T_Entity::StateANDSDate_key oldKey(oldValue);
      InfoSme_T_Entity::StateANDSDate_key newKey(newValue);
      if ( ! (oldKey == newKey) ) {
        //        smsc_log_debug(logger, "InfoSme_T_DBEntityStorage::updateValue::: erase value from nonuniq idx");
        // удаляем из неуникального индекса запись со старым значением неуникального
        // ключа и rid'ом, который вернул вызов _uniq_index_by_id_key.findIndexedValueByKey
        _nonuniq_index_by_state_and_sdate_key.eraseIndexedValue(oldKey, rid);

        //        smsc_log_debug(logger, "InfoSme_T_DBEntityStorage::updateValue::: insert key into nonuniq idx");
        // и создаем новую прекрасную запись с значением rid, указывающим на 
        // запись в хранилище с обновленным значением
        _nonuniq_index_by_state_and_sdate_key.insertIndexedValue(newKey, rid);
      }
      //      smsc_log_debug(logger, "InfoSme_T_DBEntityStorage::updateValue::: return");
      return 1;
    } else return -1;
  } else return 0;
}

InfoSme_T_DBEntityStorage::InfoSme_T_DbIterator::InfoSme_T_DbIterator(InfoSme_T_DBEntityStorage* dbIteratorCreator)
  : _dbIteratorCreator(dbIteratorCreator), _ridForSequentialBypass(0), _beginIteration(true), _searchStateCriterionIsSet(false), _fromKey(0,0), _toKey(0,0) {}

void
InfoSme_T_DBEntityStorage::InfoSme_T_DbIterator::setIndexSearchCrit(const InfoSme_T_Entity::StateANDSDate_key& fromKey,const InfoSme_T_Entity::StateANDSDate_key& toKey)
{
  _searchStateCriterionIsSet = true;
  _fromKey = fromKey;
  _toKey = toKey;
}

bool
InfoSme_T_DBEntityStorage::InfoSme_T_DbIterator::nextValue(InfoSme_T_Entity* resultValue)
{
  if ( _searchStateCriterionIsSet )
    return indexedBypass(resultValue);
  else
    return sequentialBypass(resultValue);
}

bool
InfoSme_T_DBEntityStorage::InfoSme_T_DbIterator::sequentialBypass(InfoSme_T_Entity* resultValue)
{
  smsc::core::synchronization::MutexGuard lockGuard(_dbIteratorCreator->_storageLock);
  DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::operation_status_t status;

  DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::rid_t rid;
  InfoSme_T_Entity_Adapter record;
  if ( _beginIteration ) {
    status = _dbIteratorCreator->_storage->extractFirstRecord(&record, &rid, &_ridForSequentialBypass);
    _beginIteration = false;
  } else
    status = _dbIteratorCreator->_storage->extractNextRecord(&record, &rid, &_ridForSequentialBypass);

  if ( status == DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::OPERATION_OK ) {
    *resultValue = record.getAdaptedObjRef();
    return true;
  } else
    return false;
}

bool
InfoSme_T_DBEntityStorage::InfoSme_T_DbIterator::indexedBypass(InfoSme_T_Entity* resultValue)
{
  bool status;

  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("smsc.infosme.InfoSme");
  smsc_log_debug(logger, "InfoSme_T_DbIterator::indexedBypass::: _fromKey=[%s],_toKey=[%s]",_fromKey.toString().c_str(), _toKey.toString().c_str());

  smsc::core::synchronization::MutexGuard lockGuard(_dbIteratorCreator->_storageLock);
  DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::rid_t rid;
  InfoSme_T_Entity_Adapter record;
  InfoSme_T_Entity::StateANDSDate_key foundKey(0,0);
  InfoSme_T_Entity::StateANDSDate_key searchKey(0,0);

  bool searchRes;
  if ( _beginIteration ) {
    _beginIteration = false;

    searchRes = _dbIteratorCreator->_nonuniq_index_by_state_and_sdate_key.findFirstIndexedValueByKey(_fromKey, &foundKey, &rid, indexPos);
    smsc_log_debug(logger, "InfoSme_T_DbIterator::indexedBypass::: begin: searchRes=%d", searchRes);

    searchKey = _fromKey;
  } else {
    searchRes = _dbIteratorCreator->_nonuniq_index_by_state_and_sdate_key.findNextIndexedValueByKey(_toKey, &foundKey, &rid, indexPos);
    smsc_log_debug(logger, "InfoSme_T_DbIterator::indexedBypass::: next: searchRes=%d", searchRes);

    searchKey = _toKey;
  }

  if ( searchRes)
    smsc_log_debug(logger, "InfoSme_T_DbIterator::indexedBypass::: foundKey=[%s]", foundKey.toString().c_str());

  if ( searchRes &&
       foundKey.getState() == searchKey.getState() &&
       _dbIteratorCreator->_storage->extractRecord(&record, rid) ==
       DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::OPERATION_OK ) {
    *resultValue = record.getAdaptedObjRef();
    return true;
  } else
    return false;
}
