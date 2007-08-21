#include <core/synchronization/Mutex.hpp>
#include "ApplicationStatements.hpp"

#include <core/buffers/RefPtr.hpp>

#ifdef _TEST_CASE_DBEntityStorageStatement_
static smsc::logger::Logger*
initLogger(const char* moduleName)
{
  smsc::logger::Logger::Init();
  return smsc::logger::Logger::getInstance(moduleName);
}

static smsc::logger::Logger* logger=initLogger("dmplx");
#endif

void
SelectInfoSme_T_stateAndSendDate_criterion::setUint8(int pos, uint8_t val, bool null)
  throw(SQLException)
{
  if ( pos == 1 ) 
    _message_state = val;
  else throw SQLException("Wrong position argument number");
}

void
SelectInfoSme_T_stateAndSendDate_criterion::setDateTime(int pos, time_t val, bool null)
  throw(SQLException)
{
  if ( pos == 2 ) 
    _message_date_time = val;
  else throw SQLException("Wrong position argument number");
}

ResultSet*
SelectInfoSme_T_stateAndSendDate_criterion::executeQuery()
  throw(SQLException)
{
  InfoSme_T_Entity::StateANDSDate_key minKey(_message_state, 0);
  InfoSme_T_Entity::StateANDSDate_key maxKey(_message_state, _message_date_time);
  InfoSme_T_DBEntityStorage::InfoSme_T_DbIterator* iterator = _dataSource->getIterator();
  iterator->setIndexSearchCrit(minKey, maxKey);
  DBEntityStorageResultSet* result = new SelectInfoSme_T_stateAndSendDate_criterion::InfoSme_T_ResultSet(iterator, maxKey);

  return result;
}

bool
SelectInfoSme_T_stateAndSendDate_criterion::InfoSme_T_ResultSet::fetchNext()
  throw(SQLException)
{
  InfoSme_T_Entity resultValue;

  bool isFetched = _dbIterator->nextValue(&resultValue);
  if ( isFetched && _maxKey  < InfoSme_T_Entity::StateANDSDate_key(resultValue) )
    isFetched = false;

  if (isFetched) {
    _fetchedValue = resultValue;
    smsc_log_info(_logger, "SelectInfoSme_T_stateAndSendDate_criterion::: next record of Infosme_T_=[%s]", _fetchedValue.toString().c_str());
  } else
    smsc_log_info(_logger, "SelectInfoSme_T_stateAndSendDate_criterion::: no more records of Infosme_T_");

  return isFetched;
}

uint64_t
SelectInfoSme_T_stateAndSendDate_criterion::InfoSme_T_ResultSet::getUint64(int pos)
  throw(SQLException, InvalidArgumentException)
{
  if ( pos == 1 )
    return _fetchedValue.getId();
  else throw InvalidArgumentException();
}

const char*
SelectInfoSme_T_stateAndSendDate_criterion::InfoSme_T_ResultSet::getString(int pos)
  throw(SQLException, InvalidArgumentException)
{
  if ( pos == 2 )
    return _fetchedValue.getAbonentAddress().c_str();
  else if ( pos == 3 )
    return _fetchedValue.getMessage().c_str();
  else if ( pos == 4 )
    return _fetchedValue.getRegionId().c_str();
  else throw InvalidArgumentException();
}

bool
SelectInfoSme_T_stateAndSendDate_criterion::InfoSme_T_ResultSet::isNull(int pos)
  throw(SQLException, InvalidArgumentException)
{
  return false;
}

ResultSet*
SelectInfoSme_T_fullTableScan::executeQuery()
  throw(SQLException)
{
  DBEntityStorageResultSet* result = new SelectInfoSme_T_fullTableScan::InfoSme_T_ResultSet(_dataSource);

  return result;
}

