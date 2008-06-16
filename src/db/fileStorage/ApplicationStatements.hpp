#ifndef __DBENTITYSTORAGE_APPLICATIONSTATEMENTS_HPP__
# define __DBENTITYSTORAGE_APPLICATIONSTATEMENTS_HPP__ 1

# include "DBEntityStorageStatement.hpp"

# include "InfoSme_Id_Mapping_Entity.hpp"
# include "InfoSme_Id_Mapping_DBEntityStorage.hpp"
# include <db/fileStorage/Exceptions.hpp>

# include <logger/Logger.h>
# include <core/synchronization/Mutex.hpp>
# include <core/synchronization/RecursiveMutex.hpp>
# include <core/buffers/RefPtr.hpp>
# include "StorageHelper.hpp"

using smsc::logger::Logger;

class SelectInfoSme_Id_Mapping_SmscId_criterion  : public DBEntityStorageStatement
{
public:
  SelectInfoSme_Id_Mapping_SmscId_criterion(InfoSme_Id_Mapping_DBEntityStorage* dataSource) : _dataSource(dataSource) {}
  virtual void setString(int pos, const char* str, bool null=false) throw(SQLException);

  virtual ResultSet* executeQuery() throw(SQLException);

  class InfoSme_Id_Mapping_ResultSet : public DBEntityStorageResultSet
  {
  public:
    InfoSme_Id_Mapping_ResultSet(InfoSme_Id_Mapping_DBEntityStorage::InfoSme_Id_Mapping_DbIterator* dbIterator) 
      : _dbIterator(dbIterator), _logger(Logger::getInstance("dbStrg")) {}

    ~InfoSme_Id_Mapping_ResultSet() {}
    virtual bool fetchNext() throw(SQLException);

    virtual uint64_t getUint64(int pos) throw(SQLException, InvalidArgumentException);

    virtual const char* getString(int pos) throw(SQLException, InvalidArgumentException);

  private:
    InfoSme_Id_Mapping_DBEntityStorage::InfoSme_Id_Mapping_DbIterator* _dbIterator;
    InfoSme_Id_Mapping_Entity _fetchedValue;
    smsc::logger::Logger *_logger;
  };

private:
  std::string _smscId;

  InfoSme_Id_Mapping_DBEntityStorage* _dataSource;
};

#include "InfoSme_T_Entity.hpp"
#include "InfoSme_T_DBEntityStorage.hpp"

class SelectInfoSme_T_stateAndSendDate_criterion : public DBEntityStorageStatement
{
public:
  SelectInfoSme_T_stateAndSendDate_criterion(const StorageHelper::InfoSme_T_storage_ref_t& dataSource)
    : _dataSource(dataSource), _logger(Logger::getInstance("dbStrg")) {}

  virtual void setUint8(int pos, uint8_t val, bool null=false) throw(SQLException);

  virtual void setDateTime(int pos, time_t val, bool null=false) throw(SQLException);

  virtual ResultSet* executeQuery() throw(SQLException);

  class InfoSme_T_ResultSet : public DBEntityStorageResultSet
  {
  public:
    InfoSme_T_ResultSet(InfoSme_T_DBEntityStorage::InfoSme_T_DbIterator* dbIterator,
                        const InfoSme_T_Entity::StateANDSDate_key& maxKey) : _dbIterator(dbIterator), _maxKey(maxKey), _logger(Logger::getInstance("dbStrg")) {}

    virtual bool fetchNext() throw(SQLException);

    virtual bool isNull(int pos) throw(SQLException, InvalidArgumentException);

    virtual const char* getString(int pos)
      throw(SQLException, InvalidArgumentException);

    virtual uint64_t getUint64(int pos)
      throw(SQLException, InvalidArgumentException);
  private:
    InfoSme_T_DBEntityStorage::InfoSme_T_DbIterator* _dbIterator;
    InfoSme_T_Entity::StateANDSDate_key _maxKey;
    InfoSme_T_Entity _fetchedValue;
    smsc::logger::Logger *_logger;
  };

private:
  uint8_t _message_state;
  time_t  _message_date_time;

