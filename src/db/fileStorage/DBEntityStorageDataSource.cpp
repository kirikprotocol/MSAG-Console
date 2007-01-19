#include "DBEntityStorageDataSource.hpp"
#include "ApplicationStatements.hpp"

extern "C"
DataSourceFactory*  getDataSourceFactory(void)
{
  using namespace smsc::db;

  static DBEntityStorageDataSourceFactory _dbEntityStorageDataSourceFactory;
  return &_dbEntityStorageDataSourceFactory;
}

static std::string
extractTableName(const char* sql, const char* keyword)
{
  const char* keywrdPtr = strstr(sql, keyword);
  if ( keywrdPtr ) {
    const char* searchPtr = keywrdPtr + strlen(keyword);
    while (*searchPtr && ( *searchPtr == 32 || *searchPtr == '\t' )) ++searchPtr;
    if ( *searchPtr ) {
      const char* endTableNamePtr = strchr(searchPtr,32);
      if (endTableNamePtr)
        return std::string(searchPtr,endTableNamePtr);
      else
        return std::string(searchPtr);
    }
  }
  return "";
}

# include <map>
# include <string>
# include "InfoSme_T_DBEntityStorage.hpp"
# include "InfoSme_Id_Mapping_DBEntityStorage.hpp"
# include "InfoSme_Generating_Tasks_DBEntityStorage.hpp"
# include "InfoSme_Tasks_Stat_DBEntityStorage.hpp"
# include <core/synchronization/Mutex.hpp>

class StorageHelper {
public:
  static InfoSme_T_DBEntityStorage* getInfoSme_T_Storage(const std::string& tableName);
  static void createInfoSme_T_Storage(const std::string& tableName);
  static void deleteInfoSme_T_Storage(const std::string& tableName);

  static InfoSme_Id_Mapping_DBEntityStorage* getInfoSme_Id_Mapping_Storage();

  static InfoSme_Generating_Tasks_DBEntityStorage* getInfoSme_GeneratingTasks_Storage();

  static InfoSme_Tasks_Stat_DBEntityStorage* getInfoSme_Tasks_Stat_Storage();
private:
  typedef std::map<std::string, InfoSme_T_DBEntityStorage*> storageMap_t;
  static storageMap_t _storageMap;
  static smsc::core::synchronization::Mutex _storageMapLock;

  static smsc::logger::Logger *_logger;
};

DataSource* DBEntityStorageDataSourceFactory::createDataSource()
{
  return new DBEntityStorageDataSource();
}

DBEntityStorageConnection::DBEntityStorageConnection() : _logger(Logger::getInstance("dbStrgCon")) {}

DBEntityStorageConnection::~DBEntityStorageConnection() {}

void DBEntityStorageConnection::connect() 
  throw(SQLException)
{
  isConnected = true;
}

void DBEntityStorageConnection::disconnect()
{
  isConnected = false;
}

void DBEntityStorageConnection::commit() 
  throw(SQLException) {
}
void DBEntityStorageConnection::rollback() 
  throw(SQLException) {
}
void DBEntityStorageConnection::abort() 
  throw(SQLException) {
}

bool DBEntityStorageConnection::traceIf(const char* what) 
{
  return true;
}