bool
SelectInfoSme_T_fullTableScan::InfoSme_T_ResultSet::fetchNext()
  throw(SQLException)
{
  InfoSme_T_Entity resultValue;
  bool isFetched = _iterator->nextValue(&resultValue);

  if (isFetched) {
    _fetchedValue = resultValue;
    smsc_log_info(_logger, "SelectInfoSme_T_fullTableScan::: next record of Infosme_T_=[%s]", _fetchedValue.toString().c_str());
  } else
    smsc_log_info(_logger, "SelectInfoSme_T_fullTableScan::: no more records of Infosme_T_");

  return isFetched;
}

uint64_t
SelectInfoSme_T_fullTableScan::InfoSme_T_ResultSet::getUint64(int pos)
  throw(SQLException, InvalidArgumentException)
{
  if ( pos == 1 )
    return _fetchedValue.getId();
  else throw InvalidArgumentException();
}

uint8_t
SelectInfoSme_T_fullTableScan::InfoSme_T_ResultSet::getUint8(int pos)
  throw(SQLException, InvalidArgumentException)
{
  if ( pos == 2 )
    return _fetchedValue.getState();
  else throw InvalidArgumentException();
}

time_t
SelectInfoSme_T_fullTableScan::InfoSme_T_ResultSet::getDateTime(int pos)
  throw(SQLException, InvalidArgumentException)
{
  if ( pos == 4 )
    return _fetchedValue.getSendDate();
  else throw InvalidArgumentException();
}

const char*
SelectInfoSme_T_fullTableScan::InfoSme_T_ResultSet::getString(int pos)
  throw(SQLException, InvalidArgumentException)
{
  if ( pos == 3 )
    return _fetchedValue.getAbonentAddress().c_str();
  else if ( pos == 5 )
    return _fetchedValue.getMessage().c_str();
  else throw InvalidArgumentException();
}

bool
SelectInfoSme_T_fullTableScan::InfoSme_T_ResultSet::isNull(int pos)
  throw(SQLException, InvalidArgumentException)
{
  return false;
}

void
Insert_into_InfoSme_T::setUint8(int pos, uint8_t val, bool null)
  throw(SQLException)
{
  if ( pos == 1 ) {
    _state = val;
  } else throw SQLException("Wrong position argument number");
}

void
Insert_into_InfoSme_T::setString(int pos, const char* str, bool null)
  throw(SQLException)
{
  if ( pos == 2 )
    _abonentAddress = str;
  else if ( pos == 4 )
    _message = str;
  else if ( pos == 5 )
    _regionId = str;
  else throw SQLException("Wrong position argument number");
}

void
Insert_into_InfoSme_T::setDateTime(int pos, time_t time, bool null)
  throw(SQLException)
{
  if ( pos == 3 )
    _sendDate = time;
  else throw SQLException("Wrong position argument number");
}

uint64_t
Insert_into_InfoSme_T::getIdSequenceNumber() { return _idSequenceNumber++; }

uint64_t
Insert_into_InfoSme_T::_idSequenceNumber = 0;

uint32_t
Insert_into_InfoSme_T::executeUpdate()
  throw(SQLException)
{
  InfoSme_T_Entity newValue(_dataSource->getNextIdSequenceNumber(),
                            _state,
                            _abonentAddress,
                            _sendDate,
                            _message,
                            _regionId);

  if ( _dataSource->putValue(newValue) ) {
    smsc_log_info(_logger, "Insert_into_InfoSme_T::executeUpdate::: the new record=[%s] was inserted into Infosme_T_", newValue.toString().c_str());
    return 1;
  } else throw SQLException("Insert_into_InfoSme_T::executeUpdate::: can't insert value");
}

void
Delete_from_InfoSme_T_By_Id::setUint64(int pos, uint64_t val, bool null)
  throw(SQLException)
{
  if ( pos == 1 ) 
    _msgId = val;
  else throw SQLException("Wrong position argument number");
}

uint32_t
Delete_from_InfoSme_T_By_Id::executeUpdate()
  throw(SQLException)
{
  int ret = _dataSource->eraseValue(InfoSme_T_Entity::Id_Key(_msgId));
  if ( ret < 0 )
    throw SQLException("Delete_from_InfoSme_T_By_Id::executeUpdate::: can't delete record");
  smsc_log_info(_logger, "Delete_from_InfoSme_T_By_Id::executeUpdate::: %d records with msgId=%ld were deleted", ret, _msgId);
  return ret;
}