  StorageHelper::InfoSme_T_storage_ref_t _dataSource;

  smsc::logger::Logger *_logger;
};

class SelectInfoSme_T_fullTableScan : public DBEntityStorageStatement
{
public:
  SelectInfoSme_T_fullTableScan(StorageHelper::InfoSme_T_storage_ref_t dataSource)
    : _dataSource(dataSource), _logger(Logger::getInstance("dbStrg")) {}

  virtual ResultSet* executeQuery() throw(SQLException);

  class InfoSme_T_ResultSet : public DBEntityStorageResultSet
  {
  public:
    InfoSme_T_ResultSet(StorageHelper::InfoSme_T_storage_ref_t dataSource) : _iterator(dataSource->getIterator()), _logger(Logger::getInstance("dbStrg")) {}

    ~InfoSme_T_ResultSet() { delete _iterator; }

    virtual bool fetchNext() throw(SQLException);

    virtual bool isNull(int pos) throw(SQLException, InvalidArgumentException);

    virtual const char* getString(int pos)
      throw(SQLException, InvalidArgumentException);

    virtual uint64_t getUint64(int pos)
      throw(SQLException, InvalidArgumentException);

    virtual uint8_t getUint8(int pos)
      throw(SQLException, InvalidArgumentException);

    virtual time_t getDateTime(int pos)
      throw(SQLException, InvalidArgumentException);
  private:
    DbIterator<InfoSme_T_Entity>* _iterator;
    InfoSme_T_Entity _fetchedValue;
    smsc::logger::Logger *_logger;
  };

private:
  StorageHelper::InfoSme_T_storage_ref_t _dataSource;
  smsc::logger::Logger *_logger;
};

class Insert_into_InfoSme_T  : public DBEntityStorageStatement
{
public:
  Insert_into_InfoSme_T(StorageHelper::InfoSme_T_storage_ref_t dataSource) : _dataSource(dataSource), _logger(Logger::getInstance("dbStrg")) {}

  virtual void setUint8(int pos, uint8_t val, bool null=false) throw(SQLException);

  virtual void setString(int pos, const char* str, bool null=false) throw(SQLException);

  virtual void setDateTime(int pos, time_t time, bool null=false) throw(SQLException);

  virtual uint32_t executeUpdate() throw(SQLException);

private:
  uint8_t _state;
  std::string _abonentAddress;
  time_t _sendDate;
  std::string _message;
  std::string _regionId;

  StorageHelper::InfoSme_T_storage_ref_t _dataSource;

  static uint64_t getIdSequenceNumber();
  static uint64_t _idSequenceNumber;

  smsc::logger::Logger *_logger;
};

#include "InfoSme_Generating_Tasks_DBEntityStorage.hpp"

class Insert_into_InfoSme_Generating_Tasks : public DBEntityStorageStatement
{
public:
  Insert_into_InfoSme_Generating_Tasks(InfoSme_Generating_Tasks_DBEntityStorage* dataSource) : _dataSource(dataSource),_logger(Logger::getInstance("dbStrg")) {}

  virtual void setString(int pos, const char* str, bool null=false) throw(SQLException);

  virtual uint32_t executeUpdate() throw(SQLException);

private:
  std::string _taskId;

  InfoSme_Generating_Tasks_DBEntityStorage* _dataSource;
  // DEBUG
  smsc::logger::Logger *_logger;
};

class Delete_from_InfoSme_Generating_Tasks_By_TaskId : public DBEntityStorageStatement
{
public:
  Delete_from_InfoSme_Generating_Tasks_By_TaskId(InfoSme_Generating_Tasks_DBEntityStorage* dataSource) : _dataSource(dataSource), _logger(Logger::getInstance("dbStrg")) {}

