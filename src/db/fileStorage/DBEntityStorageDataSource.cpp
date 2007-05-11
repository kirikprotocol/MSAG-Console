#include "DBEntityStorageDataSource.hpp"
#include "ApplicationStatements.hpp"
#include "SequenceNumber.hpp"
#include <sys/types.h>

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
# include "StorageHelper.hpp"


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
  try {
    smsc_log_debug(_logger, "DBEntityStorageConnection::createStatement sql=[%s]",sql);
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
      infosme_tasks_stat_fulltablescan_len = strlen("FULL_TABLE_SCAN FROM INFOSME_TASKS_STAT"),
      drop_infosme_t_index_len = strlen("DROP INDEX ON"),
      infosme_t_fulltablescan_wo_idx_len = strlen("FULL_TABLE_SCAN_WO_INDEX FROM");

    if (!strncmp(sql,"DELETE FROM INFOSME_GENERATING_TASKS",del_from_infosme_gen_tasks_len)) {
      smsc_log_info(_logger, "DBEntityStorageConnection::createStatement::: return object Delete_from_InfoSme_Generating_Tasks_By_TaskId");

      return new Delete_from_InfoSme_Generating_Tasks_By_TaskId(StorageHelper::getInfoSme_GeneratingTasks_Storage());
    } else if (!strncmp(sql,"INSERT INTO INFOSME_GENERATING_TASKS",ins_into_infosme_gen_tasks_len)) {
      smsc_log_info(_logger, "DBEntityStorageConnection::createStatement::: return object InfoSme_Generating_Tasks_DBEntityStorage");

      return new Insert_into_InfoSme_Generating_Tasks(StorageHelper::getInfoSme_GeneratingTasks_Storage());
    } else if (!strncmp(sql,"INSERT INTO INFOSME_ID_MAPPING",ins_into_infosme_id_map_len)) {
      smsc_log_info(_logger, "DBEntityStorageConnection::createStatement::: return object Insert_into_InfoSme_Id_Mapping");

      return new Insert_into_InfoSme_Id_Mapping(StorageHelper::getInfoSme_Id_Mapping_Storage());
    } else if (!strncmp(sql,"SELECT ID, TASK_ID FROM INFOSME_ID_MAPPING",sel_from_infosme_id_map_len)) {
      smsc_log_info(_logger, "DBEntityStorageConnection::createStatement::: return object SelectInfoSme_Id_Mapping_SmscId_criterion");

      return new SelectInfoSme_Id_Mapping_SmscId_criterion(StorageHelper::getInfoSme_Id_Mapping_Storage());
    } else if (!strncmp(sql,"DELETE FROM INFOSME_ID_MAPPING",del_infosme_id_map_len)) {
      if (strstr(sql, "WHERE SMSC_ID=:SMSC_ID")) {
        smsc_log_info(_logger, "DBEntityStorageConnection::createStatement::: return object Delete_from_InfoSme_Id_Mapping_By_SmscId");

        return new Delete_from_InfoSme_Id_Mapping_By_SmscId(StorageHelper::getInfoSme_Id_Mapping_Storage());
      } else {
        smsc_log_info(_logger, "DBEntityStorageConnection::createStatement::: return object Delete_from_InfoSme_Id_Mapping_By_Id");

        return new Delete_from_InfoSme_Id_Mapping_By_Id(StorageHelper::getInfoSme_Id_Mapping_Storage());
      }
    } else if (!strncmp(sql,"DELETE FROM INFOSME_TASKS_STAT",del_from_infosme_tasks_stat_len)) {
      smsc_log_info(_logger, "DBEntityStorageConnection::createStatement::: return object Delete_from_InfoSme_Tasks_Stat_By_TaskId");

      return new Delete_from_InfoSme_Tasks_Stat_By_TaskId(StorageHelper::getInfoSme_Tasks_Stat_Storage());
    } else if (!strncmp(sql,"INSERT INTO INFOSME_TASKS_STAT",ins_into_infosme_tasks_stat_len)) {
      smsc_log_info(_logger, "DBEntityStorageConnection::createStatement::: return object Insert_into_InfoSme_Tasks_Stat");

      return new Insert_into_InfoSme_Tasks_Stat(StorageHelper::getInfoSme_Tasks_Stat_Storage());
    } else if ( !strncmp(sql, "FULL_TABLE_SCAN FROM INFOSME_TASKS_STAT", infosme_tasks_stat_fulltablescan_len)) {
      smsc_log_info(_logger, "DBEntityStorageConnection::createStatement::: return object SelectInfoSme_TasksStat_fullTableScan");
      return new SelectInfoSme_TasksStat_fullTableScan(StorageHelper::getInfoSme_Tasks_Stat_Storage());
    } else if ( !strncmp(sql, "CREATE INDEX", create_index_len) ) {
      smsc_log_info(_logger, "DBEntityStorageConnection::createStatement::: return object DDLStatement for CREATE INDEX");

      return new DDLStatement();
    } else if ( !strncmp(sql, "CREATE TABLE", create_table_len) ) {
      smsc_log_info(_logger, "DBEntityStorageConnection::createStatement::: return object DDLStatement for CREATE TABLE");
      StorageHelper::createInfoSme_T_Storage(extractTableName(sql, "TABLE"));
      
      return new DDLStatement();
    } else if ( !strncmp(sql, "DROP TABLE", drop_table_len) ) {
      smsc_log_info(_logger, "DBEntityStorageConnection::createStatement::: deleteInfoSme_T_Storage argument=[%s]", extractTableName(sql, "TABLE").c_str());
      StorageHelper::deleteInfoSme_T_Storage(extractTableName(sql, "TABLE"));
      return new DDLStatement();
    } else if ( !strncmp(sql, "DROP INDEX ON", drop_infosme_t_index_len) ) {
      smsc_log_info(_logger, "DBEntityStorageConnection::createStatement::: deleteInfoSme_T_Index_in_mem argument=[%s]", extractTableName(sql, "TABLE").c_str());

      StorageHelper::deleteInfoSme_T_Storage(extractTableName(sql, "ON"), false /*bool needDropDataFile*/);
      return new DDLStatement();
    } else {
      if ( !strncmp(sql, "DELETE", delete_keyword_len) ) {
        StorageHelper::InfoSme_T_storage_ref_t
          infoSme_T_Storage = StorageHelper::getInfoSme_T_Storage(extractTableName(sql, "FROM"));
        if ( strstr(sql,"WHERE STATE=:NEW AND ABONENT=:ABONENT") ) {
          smsc_log_info(_logger, "DBEntityStorageConnection::createStatement::: return object Delete_from_InfoSme_T_By_StateAndAbonent");
          return new Delete_from_InfoSme_T_By_StateAndAbonent(infoSme_T_Storage);
        } else if ( strstr(sql, "WHERE STATE=:NEW") ) {
          smsc_log_info(_logger, "DBEntityStorageConnection::createStatement::: return object Delete_from_InfoSme_T_By_State");
          return new Delete_from_InfoSme_T_By_State(infoSme_T_Storage);
        } else if ( strstr(sql, "WHERE ID=:ID") ) {
          smsc_log_info(_logger, "DBEntityStorageConnection::createStatement::: return object Delete_from_InfoSme_T_By_Id");
          return new Delete_from_InfoSme_T_By_Id(infoSme_T_Storage);
        }
      } else if ( !strncmp(sql, "INSERT INTO", insert_into_len) ) {
        smsc_log_info(_logger, "DBEntityStorageConnection::createStatement::: return object Insert_into_InfoSme_T");
        return new Insert_into_InfoSme_T(StorageHelper::getInfoSme_T_Storage(extractTableName(sql, "INTO")));
      } else if ( !strncmp(sql, "UPDATE", update_keyword_len) ) {
        StorageHelper::InfoSme_T_storage_ref_t
          infoSme_T_Storage = StorageHelper::getInfoSme_T_Storage(extractTableName(sql, "UPDATE"));
        if ( strstr(sql, "SET STATE=:NEW WHERE STATE=:WAIT") ) {
          smsc_log_info(_logger, "DBEntityStorageConnection::createStatement::: return object Update_InfoSme_T_Set_NewState_By_OldState");
          return new Update_InfoSme_T_Set_NewState_By_OldState(infoSme_T_Storage);
        } else if ( strstr(sql, "STATE=:STATE WHERE ID=:ID") ) {
          smsc_log_info(_logger, "DBEntityStorageConnection::createStatement::: return object Update_InfoSme_T_Set_State_By_Id");
          return new Update_InfoSme_T_Set_State_By_Id(infoSme_T_Storage);
        } else if ( strstr(sql, "STATE=:NEW, SEND_DATE=:SEND_DATE WHERE ID=:ID") ) {
          smsc_log_info(_logger, "DBEntityStorageConnection::createStatement::: return object Update_InfoSme_T_Set_StateAndSendDate_By_Id");
          return new Update_InfoSme_T_Set_StateAndSendDate_By_Id(infoSme_T_Storage);
        } else if ( strstr(sql, "STATE=:ENROUTE WHERE ID=:ID AND STATE=:WAIT") ) {
          smsc_log_info(_logger, "DBEntityStorageConnection::createStatement::: return object Update_InfoSme_T_Set_State_By_IdAndState");
          return new Update_InfoSme_T_Set_State_By_IdAndState(infoSme_T_Storage);
        } else if ( strstr(sql, "SET MESSAGE = :MSG WHERE ID = :ID") ) {
          smsc_log_info(_logger, "DBEntityStorageConnection::createStatement::: return object Update_InfoSme_T_Set_NewMessage_By_Id");
          return new Update_InfoSme_T_Set_NewMessage_By_Id(infoSme_T_Storage);
        }
      } else if (!strncmp(sql, "SELECT ID, ABONENT, MESSAGE", sel_from_infosme_t_len)) {
        smsc_log_info(_logger, "DBEntityStorageConnection::createStatement::: return object SelectInfoSme_T_stateAndSendDate_criterion");
        return new SelectInfoSme_T_stateAndSendDate_criterion(StorageHelper::getInfoSme_T_Storage(extractTableName(sql, "FROM")));
      } else if (!strncmp(sql, "FULL_TABLE_SCAN FROM", infosme_t_fulltablescan_len)) {
        smsc_log_info(_logger, "DBEntityStorageConnection::createStatement::: return object FullTableScan_SelectInfoSme_T_stateAndSendDate_criterion");
        return new SelectInfoSme_T_fullTableScan(StorageHelper::getInfoSme_T_Storage(extractTableName(sql, "FROM")));
      }
    }
  } catch (std::exception& ex) {
    smsc_log_debug(_logger, "DBEntityStorageConnection::createStatement::: catch std::exception [%s]", ex.what());
    throw SQLException(ex.what());
  }

  throw SQLException("DBEntityStorageConnection::createStatement::: unknown statement");
}