void
Delete_from_InfoSme_T_By_State::setUint8(int pos, uint8_t val, bool null)
  throw(SQLException)
{
  if ( pos == 1 )
    _msgState = val;
  else throw SQLException("Wrong position argument number");
}

uint32_t
Delete_from_InfoSme_T_By_State::executeUpdate() throw(SQLException)
{
  uint32_t rowNum=0;
  InfoSme_T_Entity resultValue;

  while(_iterator->nextValue(&resultValue)) {
    if (resultValue.getState() == _msgState) {
      int eraseRet =_dataSource->eraseValue(InfoSme_T_Entity::Id_Key(resultValue));
      if ( eraseRet < 0 )
        throw SQLException("Delete_from_InfoSme_T_By_State::executeUpdate::: can't delete record");
      rowNum += eraseRet;
    }
  }
  smsc_log_info(_logger, "Delete_from_InfoSme_T_By_State::executeUpdate::: %d records with state=[%lld] were deleted", rowNum, _msgState);
  return rowNum;
}

void
Delete_from_InfoSme_T_By_StateAndAbonent::setUint8(int pos, uint8_t val, bool null)
  throw(SQLException)
{
  if ( pos == 1 )
    _msgState = val;
  else throw SQLException("Wrong position argument number");
}

void
Delete_from_InfoSme_T_By_StateAndAbonent::setString(int pos,  const char* val, bool null)
  throw(SQLException)
{
  if ( pos == 2 )
    _abonentAddress = val;
  else throw SQLException("Wrong position argument number");
}

uint32_t
Delete_from_InfoSme_T_By_StateAndAbonent::executeUpdate()
  throw(SQLException)
{
  uint32_t rowNum=0;
  InfoSme_T_Entity resultValue;
  int ret;
  while(_iterator->nextValue(&resultValue)) {
    if (resultValue.getState() == _msgState &&
        resultValue.getAbonentAddress() == _abonentAddress) {
      if ( (ret=_dataSource->eraseValue(InfoSme_T_Entity::Id_Key(resultValue))) < 0 )
        throw SQLException("Delete_from_InfoSme_T_By_StateAndAbonent::executeUpdate::: can't delete record");
      rowNum += ret;
    }
  }
  smsc_log_info(_logger, "Delete_from_InfoSme_T_By_StateAndAbonent::executeUpdate::: %d records with state=[%lld]/abonentAddress=[%s] were deleted ", rowNum, _msgState, _abonentAddress.c_str());
  return rowNum;
}

void
Update_InfoSme_T_Set_NewState_By_OldState::setUint8(int pos, uint8_t val, bool null)
  throw(SQLException)
{
  if ( pos == 1 )
    _newMsgState = val;
  else if ( pos == 2 )
    _searchState = val;
}

uint32_t
Update_InfoSme_T_Set_NewState_By_OldState::executeUpdate()
  throw(SQLException)
{
  uint32_t rowNum=0;
  InfoSme_T_Entity oldValue;

  int st;

  _iterator->setIndexSearchCrit(InfoSme_T_Entity::StateANDSDate_key(_searchState, 0),
                                InfoSme_T_Entity::StateANDSDate_key(_searchState, 0xffffffff));

  while(_iterator->nextValue(&oldValue)) {
    InfoSme_T_Entity::Id_Key primaryKey(oldValue);

    InfoSme_T_Entity newValue(oldValue.getId(),
                              _newMsgState,
                              oldValue.getAbonentAddress(),
                              oldValue.getSendDate(),
                              oldValue.getMessage(),
                              oldValue.getRegionId());

    if ( (st = _dataSource->updateValue(primaryKey, oldValue, newValue)) < 0 )
      throw SQLException("Update_InfoSme_T_Set_NewState_By_OldState::executeUpdate::: can't update record");

    smsc_log_info(_logger, "Update_InfoSme_T_Set_NewState_By_OldState::executeUpdate::: set new value=[%s] for row with key=[%s],old value=[%s]",newValue.toString().c_str(), primaryKey.toString().c_str(), oldValue.toString().c_str());
    rowNum += st;
  }

  return rowNum;
}