  virtual void setString(int pos, const char* str, bool null=false) throw(SQLException);

  virtual uint32_t executeUpdate() throw(SQLException);
private:
  std::string _taskId;
  InfoSme_Generating_Tasks_DBEntityStorage* _dataSource;
  smsc::logger::Logger *_logger;
};

class Delete_from_InfoSme_T_By_Id : public DBEntityStorageStatement
{
public:
  Delete_from_InfoSme_T_By_Id(StorageHelper::InfoSme_T_storage_ref_t dataSource) : _dataSource(dataSource), _logger(Logger::getInstance("dbStrg")) {}

  virtual void setUint64(int pos, uint64_t val, bool null=false) throw(SQLException);

  virtual uint32_t executeUpdate() throw(SQLException);
private:
  uint64_t _msgId;
  StorageHelper::InfoSme_T_storage_ref_t _dataSource;
  smsc::logger::Logger *_logger;
};

class Delete_from_InfoSme_T_By_State : public DBEntityStorageStatement
{
public:
  Delete_from_InfoSme_T_By_State(StorageHelper::InfoSme_T_storage_ref_t dataSource) : _dataSource(dataSource), _iterator(dataSource->getIterator()), _logger(Logger::getInstance("dbStrg")) {}

  ~Delete_from_InfoSme_T_By_State() { delete _iterator; }

  virtual void setUint8(int pos, uint8_t val, bool null=false) throw(SQLException);

  virtual uint32_t executeUpdate() throw(SQLException);
private:
  uint64_t _msgState;
  StorageHelper::InfoSme_T_storage_ref_t _dataSource;
  DbIterator<InfoSme_T_Entity>* _iterator;
  smsc::logger::Logger *_logger;
};

class Delete_from_InfoSme_T_By_StateAndAbonent : public DBEntityStorageStatement
{
public:
  Delete_from_InfoSme_T_By_StateAndAbonent(StorageHelper::InfoSme_T_storage_ref_t dataSource) : _dataSource(dataSource), _iterator(dataSource->getIterator()), _logger(Logger::getInstance("dbStrg")) {}

  ~Delete_from_InfoSme_T_By_StateAndAbonent() { delete _iterator; }

  virtual void setUint8(int pos, uint8_t val, bool null=false) throw(SQLException);
  virtual void setString(int pos,  const char* val, bool null=false) throw(SQLException);

  virtual uint32_t executeUpdate() throw(SQLException);
private:
  uint64_t _msgState;
  std::string _abonentAddress;
  StorageHelper::InfoSme_T_storage_ref_t _dataSource;
  DbIterator<InfoSme_T_Entity>* _iterator;
  smsc::logger::Logger *_logger;
};

class Update_InfoSme_T_Set_NewState_By_OldState : public DBEntityStorageStatement
{
public:
  Update_InfoSme_T_Set_NewState_By_OldState(StorageHelper::InfoSme_T_storage_ref_t dataSource) : _dataSource(dataSource), _iterator(dataSource->getIterator()), _logger(Logger::getInstance("dbStrg")) {}

  ~Update_InfoSme_T_Set_NewState_By_OldState() { delete _iterator; }

  virtual void setUint8(int pos, uint8_t val, bool null=false) throw(SQLException);

  virtual uint32_t executeUpdate() throw(SQLException);
private:
  uint64_t _newMsgState, _searchState;
  StorageHelper::InfoSme_T_storage_ref_t _dataSource;
  InfoSme_T_DBEntityStorage::InfoSme_T_DbIterator* _iterator;
  smsc::logger::Logger *_logger;
};

class Update_InfoSme_T_Set_State_By_Id : public DBEntityStorageStatement
{
public:
  Update_InfoSme_T_Set_State_By_Id(StorageHelper::InfoSme_T_storage_ref_t dataSource) : _dataSource(dataSource), _logger(Logger::getInstance("dbStrg")) {}
  virtual void setUint8(int pos, uint8_t val, bool null=false) throw(SQLException);
  virtual void setUint64(int pos, uint64_t val, bool null=false) throw(SQLException);

