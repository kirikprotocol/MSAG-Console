#include "StorageHelper.hpp"
#include <db/exceptions/DataSourceExceptions.h>

StorageHelper::storageMap_t StorageHelper::_storageMap;
smsc::core::synchronization::Mutex StorageHelper::_storageMapLock;
std::string StorageHelper::_dbSubDir;

void StorageHelper::setDbFilePrefix(const std::string& dbSubDir)
{
  _dbSubDir = dbSubDir;
}

StorageHelper::InfoSme_T_storage_ref_t StorageHelper::getInfoSme_T_Storage(const std::string& tableName)
{
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("dbStrgHlp");

  smsc::core::synchronization::MutexGuard lockGuard(_storageMapLock);
  smsc_log_debug(logger, "StorageHelper::getInfoSme_T_Storage::: find storage for tableName=[%s]", tableName.c_str());
  storageMap_t::iterator iter = _storageMap.find(tableName);
  if ( iter == _storageMap.end() ) {
    SimpleFileDispatcher<InfoSme_T_Entity_Adapter>* infoSme_T_Storage = new SimpleFileDispatcher<InfoSme_T_Entity_Adapter>(_dbSubDir + tableName + ".db");
    DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::operation_status_t
      st =infoSme_T_Storage->open();
    if ( st != DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::OPERATION_OK ) {
      smsc_log_error(logger, "StorageHelper::getInfoSme_T_Storage::: storage for tableName=[%s] not found", tableName.c_str());
      throw smsc::db::SQLException("StorageHelper::getInfoSme_T_Storage:: storage doesn't exist");
    }
    std::pair<storageMap_t::iterator,bool> insResult =
      _storageMap.insert(std::make_pair(tableName,
                                        InfoSme_T_storage_ref_t(new InfoSme_T_DBEntityStorage(infoSme_T_Storage))
                                        ));
    if ( insResult.second == false ) {
      smsc_log_error(logger, "StorageHelper::getInfoSme_T_Storage::: storage for tableName=[%s] not found", tableName.c_str());
      throw smsc::db::SQLException("StorageHelper::getInfoSme_T_Storage:: table doesn't exist");
    } else
      iter = insResult.first;
  }

  smsc_log_debug(logger, "StorageHelper::getInfoSme_T_Storage::: found storage for tableName=[%s]", tableName.c_str());
  return iter->second;
}

#include "InfoSme_T_Entity_Adapter.hpp"

void StorageHelper::createInfoSme_T_Storage(const std::string& tableName)
{
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("dbStrgHlp");
  smsc::core::synchronization::MutexGuard lockGuard(_storageMapLock);
  smsc_log_debug(logger, "StorageHelper::createInfoSme_T_Storage::: create storage for tableName=[%s]", tableName.c_str());
  storageMap_t::iterator iter = _storageMap.find(tableName);
  if ( iter == _storageMap.end() ) {
    SimpleFileDispatcher<InfoSme_T_Entity_Adapter>* infoSme_T_Storage = new SimpleFileDispatcher<InfoSme_T_Entity_Adapter>(_dbSubDir + tableName + ".db");
    DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::operation_status_t
      st =infoSme_T_Storage->open();
    if ( !(st == DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::OPERATION_OK ||
           ( st == DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::NO_SUCH_FILE &&
             infoSme_T_Storage->create() == DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::OPERATION_OK )
           )
         )
      throw smsc::db::SQLException("StorageHelper::createInfoSme_T_Storage:: can't create storage");
    _storageMap.insert(std::make_pair(tableName,
                                      InfoSme_T_storage_ref_t(new InfoSme_T_DBEntityStorage(infoSme_T_Storage, false))
                                      ));
    smsc_log_debug(logger, "StorageHelper::createInfoSme_T_Storage::: storage was created for tableName=[%s]", tableName.c_str());
  } else {
    smsc_log_debug(logger, "StorageHelper::createInfoSme_T_Storage::: storage already exists for tableName=[%s]", tableName.c_str());
    throw smsc::db::SQLException("StorageHelper::createInfoSme_T_Storage:: table already exists");
  }
}

void StorageHelper::deleteInfoSme_T_Storage(const std::string& tableName, bool needDropDataFile)
{
  smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("dbStrgHlp");
  smsc::core::synchronization::MutexGuard lockGuard(_storageMapLock);

  storageMap_t::iterator iter = _storageMap.find(tableName);
  if ( iter != _storageMap.end() ) {
    smsc_log_debug(logger, "StorageHelper::deleteInfoSme_T_Storage::: delete storage for tableName=[%s]", tableName.c_str());
    if ( needDropDataFile )
      SimpleFileDispatcher<InfoSme_T_Entity_Adapter>(_dbSubDir + tableName + ".db").drop();

    _storageMap.erase(iter);

    smsc_log_debug(logger, "StorageHelper::deleteInfoSme_T_Storage::: storage was deleted for tableName=[%s]", tableName.c_str());

    if ( !needDropDataFile ) {
      SimpleFileDispatcher<InfoSme_T_Entity_Adapter>* infoSme_T_Storage = new SimpleFileDispatcher<InfoSme_T_Entity_Adapter>(_dbSubDir + tableName + ".db");
      DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::operation_status_t
        st =infoSme_T_Storage->open();
      if ( st == DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::OPERATION_OK ) {
        std::pair<storageMap_t::iterator,bool> insResult =
          _storageMap.insert(std::make_pair(tableName,
                                            InfoSme_T_storage_ref_t(new InfoSme_T_DBEntityStorage(infoSme_T_Storage, false))
                                            ));
      }
    }
  }
}