void
Update_InfoSme_T_Set_State_By_Id::setUint8(int pos, uint8_t val, bool null)
  throw(SQLException)
{
  if (pos == 1)
    _newMsgState = val;
  else throw SQLException("Wrong position argument number");
}

void
Update_InfoSme_T_Set_State_By_Id::setUint64(int pos, uint64_t val, bool null)
  throw(SQLException)
{
  if (pos ==2)
    _msgId = val;
  else throw SQLException("Wrong position argument number");
}

uint32_t
Update_InfoSme_T_Set_State_By_Id::executeUpdate()
  throw(SQLException)
{
  InfoSme_T_Entity::Id_Key primaryKey(_msgId);

  InfoSme_T_Entity oldValue;
  bool ret = _dataSource->findValue(primaryKey, &oldValue);
  if (ret) {
    InfoSme_T_Entity newValue(oldValue.getId(),
                              _newMsgState,
                              oldValue.getAbonentAddress(),
                              oldValue.getSendDate(),
                              oldValue.getMessage(),
                              oldValue.getRegionId());

    int updateRes = _dataSource->updateValue(primaryKey, oldValue, newValue);
    if ( updateRes < 0 )
      throw SQLException("Update_InfoSme_T_Set_State_By_Id::executeUpdate::: can't update record");

    smsc_log_info(_logger, "Update_InfoSme_T_Set_State_By_Id::executeUpdate::: set new value=[%s] for row with key=[%s]", newValue.toString().c_str(), primaryKey.toString().c_str());

    return updateRes;
  } else return 0;
}

void
Update_InfoSme_T_Set_State_By_IdAndState::setUint8(int pos, uint8_t val, bool null)
  throw(SQLException)
{
  if (pos == 1)
    _newMsgState = val;
  else if ( pos == 3 ) 
    _oldMsgState = val;
  else
    throw SQLException("Wrong position argument number");
}

void
Update_InfoSme_T_Set_State_By_IdAndState::setUint64(int pos, uint64_t val, bool null)
  throw(SQLException)
{
  if (pos == 2)
    _msgId = val;
  else throw SQLException("Wrong position argument number");
}

uint32_t
Update_InfoSme_T_Set_State_By_IdAndState::executeUpdate()
  throw(SQLException)
{
  InfoSme_T_Entity::Id_Key primaryKey(_msgId);

  InfoSme_T_Entity oldValue;
  bool ret = _dataSource->findValue(primaryKey, &oldValue);

  if ( ret && oldValue.getState() == _oldMsgState ) {
    InfoSme_T_Entity newValue(oldValue.getId(),
                              _newMsgState,
                              oldValue.getAbonentAddress(),
                              oldValue.getSendDate(),
                              oldValue.getMessage(),
                              oldValue.getRegionId());
    int updateRes = _dataSource->updateValue(primaryKey, oldValue, newValue);
    if ( updateRes < 0 )
      throw SQLException("Update_InfoSme_T_Set_State_By_IdAndState::executeUpdate::: can't update record");

    smsc_log_info(_logger, "Update_InfoSme_T_Set_State_By_IdAndState::executeUpdate::: set new value=[%s] for row with key=[%s]", newValue.toString().c_str(), primaryKey.toString().c_str());
    return updateRes;
  } else
    return 0;
}

void
Update_InfoSme_T_Set_StateAndSendDate_By_Id::setUint64(int pos, uint64_t val, bool null) throw(SQLException)
{
  if ( pos == 3 )
    _msgId = val;
  else throw SQLException("Wrong position argument number");
}

void
Update_InfoSme_T_Set_StateAndSendDate_By_Id::setUint8(int pos, uint8_t val, bool null) throw(SQLException)
{
  if ( pos == 1 )
    _newMsgState = val;
  else throw SQLException("Wrong position argument number");
}