Statement*
DBEntityStorageConnection::createStatement(const char* sql)
  throw(SQLException)
{
  smsc_log_debug(_logger, "Enter DBEntityStorageConnection::createStatement,sql=[%s]",sql);
  static size_t
    del_infosme_id_map_len = strlen("DELETE FROM INFOSME_ID_MAPPING"),
    sel_from_infosme_id_map_len = strlen("SELECT ID, TASK_ID FROM INFOSME_ID_MAPPING"),
    ins_into_infosme_id_map_len = strlen("INSERT INTO INFOSME_ID_MAPPING"),
    ins_into_infosme_gen_tasks_len = strlen("INSERT INTO INFOSME_GENERATING_TASKS"),
    del_from_infosme_gen_tasks_len = strlen("DELETE FROM INFOSME_GENERATING_TASKS"),
    del_from_infosme_tasks_stat_len = strlen("DELETE FROM INFOSME_TASKS_STAT"),
    ins_into_infosme_tasks_stat_len = strlen("INSERT INTO INFOSME_TASKS_STAT"),
    sel_from_infosme_t_len = strlen("SELECT ID, ABONENT, MESSAGE"),
    create_index_len = strlen("CREATE INDEX"),
    create_table_len = strlen("CREATE TABLE"),
    drop_table_len = strlen("DROP TABLE"),
    delete_keyword_len = strlen("DELETE"),
    insert_into_len = strlen("INSERT INTO"),
    update_keyword_len = strlen("UPDATE"),
    infosme_t_fulltablescan_len = strlen("FULL_TABLE_SCAN FROM"),
    infosme_tasks_stat_fulltablescan_len = strlen("FULL_TABLE_SCAN FROM INFOSME_TASKS_STAT");

  if (!strncmp(sql,"DELETE FROM INFOSME_GENERATING_TASKS",del_from_infosme_gen_tasks_len)) {
    smsc_log_debug(_logger, "DBEntityStorageConnection::createStatement::: return object Delete_from_InfoSme_Generating_Tasks_By_TaskId");

    return new Delete_from_InfoSme_Generating_Tasks_By_TaskId(StorageHelper::getInfoSme_GeneratingTasks_Storage());
  } else if (!strncmp(sql,"INSERT INTO INFOSME_GENERATING_TASKS",ins_into_infosme_gen_tasks_len)) {
    smsc_log_debug(_logger, "DBEntityStorageConnection::createStatement::: return object InfoSme_Generating_Tasks_DBEntityStorage");

    return new Insert_into_InfoSme_Generating_Tasks(StorageHelper::getInfoSme_GeneratingTasks_Storage());
  } else if (!strncmp(sql,"INSERT INTO INFOSME_ID_MAPPING",ins_into_infosme_id_map_len)) {
    smsc_log_debug(_logger, "DBEntityStorageConnection::createStatement::: return object Insert_into_InfoSme_Id_Mapping");

    return new Insert_into_InfoSme_Id_Mapping(StorageHelper::getInfoSme_Id_Mapping_Storage());
  } else if (!strncmp(sql,"SELECT ID, TASK_ID FROM INFOSME_ID_MAPPING",sel_from_infosme_id_map_len)) {
    smsc_log_debug(_logger, "DBEntityStorageConnection::createStatement::: return object SelectInfoSme_Id_Mapping_SmscId_criterion");

    return new SelectInfoSme_Id_Mapping_SmscId_criterion(StorageHelper::getInfoSme_Id_Mapping_Storage());
  } else if (!strncmp(sql,"DELETE FROM INFOSME_ID_MAPPING",del_infosme_id_map_len)) {
    smsc_log_debug(_logger, "DBEntityStorageConnection::createStatement::: return object Delete_from_InfoSme_Id_Mapping_By_SmscId");

    return new Delete_from_InfoSme_Id_Mapping_By_SmscId(StorageHelper::getInfoSme_Id_Mapping_Storage());
  } else if (!strncmp(sql,"DELETE FROM INFOSME_TASKS_STAT",del_from_infosme_tasks_stat_len)) {
    smsc_log_debug(_logger, "DBEntityStorageConnection::createStatement::: return object Delete_from_InfoSme_Tasks_Stat_By_TaskId");

    return new Delete_from_InfoSme_Tasks_Stat_By_TaskId(StorageHelper::getInfoSme_Tasks_Stat_Storage());
  } else if (!strncmp(sql,"INSERT INTO INFOSME_TASKS_STAT",ins_into_infosme_tasks_stat_len)) {
    smsc_log_debug(_logger, "DBEntityStorageConnection::createStatement::: return object Insert_into_InfoSme_Tasks_Stat");

    return new Insert_into_InfoSme_Tasks_Stat(StorageHelper::getInfoSme_Tasks_Stat_Storage());
  } else if ( !strncmp(sql, "FULL_TABLE_SCAN FROM INFOSME_TASKS_STAT", infosme_tasks_stat_fulltablescan_len)) {
    smsc_log_debug(_logger, "DBEntityStorageConnection::createStatement::: return object SelectInfoSme_TasksStat_fullTableScan");
    return new SelectInfoSme_TasksStat_fullTableScan(StorageHelper::getInfoSme_Tasks_Stat_Storage());
  } else if ( !strncmp(sql, "CREATE INDEX", create_index_len) ) {
    smsc_log_debug(_logger, "DBEntityStorageConnection::createStatement::: return object DDLStatement for CREATE INDEX");

    return new DDLStatement();
  } else if ( !strncmp(sql, "CREATE TABLE", create_table_len) ) {
    smsc_log_debug(_logger, "DBEntityStorageConnection::createStatement::: return object DDLStatement for CREATE TABLE");
    StorageHelper::createInfoSme_T_Storage(extractTableName(sql, "TABLE"));

    return new DDLStatement();
  } else if ( !strncmp(sql, "DROP TABLE", drop_table_len) ) {
    smsc_log_debug(_logger, "DBEntityStorageConnection::createStatement::: call StorageHelper::deleteInfoSme_T_Storage, sql=[%s]", sql);
    smsc_log_debug(_logger, "DBEntityStorageConnection::createStatement::: deleteInfoSme_T_Storage argument=[%s]", extractTableName(sql, "TABLE").c_str());
    StorageHelper::deleteInfoSme_T_Storage(extractTableName(sql, "TABLE"));
    smsc_log_debug(_logger, "DBEntityStorageConnection::createStatement::: return object DDLStatement for DROP TABLE");
    return new DDLStatement();
  } else {
    if ( !strncmp(sql, "DELETE", delete_keyword_len) ) {
      InfoSme_T_DBEntityStorage*
        infoSme_T_Storage = StorageHelper::getInfoSme_T_Storage(extractTableName(sql, "FROM"));
      if ( strstr(sql,"WHERE STATE=:NEW AND ABONENT=:ABONENT") ) {
        smsc_log_debug(_logger, "DBEntityStorageConnection::createStatement::: return object Delete_from_InfoSme_T_By_StateAndAbonent");
        return new Delete_from_InfoSme_T_By_StateAndAbonent(infoSme_T_Storage);
      } else if ( strstr(sql, "WHERE STATE=:NEW") ) {
        smsc_log_debug(_logger, "DBEntityStorageConnection::createStatement::: return object Delete_from_InfoSme_T_By_State");
        return new Delete_from_InfoSme_T_By_State(infoSme_T_Storage);
      } else if ( strstr(sql, "WHERE ID=:ID") ) {
        smsc_log_debug(_logger, "DBEntityStorageConnection::createStatement::: return object Delete_from_InfoSme_T_By_Id");
        return new Delete_from_InfoSme_T_By_Id(infoSme_T_Storage);
      }
    } else if ( !strncmp(sql, "INSERT INTO", insert_into_len) ) {
      smsc_log_debug(_logger, "DBEntityStorageConnection::createStatement::: return object Insert_into_InfoSme_T");
      return new Insert_into_InfoSme_T(StorageHelper::getInfoSme_T_Storage(extractTableName(sql, "INTO")));
    } else if ( !strncmp(sql, "UPDATE", update_keyword_len) ) {
      InfoSme_T_DBEntityStorage*
        infoSme_T_Storage = StorageHelper::getInfoSme_T_Storage(extractTableName(sql, "UPDATE"));
      if ( strstr(sql, "SET STATE=:NEW WHERE STATE=:WAIT") ) {
        smsc_log_debug(_logger, "DBEntityStorageConnection::createStatement::: return object Update_InfoSme_T_Set_NewState_By_OldState");
        return new Update_InfoSme_T_Set_NewState_By_OldState(infoSme_T_Storage);
      } else if ( strstr(sql, "STATE=:STATE WHERE ID=:ID") ) {
        smsc_log_debug(_logger, "DBEntityStorageConnection::createStatement::: return object Update_InfoSme_T_Set_State_By_Id");
        return new Update_InfoSme_T_Set_State_By_Id(infoSme_T_Storage);
      } else if ( strstr(sql, "STATE=:NEW, SEND_DATE=:SEND_DATE WHERE ID=:ID") ) {
        smsc_log_debug(_logger, "DBEntityStorageConnection::createStatement::: return object Update_InfoSme_T_Set_StateAndSendDate_By_Id");
        return new Update_InfoSme_T_Set_StateAndSendDate_By_Id(infoSme_T_Storage);
      } else if ( strstr(sql, "STATE=:ENROUTE WHERE ID=:ID AND STATE=:WAIT") ) {
        smsc_log_debug(_logger, "DBEntityStorageConnection::createStatement::: return object Update_InfoSme_T_Set_State_By_IdAndState");
        return new Update_InfoSme_T_Set_State_By_IdAndState(infoSme_T_Storage);
      }
    } else if (!strncmp(sql, "SELECT ID, ABONENT, MESSAGE", sel_from_infosme_t_len)) {
      smsc_log_debug(_logger, "DBEntityStorageConnection::createStatement::: return object SelectInfoSme_T_stateAndSendDate_criterion");
      return new SelectInfoSme_T_stateAndSendDate_criterion(StorageHelper::getInfoSme_T_Storage(extractTableName(sql, "FROM")));
    } else if (traceIf("FULL_TABLE_SCAN FROM") && !strncmp(sql, "FULL_TABLE_SCAN FROM", infosme_t_fulltablescan_len)) {
      return new SelectInfoSme_T_fullTableScan(StorageHelper::getInfoSme_T_Storage(extractTableName(sql, "FROM")));
    }
  }
  throw SQLException("DBEntityStorageConnection::createStatement::: unknown statement");
}