/*
SimpleFileDispatcher<InfoSme_T_Entity_Adapter>* infoSme_T_Storage = new SimpleFileDispatcher<InfoSme_T_Entity_Adapter>(tableName + ".db");
DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::operation_status_t
      st =infoSme_T_Storage->open();
    if ( st != DataStorage_FileDispatcher<InfoSme_T_Entity_Adapter>::OPERATION_OK ) {
      smsc_log_error(logger, "StorageHelper::getInfoSme_T_Storage::: storage for tableName=[%s] not found", tableName.c_str());
      throw smsc::db::SQLException("StorageHelper::getInfoSme_T_Storage:: storage doesn't exist");
    }
    std::pair<storageMap_t::iterator,bool> insResult =
    _storageMap.insert(std::make_pair(tableName,
                                        InfoSme_T_storage_ref_t(new InfoSme_T_DBEntityStorage(infoSme_T_Storage))
                                        ));
 */
#include "InfoSme_Id_Mapping_Entity_Adapter.hpp"

static
SimpleFileDispatcher<InfoSme_Id_Mapping_Entity_Adapter>*
get_infosme_id_mapping_file_dispatcher(const std::string& dbSubDir)
{
  static SimpleFileDispatcher<InfoSme_Id_Mapping_Entity_Adapter>
    storage(dbSubDir + "INFOSME_ID_MAPPING.db");

  DataStorage_FileDispatcher<InfoSme_Id_Mapping_Entity_Adapter>::operation_status_t
    st =storage.open();
  if ( st == DataStorage_FileDispatcher<InfoSme_Id_Mapping_Entity_Adapter>::OPERATION_OK )
    storage.drop();

  st = storage.create();
  if (st != DataStorage_FileDispatcher<InfoSme_Id_Mapping_Entity_Adapter>::OPERATION_OK )
    throw smsc::db::SQLException("StorageHelper::getInfoSme_Id_Mapping_Storage::: can't create storage");

  return &storage;
}

InfoSme_Id_Mapping_DBEntityStorage*
StorageHelper::getInfoSme_Id_Mapping_Storage()
{
  static InfoSme_Id_Mapping_DBEntityStorage
    infoSme_Id_Mapping_Storage(get_infosme_id_mapping_file_dispatcher(_dbSubDir));

  return &infoSme_Id_Mapping_Storage;
}

#include "InfoSme_Generating_Tasks_Entity_Adapter.hpp"

static
SimpleFileDispatcher<InfoSme_Generating_Tasks_Entity_Adapter>*
get_infosme_generating_tasks_file_dispatcher(const std::string& dbSubDir)
{
  static SimpleFileDispatcher<InfoSme_Generating_Tasks_Entity_Adapter>
    storage(dbSubDir + "INFOSME_GENERATING_TASKS.db");

  DataStorage_FileDispatcher<InfoSme_Generating_Tasks_Entity_Adapter>::operation_status_t
    st =storage.open();
  if ( !(st == DataStorage_FileDispatcher<InfoSme_Generating_Tasks_Entity_Adapter>::OPERATION_OK ||
         ( st == DataStorage_FileDispatcher<InfoSme_Generating_Tasks_Entity_Adapter>::NO_SUCH_FILE &&
           storage.create() == DataStorage_FileDispatcher<InfoSme_Generating_Tasks_Entity_Adapter>::OPERATION_OK )
         )
       )
    throw smsc::db::SQLException("StorageHelper::get_infosme_generating_tasks_file_dispatcher::: can't create storage");

  return &storage;
}

InfoSme_Generating_Tasks_DBEntityStorage*
StorageHelper::getInfoSme_GeneratingTasks_Storage()
{
  static InfoSme_Generating_Tasks_DBEntityStorage
    infoSme_GeneratingTasks_storage(get_infosme_generating_tasks_file_dispatcher(_dbSubDir));
  return &infoSme_GeneratingTasks_storage;
}

static
SimpleFileDispatcher<InfoSme_Tasks_Stat_Entity_Adapter>*
get_infosme_tasks_stat_file_dispatcher(const std::string& dbSubDir)
{
  static SimpleFileDispatcher<InfoSme_Tasks_Stat_Entity_Adapter>
    storage(dbSubDir + "INFOSME_TASKS_STAT.db");

  DataStorage_FileDispatcher<InfoSme_Tasks_Stat_Entity_Adapter>::operation_status_t
    st =storage.open();
  if ( !(st == DataStorage_FileDispatcher<InfoSme_Tasks_Stat_Entity_Adapter>::OPERATION_OK ||
         ( st == DataStorage_FileDispatcher<InfoSme_Tasks_Stat_Entity_Adapter>::NO_SUCH_FILE &&
           storage.create() == DataStorage_FileDispatcher<InfoSme_Tasks_Stat_Entity_Adapter>::OPERATION_OK )
         )
       )
    throw smsc::db::SQLException("StorageHelper::get_infosme_tasks_stat_file_dispatcher::: can't create storage");

  return &storage;
}

InfoSme_Tasks_Stat_DBEntityStorage*
StorageHelper::getInfoSme_Tasks_Stat_Storage()
{
  static InfoSme_Tasks_Stat_DBEntityStorage
    infoSme_Tasks_Stat_storage(get_infosme_tasks_stat_file_dispatcher(_dbSubDir));

  return &infoSme_Tasks_Stat_storage;
}