void
Update_InfoSme_T_Set_StateAndSendDate_By_Id::setDateTime(int pos, time_t val, bool null)
  throw(SQLException)
{
  if ( pos == 2 )
    _dateTime = val;
  else throw SQLException("Wrong position argument number");
}

uint32_t
Update_InfoSme_T_Set_StateAndSendDate_By_Id::executeUpdate()
  throw(SQLException)
{
  InfoSme_T_Entity::Id_Key primaryKey(_msgId);

  InfoSme_T_Entity oldValue;
  bool ret = _dataSource->findValue(primaryKey, &oldValue);
  if ( ret ) {
    InfoSme_T_Entity newValue(oldValue.getId(),
                              _newMsgState,
                              oldValue.getAbonentAddress(),
                              _dateTime,
                              oldValue.getMessage(),
                              oldValue.getRegionId());
    int updateRes = _dataSource->updateValue(primaryKey, oldValue, newValue);
    if ( updateRes < 0 )
      throw SQLException("Update_InfoSme_T_Set_StateAndSendDate_By_Id::executeUpdate::: can't update record");

    smsc_log_info(_logger, "Update_InfoSme_T_Set_State_By_Id::executeUpdate::: set new value=[%s] for row with key=[%s]", newValue.toString().c_str(), primaryKey.toString().c_str());
    return updateRes;
  } else
    return 0;
}

void
Update_InfoSme_T_Set_NewMessage_By_Id::setUint64(int pos, uint64_t val, bool null)
  throw(SQLException)
{
  if ( pos == 2 )
    _msgId = val;
  else throw SQLException("Wrong position argument number");
}

void
Update_InfoSme_T_Set_NewMessage_By_Id::setString(int pos, const char* val, bool null)
  throw(SQLException)
{
  if ( pos == 1 )
    _newMsg = val;
  else throw SQLException("Wrong position argument number");
}

uint32_t
Update_InfoSme_T_Set_NewMessage_By_Id::executeUpdate()
  throw(SQLException)
{
  InfoSme_T_Entity::Id_Key primaryKey(_msgId);

  InfoSme_T_Entity oldValue;
  bool ret = _dataSource->findValue(primaryKey, &oldValue);
  if ( ret ) {
    InfoSme_T_Entity newValue(oldValue.getId(),
                              oldValue.getState(),
                              oldValue.getAbonentAddress(),
                              oldValue.getSendDate(),
                              _newMsg,
                              oldValue.getRegionId());
    int updateRes;
    if ( _newMsg.size() == oldValue.getMessage().size() ) {
      updateRes = _dataSource->updateValue(primaryKey, oldValue, newValue);
    } else {
      if ( _dataSource->eraseValue(primaryKey) != 1 )
        throw SQLException("Update_InfoSme_T_Set_NewMessage_By_Id::executeUpdate::: can't replace record");
      if ( !_dataSource->putValue(newValue) )
        throw SQLException("Update_InfoSme_T_Set_NewMessage_By_Id::executeUpdate::: can't replace record (new value)");
    }
    if ( updateRes < 0 )
      throw SQLException("Update_InfoSme_T_Set_NewMessage_By_Id::executeUpdate::: can't update record");

    return updateRes;
  } else
    return 0;
}

//-----------------------------------------------------------------------------

void
SelectInfoSme_Id_Mapping_SmscId_criterion::setString(int pos, const char* str, bool null) 
  throw(SQLException)
{
  if ( pos == 1 )
    _smscId = str;
  else throw SQLException("Wrong position argument number");
}

ResultSet*
SelectInfoSme_Id_Mapping_SmscId_criterion::executeQuery()
  throw(SQLException)
{
  InfoSme_Id_Mapping_Entity::SmscId_Key key(_smscId);
  InfoSme_Id_Mapping_DBEntityStorage::InfoSme_Id_Mapping_DbIterator* dbIterator = _dataSource->getIterator();
  dbIterator->setIndexSearchCrit(key);
  DBEntityStorageResultSet* result = new SelectInfoSme_Id_Mapping_SmscId_criterion::InfoSme_Id_Mapping_ResultSet(dbIterator);
  return result;
}

