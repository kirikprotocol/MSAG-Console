#ifndef __DBENTITYSTORAGE_APPLICATIONSTATEMENTS_HPP__
# define __DBENTITYSTORAGE_APPLICATIONSTATEMENTS_HPP__ 1

# include "DBEntityStorageStatement.hpp"

# include "InfoSme_Id_Mapping_Entity.hpp"
# include "InfoSme_Id_Mapping_DBEntityStorage.hpp"
# include <db/dbEntityStorage/Exceptions.hpp>

# include <logger/Logger.h>

using smsc::logger::Logger;

class SelectInfoSme_Id_Mapping_SmscId_criterion  : public DBEntityStorageStatement
{
public:
  SelectInfoSme_Id_Mapping_SmscId_criterion(InfoSme_Id_Mapping_DBEntityStorage* dataSource) : _dataSource(dataSource), _logger(Logger::getInstance("select1")) {}
  virtual void setString(int pos, const char* str, bool null=false) throw(SQLException);

  virtual ResultSet* executeQuery() throw(SQLException);

  class InfoSme_Id_Mapping_ResultSet : public DBEntityStorageResultSet
  {
  public:
    InfoSme_Id_Mapping_ResultSet(InfoSme_Id_Mapping_DBEntityStorage* dataSource,
                                 const InfoSme_Id_Mapping_Entity::SmscId_Key& key) : _dataSource(dataSource), _key(key), _beginFetch(true), _logger(Logger::getInstance("select1")) {}

    virtual bool fetchNext() throw(SQLException);

    virtual uint64_t getUint64(int pos) throw(SQLException, InvalidArgumentException);

    virtual const char* getString(int pos) throw(SQLException, InvalidArgumentException);

  private:
    InfoSme_Id_Mapping_DBEntityStorage* _dataSource;
    InfoSme_Id_Mapping_Entity::SmscId_Key _key;
    InfoSme_Id_Mapping_Entity _fetchedValue;
    bool _beginFetch;
    // DEBUG
    smsc::logger::Logger *_logger;
  };

private:
  std::string _smscId;

  InfoSme_Id_Mapping_DBEntityStorage* _dataSource;
  // DEBUG
  smsc::logger::Logger *_logger;
};

#include "InfoSme_T_Entity.hpp"
#include "InfoSme_T_DBEntityStorage.hpp"

class SelectInfoSme_T_stateAndSendDate_criterion : public DBEntityStorageStatement
{
public:
  //  SelectInfoSme_T_stateAndSendDate_criterion(InfoSme_T_DBEntityStorage* dataSource)
  SelectInfoSme_T_stateAndSendDate_criterion(InfoSme_T_DBEntityStorage* dataSource)
    : _dataSource(dataSource), _logger(Logger::getInstance("select2")) {}

  virtual void setUint8(int pos, uint8_t val, bool null=false) throw(SQLException);

  virtual void setDateTime(int pos, time_t val, bool null=false) throw(SQLException);

  virtual ResultSet* executeQuery() throw(SQLException);

  class InfoSme_T_ResultSet : public DBEntityStorageResultSet
  {
  public:
    //    InfoSme_T_ResultSet(InfoSme_T_DBEntityStorage* dataSource,
    InfoSme_T_ResultSet(InfoSme_T_DBEntityStorage* dataSource,
                        const InfoSme_T_Entity::StateANDSDate_key& minKey, 
                        const InfoSme_T_Entity::StateANDSDate_key& maxKey) : _dataSource(dataSource), _minKey(minKey), _maxKey(maxKey), _beginSearch(true),_logger(Logger::getInstance("resultset1")) {}

    virtual bool fetchNext() throw(SQLException);

    virtual bool isNull(int pos) throw(SQLException, InvalidArgumentException);

    virtual const char* getString(int pos)
      throw(SQLException, InvalidArgumentException);