Routine*
DBEntityStorageConnection::createRoutine(const char* call, bool func)
  throw(SQLException)
{
  throw SQLException("DBEntityStorageConnection::createRoutine::: not supported");
}

StorageHelper::storageMap_t StorageHelper::_storageMap;
smsc::core::synchronization::Mutex StorageHelper::_storageMapLock;

InfoSme_T_DBEntityStorage* StorageHelper::getInfoSme_T_Storage(const std::string& tableName)
{
  smsc::logger::Logger* logger = Logger::getInstance("dbStrgHlp");

  smsc::core::synchronization::MutexGuard lockGuard(_storageMapLock);
  smsc_log_debug(logger, "StorageHelper::getInfoSme_T_Storage::: find storage for tableName=[%s]", tableName.c_str());
  storageMap_t::iterator iter = _storageMap.find(tableName);
  if ( iter == _storageMap.end() ) {
    SimpleFileDispatcher<InfoSme_T_Entity_Adapter>* infoSme_T_Storage = new SimpleFileDispatcher<InfoSme_T_Entity_Adapter>(tableName + ".db");
    DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::operation_status_t
      st =infoSme_T_Storage->open();
    if ( st != DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::OPERATION_OK ) {
      smsc_log_error(logger, "StorageHelper::getInfoSme_T_Storage::: storage for tableName=[%s] not found", tableName.c_str());
      throw SQLException("StorageHelper::getInfoSme_T_Storage:: storage doesn't exist");
    }
    std::pair<storageMap_t::iterator,bool> insResult =
      _storageMap.insert(std::make_pair(tableName,
                                        new InfoSme_T_DBEntityStorage(infoSme_T_Storage)
                                        ));
    if ( insResult.second == false ) {
      smsc_log_error(logger, "StorageHelper::getInfoSme_T_Storage::: storage for tableName=[%s] not found", tableName.c_str());
      throw SQLException("StorageHelper::getInfoSme_T_Storage:: table doesn't exist");
    } else
      iter = insResult.first;
  }
  smsc_log_debug(logger, "StorageHelper::getInfoSme_T_Storage::: found storage for tableName=[%s]", tableName.c_str());
  return iter->second;
}