bool
SelectInfoSme_Id_Mapping_SmscId_criterion::InfoSme_Id_Mapping_ResultSet::fetchNext()
  throw(SQLException)
{
  InfoSme_Id_Mapping_Entity resultValue;
  bool isFetched = _dbIterator->nextValue(&resultValue);

  if (isFetched) {
    _fetchedValue = resultValue;
    smsc_log_info(_logger, "SelectInfoSme_Id_Mapping_SmscId_criterion::: next record of InfoSme_Id_Mapping=[%s]", _fetchedValue.toString().c_str());
  } else
    smsc_log_info(_logger, "SelectInfoSme_Id_Mapping_SmscId_criterion::: no more records of InfoSme_Id_Mapping");

  return isFetched;
}

uint64_t
SelectInfoSme_Id_Mapping_SmscId_criterion::InfoSme_Id_Mapping_ResultSet::getUint64(int pos)
  throw(SQLException, InvalidArgumentException)
{
  if ( pos == 1 ) {
    return _fetchedValue.getId();
  } else throw InvalidArgumentException();
}

const char*
SelectInfoSme_Id_Mapping_SmscId_criterion::InfoSme_Id_Mapping_ResultSet::getString(int pos)
  throw(SQLException, InvalidArgumentException)
{
  if ( pos == 2 ) {
    return _fetchedValue.getTaskId().c_str();
  } else throw InvalidArgumentException();
}

void
Insert_into_InfoSme_Id_Mapping::setUint64(int pos, uint64_t val, bool null) throw(SQLException)
{
  if ( pos == 1 ) _msgId = val;
  else  SQLException("Wrong position argument number");
}

void
Insert_into_InfoSme_Id_Mapping::setString(int pos, const char* val, bool null) throw(SQLException)
{
  if ( pos == 2 ) _smscId = val;
  else if ( pos == 3 ) _taskId = val;
  else  SQLException("Wrong position argument number");
}

uint32_t
Insert_into_InfoSme_Id_Mapping::executeUpdate() throw(SQLException)
{
  InfoSme_Id_Mapping_Entity newRecord(_msgId,
                                      _smscId,
                                      _taskId);

  if ( _dataSource->putValue(newRecord) ) {
    smsc_log_info(_logger, "Insert_into_InfoSme_Id_Mapping::executeUpdate::: the new record=[%s] was inserted", newRecord.toString().c_str());
    return 1;
  } else return 0;
}

void Delete_from_InfoSme_Id_Mapping_By_SmscId::setString(int pos, const char* val, bool null)
  throw(SQLException)
{
  if ( pos == 1 )
    _smscId = val;
  else  SQLException("Wrong position argument number");
}

uint32_t
Delete_from_InfoSme_Id_Mapping_By_SmscId::executeUpdate()
  throw(SQLException)
{
  uint32_t rowNum=0;
  InfoSme_Id_Mapping_Entity resultValue;

  int eraseRet;

  InfoSme_Id_Mapping_Entity::SmscId_Key smscid_key(_smscId);
  smsc::core::buffers::RefPtr<smsc::core::synchronization::RecursiveMutex, smsc::core::synchronization::Mutex> mutex;
  {
    smsc::core::synchronization::MutexGuard mutexGuard(InfoSme_Id_Mapping_Entity::_mutexRegistryLock_ForSmscIdExAccess);

    mutex = InfoSme_Id_Mapping_Entity::_mutexRegistry_ForSmscIdExAccess.getObject(smscid_key);
  }

  if ( mutex.Get() ) {
    {
      smsc::core::synchronization::RecursiveMutexGuard mutexGuard(*mutex);

      InfoSme_Id_Mapping_DBEntityStorage::InfoSme_Id_Mapping_DbIterator* dbIterator = _dataSource->getIterator();
      dbIterator->setIndexSearchCrit(smscid_key);

      InfoSme_Id_Mapping_Entity resultValue;
      bool isFetched = dbIterator->nextValue(&resultValue);
      while (isFetched ) {
        eraseRet = _dataSource->eraseValue(InfoSme_Id_Mapping_Entity::Id_Key(resultValue.getId()));
        if ( !eraseRet )
          throw SQLException("Delete_from_InfoSme_Id_Mapping_By_SmscId::executeUpdate::: can't delete value by primary key");
        ++rowNum;
        isFetched = dbIterator->nextValue(&resultValue);
      }

    }
    smsc::core::synchronization::MutexGuard mutexGuard(InfoSme_Id_Mapping_Entity::_mutexRegistryLock_ForSmscIdExAccess);
    InfoSme_Id_Mapping_Entity::_mutexRegistry_ForSmscIdExAccess.toUnregisterObject(smscid_key);
    
  }
  return rowNum;
}