  virtual uint32_t executeUpdate() throw(SQLException);
private:
  uint8_t _newMsgState;
  uint64_t _msgId;
  StorageHelper::InfoSme_T_storage_ref_t _dataSource;
  smsc::logger::Logger *_logger;
};

class Update_InfoSme_T_Set_State_By_IdAndState : public DBEntityStorageStatement
{
public:
  Update_InfoSme_T_Set_State_By_IdAndState(StorageHelper::InfoSme_T_storage_ref_t dataSource) : _dataSource(dataSource), _logger(Logger::getInstance("dbStrg")) {}
  virtual void setUint8(int pos, uint8_t val, bool null=false) throw(SQLException);
  virtual void setUint64(int pos, uint64_t val, bool null=false) throw(SQLException);
  virtual uint32_t executeUpdate() throw(SQLException);
private:
  uint8_t _newMsgState, _oldMsgState;
  uint64_t _msgId;
  StorageHelper::InfoSme_T_storage_ref_t _dataSource;
  smsc::logger::Logger *_logger;
};

class Update_InfoSme_T_Set_StateAndSendDate_By_Id : public DBEntityStorageStatement
{
public:
  Update_InfoSme_T_Set_StateAndSendDate_By_Id(StorageHelper::InfoSme_T_storage_ref_t dataSource) : _dataSource(dataSource), _logger(Logger::getInstance("dbStrg")) {}

  virtual void setUint64(int pos, uint64_t val, bool null=false) throw(SQLException);

  virtual void setUint8(int pos, uint8_t val, bool null=false) throw(SQLException);
  virtual void setDateTime(int pos, time_t val, bool null=false)
    throw(SQLException);
  virtual uint32_t executeUpdate() throw(SQLException);
private:
  uint8_t _newMsgState;
  uint64_t _msgId;
  time_t _dateTime;
  StorageHelper::InfoSme_T_storage_ref_t _dataSource;
  smsc::logger::Logger *_logger;
};

class Update_InfoSme_T_Set_NewMessage_By_Id : public DBEntityStorageStatement
{
public:
  Update_InfoSme_T_Set_NewMessage_By_Id(StorageHelper::InfoSme_T_storage_ref_t dataSource) : _dataSource(dataSource), _logger(Logger::getInstance("dbStrg")) {}

  virtual void setUint64(int pos, uint64_t val, bool null=false) throw(SQLException);

  virtual void setString(int pos, const char* val, bool null=false) throw(SQLException);
  virtual uint32_t executeUpdate() throw(SQLException);
private:
  std::string _newMsg;
  uint64_t _msgId;
  StorageHelper::InfoSme_T_storage_ref_t _dataSource;
  smsc::logger::Logger *_logger;
};

class Insert_into_InfoSme_Id_Mapping : public DBEntityStorageStatement
{
public:
  Insert_into_InfoSme_Id_Mapping(InfoSme_Id_Mapping_DBEntityStorage* dataSource) : _dataSource(dataSource), _logger(Logger::getInstance("dbStrg")) {}

  virtual void setUint64(int pos, uint64_t val, bool null=false) throw(SQLException);

  virtual void setString(int pos, const char* val, bool null=false) throw(SQLException);

  virtual uint32_t executeUpdate() throw(SQLException);

private:
  uint64_t _msgId;
  std::string _taskId, _smscId;
  
  InfoSme_Id_Mapping_DBEntityStorage* _dataSource;
  smsc::logger::Logger *_logger;
};

class Delete_from_InfoSme_Id_Mapping_By_SmscId : public DBEntityStorageStatement
{
public:
  Delete_from_InfoSme_Id_Mapping_By_SmscId(InfoSme_Id_Mapping_DBEntityStorage* dataSource) : _dataSource(dataSource) {}