#include "InfoSme_T_Entity_Adapter.hpp"

void StorageHelper::createInfoSme_T_Storage(const std::string& tableName)
{
  smsc::logger::Logger* logger = Logger::getInstance("dbStrgHlp");
  smsc::core::synchronization::MutexGuard lockGuard(_storageMapLock);
  smsc_log_debug(logger, "StorageHelper::createInfoSme_T_Storage::: create storage for tableName=[%s]", tableName.c_str());
  storageMap_t::iterator iter = _storageMap.find(tableName);
  if ( iter == _storageMap.end() ) {
    SimpleFileDispatcher<InfoSme_T_Entity_Adapter>* infoSme_T_Storage = new SimpleFileDispatcher<InfoSme_T_Entity_Adapter>(tableName + ".db");
    DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::operation_status_t
      st =infoSme_T_Storage->open();
    if ( !(st == DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::OPERATION_OK ||
           ( st == DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::NO_SUCH_FILE &&
             infoSme_T_Storage->create() == DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::OPERATION_OK )
           )
         )
      throw SQLException("StorageHelper::createInfoSme_T_Storage:: can't create storage");
    _storageMap.insert(std::make_pair(tableName,
                                      new InfoSme_T_DBEntityStorage(infoSme_T_Storage)
                                      ));
    smsc_log_debug(logger, "StorageHelper::createInfoSme_T_Storage::: storage was created for tableName=[%s]", tableName.c_str());
  } else {
    smsc_log_debug(logger, "StorageHelper::createInfoSme_T_Storage::: storage already exists for tableName=[%s]", tableName.c_str());
    throw SQLException("StorageHelper::createInfoSme_T_Storage:: table already exists");
  }
}