void
Delete_from_InfoSme_Id_Mapping_By_Id::setUint64(int pos, uint64_t val, bool null)
  throw(SQLException)
{
  if ( pos == 1 )
    _msgId = val;
  else  SQLException("Wrong position argument number");
}

uint32_t
Delete_from_InfoSme_Id_Mapping_By_Id::executeUpdate()
  throw(SQLException)
{
  int ret = _dataSource->eraseValue(InfoSme_Id_Mapping_Entity::Id_Key(_msgId));
  if ( ret < 0 )
    throw SQLException("Delete_from_InfoSme_Id_Mapping_By_Id::executeUpdate::: can't delete record");
  smsc_log_info(_logger, "Delete_from_InfoSme_Id_Mapping_By_Id::executeUpdate::: %d records were deleted for msgId=%ld", ret, _msgId);
  return ret;
}

//=============================================================================

void
Insert_into_InfoSme_Generating_Tasks::setString(int pos, const char* str, bool null)
  throw(SQLException)
{
  if ( pos == 1 )
    _taskId = str;
  else throw SQLException("Wrong position argument number");
}

#include "InfoSme_Generating_Tasks_Entity.hpp"

uint32_t
Insert_into_InfoSme_Generating_Tasks::executeUpdate()
  throw(SQLException)
{
  InfoSme_Generating_Tasks_Entity newValue(_taskId);

  if ( _dataSource->putValue(newValue) ) {
    smsc_log_info(_logger, "Insert_into_InfoSme_Generating_Tasks::executeUpdate::: the new record=[%s] was inserted into InfoSme_Generating_Tasks", newValue.toString().c_str());
    return 1;
  } else return 0;
}

//=============================================================================

void
Delete_from_InfoSme_Generating_Tasks_By_TaskId::setString(int pos, const char* str, bool null)
  throw(SQLException)
{
  if ( pos == 1 )
    _taskId = str;
  else
    throw SQLException("Wrong position argument number");
}

uint32_t
Delete_from_InfoSme_Generating_Tasks_By_TaskId::executeUpdate()
  throw(SQLException)
{
  int eraseRet = _dataSource->eraseValue(InfoSme_Generating_Tasks_Entity::TaskId_Key(_taskId));
  if ( eraseRet < 0 )
    throw SQLException("Delete_from_InfoSme_Generating_Tasks_By_TaskId::executeUpdate::: can't delete record");
  smsc_log_info(_logger, "Delete_from_InfoSme_Generating_Tasks_By_TaskId::executeUpdate::: %d records with taskId=%s were deleted ", eraseRet, _taskId.c_str());
  return eraseRet;
}

//=============================================================================

void
Delete_from_InfoSme_Tasks_Stat_By_TaskId::setString(int pos, const char* val, bool null)
  throw(SQLException)
{
  if ( pos == 1 )
    _taskId = val;
  else throw SQLException("Wrong position argument number");
}