    virtual uint64_t getUint64(int pos)
      throw(SQLException, InvalidArgumentException);
  private:
    InfoSme_T_DBEntityStorage* _dataSource;
    InfoSme_T_Entity::StateANDSDate_key _minKey, _maxKey;
    InfoSme_T_Entity _fetchedValue;
    bool _beginSearch;
    smsc::logger::Logger *_logger;
  };

private:
  uint8_t _message_state;
  time_t  _message_date_time;

  InfoSme_T_DBEntityStorage* _dataSource;
  smsc::logger::Logger *_logger;
};

class SelectInfoSme_T_fullTableScan : public DBEntityStorageStatement
{
public:
  SelectInfoSme_T_fullTableScan(InfoSme_T_DBEntityStorage* dataSource)
    : _dataSource(dataSource), _logger(Logger::getInstance("select2")) {}

  virtual ResultSet* executeQuery() throw(SQLException);

  class InfoSme_T_ResultSet : public DBEntityStorageResultSet
  {
  public:
    InfoSme_T_ResultSet(InfoSme_T_DBEntityStorage* dataSource) : _iterator(dataSource->getIterator()), _logger(Logger::getInstance("resultset1")) {}

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
  InfoSme_T_DBEntityStorage* _dataSource;
  smsc::logger::Logger *_logger;
};

class Insert_into_InfoSme_T  : public DBEntityStorageStatement
{
public:
  Insert_into_InfoSme_T(InfoSme_T_DBEntityStorage* dataSource) : _dataSource(dataSource), _logger(Logger::getInstance("insert1")) {}

  virtual void setUint8(int pos, uint8_t val, bool null=false) throw(SQLException);

  virtual void setString(int pos, const char* str, bool null=false) throw(SQLException);

  virtual void setDateTime(int pos, time_t time, bool null=false) throw(SQLException);

  virtual uint32_t executeUpdate() throw(SQLException);

private:
  uint8_t _state;
  std::string _abonentAddress;
  time_t _sendDate;
  std::string _message;

  //  InfoSme_T_DBEntityStorage* _dataSource;
  InfoSme_T_DBEntityStorage* _dataSource;

  static uint64_t getIdSequenceNumber();
  static uint64_t _idSequenceNumber;
  // DEBUG
  smsc::logger::Logger *_logger;
};

#include "InfoSme_Generating_Tasks_DBEntityStorage.hpp"

class Insert_into_InfoSme_Generating_Tasks : public DBEntityStorageStatement
{
public:
  Insert_into_InfoSme_Generating_Tasks(InfoSme_Generating_Tasks_DBEntityStorage* dataSource) : _dataSource(dataSource),_logger(Logger::getInstance("insert2")) {}

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
  Delete_from_InfoSme_Generating_Tasks_By_TaskId(InfoSme_Generating_Tasks_DBEntityStorage* dataSource) : _dataSource(dataSource) {}

  virtual void setString(int pos, const char* str, bool null=false) throw(SQLException);

  virtual uint32_t executeUpdate() throw(SQLException);
private:
  std::string _taskId;
  InfoSme_Generating_Tasks_DBEntityStorage* _dataSource;
};

class Delete_from_InfoSme_T_By_Id : public DBEntityStorageStatement
{
public:
  Delete_from_InfoSme_T_By_Id(InfoSme_T_DBEntityStorage* dataSource) : _dataSource(dataSource), _logger(Logger::getInstance("delete1")) {}

  virtual void setUint64(int pos, uint64_t val, bool null=false) throw(SQLException);

  virtual uint32_t executeUpdate() throw(SQLException);
private:
  uint64_t _msgId;
  InfoSme_T_DBEntityStorage* _dataSource;
  smsc::logger::Logger *_logger;
};

class Delete_from_InfoSme_T_By_State : public DBEntityStorageStatement
{
public:
  Delete_from_InfoSme_T_By_State(InfoSme_T_DBEntityStorage* dataSource) : _dataSource(dataSource), _iterator(dataSource->getIterator()), _logger(Logger::getInstance("delete2")) {}

  ~Delete_from_InfoSme_T_By_State() { delete _iterator; }