void StorageHelper::deleteInfoSme_T_Storage(const std::string& tableName)
{
  smsc::logger::Logger* logger = Logger::getInstance("dbStrgHlp");
  smsc::core::synchronization::MutexGuard lockGuard(_storageMapLock);
  smsc_log_debug(logger, "StorageHelper::deleteInfoSme_T_Storage::: delete storage for tableName=[%s]", tableName.c_str());
  storageMap_t::iterator iter = _storageMap.find(tableName);
  if ( iter != _storageMap.end() ) {
    SimpleFileDispatcher<InfoSme_T_Entity_Adapter>(tableName + ".db").drop();
    delete iter->second;
    _storageMap.erase(iter);
    smsc_log_debug(logger, "StorageHelper::deleteInfoSme_T_Storage::: storage was deleted for tableName=[%s]", tableName.c_str());
  }
}

#include "InfoSme_Id_Mapping_Entity_Adapter.hpp"

static
SimpleFileDispatcher<InfoSme_Id_Mapping_Entity_Adapter>*
get_infosme_id_mapping_file_dispatcher()
{
  static SimpleFileDispatcher<InfoSme_Id_Mapping_Entity_Adapter>
    storage("INFOSME_ID_MAPPING.db");

  DataStorage_FileDispatcher<InfoSme_Id_Mapping_Entity_Adapter>::operation_status_t
    st =storage.open();
  if ( !(st == DataStorage_FileDispatcher<InfoSme_Id_Mapping_Entity_Adapter>::OPERATION_OK ||
         ( st == DataStorage_FileDispatcher<InfoSme_Id_Mapping_Entity_Adapter>::NO_SUCH_FILE &&
           storage.create() == DataStorage_FileDispatcher<InfoSme_Id_Mapping_Entity_Adapter>::OPERATION_OK )
         )
       )
    throw SQLException("StorageHelper::getInfoSme_Id_Mapping_Storage::: can't create storage");

  return &storage;
}