uint32_t
Delete_from_InfoSme_Tasks_Stat_By_TaskId::executeUpdate()
  throw(SQLException)
{
  int eraseRet;
  eraseRet = _dataSource->eraseValue(InfoSme_Tasks_Stat_Entity::TaskId_Key(_taskId));
  if ( eraseRet < 0 )
    throw SQLException("Delete_from_InfoSme_Tasks_Stat_By_TaskId::executeUpdate::: can't delete record");
  smsc_log_info(_logger, "Delete_from_InfoSme_Tasks_Stat_By_TaskId::executeUpdate::: %d records with taskId=%s were deleted", eraseRet, _taskId.c_str());
  return eraseRet;
}

void
Insert_into_InfoSme_Tasks_Stat::setUint32(int pos, uint32_t val, bool null)
  throw(SQLException)
{ if ( pos == 2 )
    _period = val;
  else if ( pos == 3 )
    _generated = val;
  else if ( pos == 4 )
    _delivered = val;
  else if ( pos == 5 )
    _retried = val;
  else if ( pos == 6 )
    _failed = val;
  else throw SQLException("Wrong position argument number");
}

void
Insert_into_InfoSme_Tasks_Stat::setString(int pos, const char* val, bool null) throw(SQLException)
{
  if ( pos == 1 )
    _taskId = val;
  else throw SQLException("Wrong position argument number");
}

uint32_t
Insert_into_InfoSme_Tasks_Stat::executeUpdate()
  throw(SQLException)
{
  InfoSme_Tasks_Stat_Entity newValue(_taskId,
                                     _period,
                                     _generated,
                                     _delivered,
                                     _retried,
                                     _failed);
  if ( _dataSource->putValue(newValue) ) {
    smsc_log_info(_logger, "Insert_into_InfoSme_Tasks_Stat::executeUpdate::: the new record=[%s] was inserted into Infosme_Tasks_Stat", newValue.toString().c_str());
    return 1;
  } else return 0;
}

//-----------------------------------------------------------------------------
ResultSet*
SelectInfoSme_TasksStat_fullTableScan::executeQuery()
  throw(SQLException)
{
  DBEntityStorageResultSet* result = new SelectInfoSme_TasksStat_fullTableScan::InfoSme_Tasks_Stat_ResultSet(_dataSource);

  return result;
}

bool
SelectInfoSme_TasksStat_fullTableScan::InfoSme_Tasks_Stat_ResultSet::fetchNext()
  throw(SQLException)
{
  InfoSme_Tasks_Stat_Entity resultValue;
  bool isFetched = _iterator->nextValue(&resultValue);

  if (isFetched) {
    _fetchedValue = resultValue;
    smsc_log_info(_logger, "SelectInfoSme_TasksStat_fullTableScan::: next record of Infosme_Tasks_Stat=[%s]", _fetchedValue.toString().c_str());
  } else
    smsc_log_info(_logger, "SelectInfoSme_TasksStat_fullTableScan::: no more records of Infosme_Tasks_Stat");

  return isFetched;
}

uint32_t
SelectInfoSme_TasksStat_fullTableScan::InfoSme_Tasks_Stat_ResultSet::getUint32(int pos)
  throw(SQLException, InvalidArgumentException)
{
  if ( pos == 2 )
    return _fetchedValue.getPeriod();
  if ( pos == 3 ) 
    return _fetchedValue.getGenerated();
  if ( pos == 4 )
    return _fetchedValue.getDelivered();
  if ( pos == 5 )
    return _fetchedValue.getRetried();
  if ( pos == 6 )
    return _fetchedValue.getFailed();
  else throw InvalidArgumentException();
}

const char*
SelectInfoSme_TasksStat_fullTableScan::InfoSme_Tasks_Stat_ResultSet::getString(int pos)
  throw(SQLException, InvalidArgumentException)
{
  if ( pos == 1 )
    return _fetchedValue.getTaskId().c_str();
  else throw InvalidArgumentException();
}

bool
SelectInfoSme_TasksStat_fullTableScan::InfoSme_Tasks_Stat_ResultSet::isNull(int pos)
  throw(SQLException, InvalidArgumentException)
{
  return false;
}