  virtual void setUint8(int pos, uint8_t val, bool null=false) throw(SQLException);

  virtual uint32_t executeUpdate() throw(SQLException);
private:
  uint64_t _msgState;
  InfoSme_T_DBEntityStorage* _dataSource;
  DbIterator<InfoSme_T_Entity>* _iterator;
  smsc::logger::Logger *_logger;
};

class Delete_from_InfoSme_T_By_StateAndAbonent : public DBEntityStorageStatement
{
public:
  Delete_from_InfoSme_T_By_StateAndAbonent(InfoSme_T_DBEntityStorage* dataSource) : _dataSource(dataSource), _iterator(dataSource->getIterator()), _logger(Logger::getInstance("delete3")) {}

  ~Delete_from_InfoSme_T_By_StateAndAbonent() { delete _iterator; }

  virtual void setUint8(int pos, uint8_t val, bool null=false) throw(SQLException);
  virtual void setString(int pos,  const char* val, bool null=false) throw(SQLException);

  virtual uint32_t executeUpdate() throw(SQLException);
private:
  uint64_t _msgState;
  std::string _abonentAddress;
  InfoSme_T_DBEntityStorage* _dataSource;
  DbIterator<InfoSme_T_Entity>* _iterator;
  smsc::logger::Logger *_logger;
};

class Update_InfoSme_T_Set_NewState_By_OldState : public DBEntityStorageStatement
{
public:
  Update_InfoSme_T_Set_NewState_By_OldState(InfoSme_T_DBEntityStorage* dataSource) : _dataSource(dataSource), _iterator(dataSource->getIterator()), _logger(Logger::getInstance("update1")) {}

  ~Update_InfoSme_T_Set_NewState_By_OldState() { delete _iterator; }

  virtual void setUint8(int pos, uint8_t val, bool null=false) throw(SQLException);

  virtual uint32_t executeUpdate() throw(SQLException);
private:
  uint64_t _newMsgState, _searchState;
  InfoSme_T_DBEntityStorage* _dataSource;
  DbIterator<InfoSme_T_Entity>* _iterator;
  smsc::logger::Logger *_logger;
};

class Update_InfoSme_T_Set_State_By_Id : public DBEntityStorageStatement
{
public:
  Update_InfoSme_T_Set_State_By_Id(InfoSme_T_DBEntityStorage* dataSource) : _dataSource(dataSource), _logger(Logger::getInstance("update2")) {}
  virtual void setUint8(int pos, uint8_t val, bool null=false) throw(SQLException);
  virtual void setUint64(int pos, uint64_t val, bool null=false) throw(SQLException);

  virtual uint32_t executeUpdate() throw(SQLException);
private:
  uint8_t _newMsgState;
  uint64_t _msgId;
  InfoSme_T_DBEntityStorage* _dataSource;
  smsc::logger::Logger *_logger;
};

class Update_InfoSme_T_Set_State_By_IdAndState : public DBEntityStorageStatement
{
public:
  Update_InfoSme_T_Set_State_By_IdAndState(InfoSme_T_DBEntityStorage* dataSource) : _dataSource(dataSource), _logger(Logger::getInstance("update3")) {}
  virtual void setUint8(int pos, uint8_t val, bool null=false) throw(SQLException);
  virtual void setUint64(int pos, uint64_t val, bool null=false) throw(SQLException);
  virtual uint32_t executeUpdate() throw(SQLException);
private:
  uint8_t _newMsgState, _oldMsgState;
  uint64_t _msgId;
  InfoSme_T_DBEntityStorage* _dataSource;
  smsc::logger::Logger *_logger;
};

class Update_InfoSme_T_Set_StateAndSendDate_By_Id : public DBEntityStorageStatement
{
public:
  Update_InfoSme_T_Set_StateAndSendDate_By_Id(InfoSme_T_DBEntityStorage* dataSource) : _dataSource(dataSource), _logger(Logger::getInstance("update4")) {}

  virtual void setUint64(int pos, uint64_t val, bool null=false) throw(SQLException);