InfoSme_Id_Mapping_DBEntityStorage*
StorageHelper::getInfoSme_Id_Mapping_Storage()
{
  static InfoSme_Id_Mapping_DBEntityStorage
    infoSme_Id_Mapping_Storage(get_infosme_id_mapping_file_dispatcher());

  return &infoSme_Id_Mapping_Storage;
}

#include "InfoSme_Generating_Tasks_Entity_Adapter.hpp"

static
SimpleFileDispatcher<InfoSme_Generating_Tasks_Entity_Adapter>*
get_infosme_generating_tasks_file_dispatcher()
{
  static SimpleFileDispatcher<InfoSme_Generating_Tasks_Entity_Adapter>
    storage("INFOSME_GENERATING_TASKS.db");

  DataStorage_FileDispatcher<InfoSme_Generating_Tasks_Entity_Adapter>::operation_status_t
    st =storage.open();
  if ( !(st == DataStorage_FileDispatcher<InfoSme_Generating_Tasks_Entity_Adapter>::OPERATION_OK ||
         ( st == DataStorage_FileDispatcher<InfoSme_Generating_Tasks_Entity_Adapter>::NO_SUCH_FILE &&
           storage.create() == DataStorage_FileDispatcher<InfoSme_Generating_Tasks_Entity_Adapter>::OPERATION_OK )
         )
       )
    throw SQLException("StorageHelper::get_infosme_generating_tasks_file_dispatcher::: can't create storage");

  return &storage;
}

InfoSme_Generating_Tasks_DBEntityStorage*
StorageHelper::getInfoSme_GeneratingTasks_Storage()
{
  static InfoSme_Generating_Tasks_DBEntityStorage
    infoSme_GeneratingTasks_storage(get_infosme_generating_tasks_file_dispatcher());
  return &infoSme_GeneratingTasks_storage;
}

static
SimpleFileDispatcher<InfoSme_Tasks_Stat_Entity_Adapter>*
get_infosme_tasks_stat_file_dispatcher()
{
  static SimpleFileDispatcher<InfoSme_Tasks_Stat_Entity_Adapter>
    storage("INFOSME_TASKS_STAT.db");

  DataStorage_FileDispatcher<InfoSme_Tasks_Stat_Entity_Adapter>::operation_status_t
    st =storage.open();
  if ( !(st == DataStorage_FileDispatcher<InfoSme_Tasks_Stat_Entity_Adapter>::OPERATION_OK ||
         ( st == DataStorage_FileDispatcher<InfoSme_Tasks_Stat_Entity_Adapter>::NO_SUCH_FILE &&
           storage.create() == DataStorage_FileDispatcher<InfoSme_Tasks_Stat_Entity_Adapter>::OPERATION_OK )
         )
       )
    throw SQLException("StorageHelper::get_infosme_tasks_stat_file_dispatcher::: can't create storage");

  return &storage;
}

InfoSme_Tasks_Stat_DBEntityStorage*
StorageHelper::getInfoSme_Tasks_Stat_Storage()
{
  static InfoSme_Tasks_Stat_DBEntityStorage
    infoSme_Tasks_Stat_storage(get_infosme_tasks_stat_file_dispatcher());

  return &infoSme_Tasks_Stat_storage;
}

Connection* DBEntityStorageDataSource::getConnection()
{
  return newConnection();
}

void DBEntityStorageDataSource::closeRegisteredQueries(const char* id)
{
  getConnection()->unregisterStatement(id);
  getConnection()->unregisterRoutine(id);
}


DBEntityStorageDriver::DBEntityStorageDriver(ConfigView* config)
  throw (ConfigException) : DBDriver(config), _logger(Logger::getInstance("dbStrgDrv"))
{
  
}

Connection*
DBEntityStorageDriver::newConnection()
{
  static DBEntityStorageConnection _localStorage;
  return &_localStorage;
}