  virtual void setString(int pos, const char* val, bool null=false) throw(SQLException);

  virtual uint32_t executeUpdate() throw(SQLException);

private:
  std::string _smscId;
  
  InfoSme_Id_Mapping_DBEntityStorage* _dataSource;
};

class Delete_from_InfoSme_Id_Mapping_By_Id : public DBEntityStorageStatement
{
public:
  Delete_from_InfoSme_Id_Mapping_By_Id(InfoSme_Id_Mapping_DBEntityStorage* dataSource) : _dataSource(dataSource), _logger(Logger::getInstance("dbStrg")) {}

  virtual void setUint64(int pos, uint64_t val, bool null=false) throw(SQLException);

  virtual uint32_t executeUpdate() throw(SQLException);

private:
  uint64_t _msgId;
  
  InfoSme_Id_Mapping_DBEntityStorage* _dataSource;
  smsc::logger::Logger *_logger;
};

#include "InfoSme_Tasks_Stat_DBEntityStorage.hpp"

class Delete_from_InfoSme_Tasks_Stat_By_TaskId : public DBEntityStorageStatement
{
public:
  Delete_from_InfoSme_Tasks_Stat_By_TaskId(InfoSme_Tasks_Stat_DBEntityStorage* dataSource) : _dataSource(dataSource), _logger(Logger::getInstance("dbStrg")) {}

  virtual void setString(int pos, const char* val, bool null=false) throw(SQLException);
  virtual uint32_t executeUpdate() throw(SQLException);

private:
  std::string _taskId;

  InfoSme_Tasks_Stat_DBEntityStorage* _dataSource;
  smsc::logger::Logger *_logger;
};

class Insert_into_InfoSme_Tasks_Stat : public DBEntityStorageStatement
{
public:
  Insert_into_InfoSme_Tasks_Stat(InfoSme_Tasks_Stat_DBEntityStorage* dataSource) : _dataSource(dataSource), _logger(Logger::getInstance("dbStrg")) {}

  virtual void setUint32(int pos, uint32_t val, bool null=false) throw(SQLException);
  virtual void setString(int pos, const char* val, bool null=false) throw(SQLException);

  virtual uint32_t executeUpdate() throw(SQLException);

private:
  uint32_t _period, _generated, _delivered, _retried, _failed;
  std::string _taskId;
  
  InfoSme_Tasks_Stat_DBEntityStorage* _dataSource;
  smsc::logger::Logger *_logger;
};

class SelectInfoSme_TasksStat_fullTableScan : public DBEntityStorageStatement
{
public:
  SelectInfoSme_TasksStat_fullTableScan(InfoSme_Tasks_Stat_DBEntityStorage* dataSource)
    : _dataSource(dataSource),  _logger(Logger::getInstance("dbStrg")) {}

  virtual ResultSet* executeQuery() throw(SQLException);

  class InfoSme_Tasks_Stat_ResultSet : public DBEntityStorageResultSet
  {
  public:
    InfoSme_Tasks_Stat_ResultSet(InfoSme_Tasks_Stat_DBEntityStorage* dataSource) : _iterator(dataSource->getIterator()), _logger(Logger::getInstance("dbStrg")) {}

    ~InfoSme_Tasks_Stat_ResultSet() { delete _iterator; }

    virtual bool fetchNext() throw(SQLException);

    virtual bool isNull(int pos) throw(SQLException, InvalidArgumentException);

    virtual const char* getString(int pos)
      throw(SQLException, InvalidArgumentException);

    virtual uint32_t getUint32(int pos)
      throw(SQLException, InvalidArgumentException);

  private:
    DbIterator<InfoSme_Tasks_Stat_Entity>* _iterator;
    InfoSme_Tasks_Stat_Entity _fetchedValue;
    smsc::logger::Logger *_logger;
  };

private:
  InfoSme_Tasks_Stat_DBEntityStorage* _dataSource;
  smsc::logger::Logger *_logger;

};
#endif