  virtual void setUint8(int pos, uint8_t val, bool null=false) throw(SQLException);
  virtual void setDateTime(int pos, time_t val, bool null=false)
    throw(SQLException);
  virtual uint32_t executeUpdate() throw(SQLException);
private:
  uint8_t _newMsgState;
  uint64_t _msgId;
  time_t _dateTime;
  InfoSme_T_DBEntityStorage* _dataSource;
  smsc::logger::Logger *_logger;
};

class Insert_into_InfoSme_Id_Mapping : public DBEntityStorageStatement
{
public:
  Insert_into_InfoSme_Id_Mapping(InfoSme_Id_Mapping_DBEntityStorage* dataSource) : _dataSource(dataSource) {}

  virtual void setUint64(int pos, uint64_t val, bool null=false) throw(SQLException);

  virtual void setString(int pos, const char* val, bool null=false) throw(SQLException);

  virtual uint32_t executeUpdate() throw(SQLException);

private:
  uint64_t _msgId;
  std::string _taskId, _smscId;
  
  InfoSme_Id_Mapping_DBEntityStorage* _dataSource;
};

class Delete_from_InfoSme_Id_Mapping_By_SmscId : public DBEntityStorageStatement
{
public:
  Delete_from_InfoSme_Id_Mapping_By_SmscId(InfoSme_Id_Mapping_DBEntityStorage* dataSource) : _dataSource(dataSource), _iterator(dataSource->getIterator()) {}

  ~Delete_from_InfoSme_Id_Mapping_By_SmscId() { delete _iterator; }

  virtual void setString(int pos, const char* val, bool null=false) throw(SQLException);

  virtual uint32_t executeUpdate() throw(SQLException);

private:
  std::string _smscId;
  
  InfoSme_Id_Mapping_DBEntityStorage* _dataSource;
  DbIterator<InfoSme_Id_Mapping_Entity>* _iterator;
};

#include "InfoSme_Tasks_Stat_DBEntityStorage.hpp"

class Delete_from_InfoSme_Tasks_Stat_By_TaskId : public DBEntityStorageStatement
{
public:
  Delete_from_InfoSme_Tasks_Stat_By_TaskId(InfoSme_Tasks_Stat_DBEntityStorage* dataSource) : _dataSource(dataSource) {}

  virtual void setString(int pos, const char* val, bool null=false) throw(SQLException);
  virtual uint32_t executeUpdate() throw(SQLException);

private:
  std::string _taskId;

  InfoSme_Tasks_Stat_DBEntityStorage* _dataSource;
};

class Insert_into_InfoSme_Tasks_Stat : public DBEntityStorageStatement
{
public:
  Insert_into_InfoSme_Tasks_Stat(InfoSme_Tasks_Stat_DBEntityStorage* dataSource) : _dataSource(dataSource) {}

  virtual void setUint32(int pos, uint32_t val, bool null=false) throw(SQLException);
  virtual void setString(int pos, const char* val, bool null=false) throw(SQLException);

  virtual uint32_t executeUpdate() throw(SQLException);

private:
  uint32_t _period, _generated, _delivered, _retried, _failed;
  std::string _taskId;
  
  InfoSme_Tasks_Stat_DBEntityStorage* _dataSource;
};

class SelectInfoSme_TasksStat_fullTableScan : public DBEntityStorageStatement
{
public:
  SelectInfoSme_TasksStat_fullTableScan(InfoSme_Tasks_Stat_DBEntityStorage* dataSource)
    : _dataSource(dataSource),  _logger(Logger::getInstance("select2")) {}

  virtual ResultSet* executeQuery() throw(SQLException);

  class InfoSme_Tasks_Stat_ResultSet : public DBEntityStorageResultSet
  {
  public:
    InfoSme_Tasks_Stat_ResultSet(InfoSme_Tasks_Stat_DBEntityStorage* dataSource) : _iterator(dataSource->getIterator()), _logger(Logger::getInstance("resultset1")) {}

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