Routine*
DBEntityStorageConnection::createRoutine(const char* call, bool func)
  throw(SQLException)
{
  throw SQLException("DBEntityStorageConnection::createRoutine::: not supported");
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
  std::string dbSubDir("./");
  try {
    dbSubDir += config->getString("dbSubDir");
  } catch (...) {}

  if ( dbSubDir[dbSubDir.size()-1] != '/' )
    dbSubDir += '/';

  if ( ::mkdir(dbSubDir.c_str(), S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP) < 0 ) {
    if ( errno != EEXIST ) {
      std::ostringstream obuf("DBEntityStorageDriver::DBEntityStorageDriver::: can't create directory ");
      obuf << "[" << dbSubDir.c_str() 
           << "]. " << strerror(errno);
      smsc_log_debug(_logger, obuf.str().c_str());
      throw ConfigException(obuf.str().c_str());
    }
  }
  SequenceNumber::getInstance().initialize(dbSubDir + "seqNum.db");

  try { 
    // make static storage initialization 
    StorageHelper::setDbFilePrefix(dbSubDir);
    InfoSme_Id_Mapping_DBEntityStorage* _init_InfoSme_Id_Mapping_Storage =
      StorageHelper::getInfoSme_Id_Mapping_Storage();

    InfoSme_Generating_Tasks_DBEntityStorage* _init_InfoSme_GeneratingTasks_Storage =
      StorageHelper::getInfoSme_GeneratingTasks_Storage();

    InfoSme_Tasks_Stat_DBEntityStorage* _init_InfoSme_Tasks_Stat_Storage = 
      StorageHelper::getInfoSme_Tasks_Stat_Storage();
  } catch (std::exception& ex) {
    smsc_log_debug(_logger, "DBEntityStorageDriver::DBEntityStorageDriver::: catch exception=[%s]", ex.what());
    throw ConfigException(ex.what());
  }

}

Connection*
DBEntityStorageDriver::newConnection()
{
  static DBEntityStorageConnection _localStorage;
  return &_localStorage;
}

extern "C"
DataSourceFactory*  getDataSourceFactory(void)
{
  //  using namespace smsc::db;
  static DBEntityStorageDataSourceFactory _dbEntityStorageDataSourceFactory;

  return &_dbEntityStorageDataSourceFactory;
}
