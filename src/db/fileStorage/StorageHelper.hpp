#ifndef __DBENTITYSTORAGE_STORAGEHELPER_HPP__
# define __DBENTITYSTORAGE_STORAGEHELPER_HPP__ 1

# include <string>
# include <map>

# include <logger/Logger.h>
# include <core/synchronization/Mutex.hpp>
# include <core/buffers/RefPtr.hpp>

# include "InfoSme_T_DBEntityStorage.hpp"
# include "InfoSme_Id_Mapping_DBEntityStorage.hpp"
# include "InfoSme_Generating_Tasks_DBEntityStorage.hpp"
# include "InfoSme_Tasks_Stat_DBEntityStorage.hpp"

class StorageHelper {
public:
  typedef smsc::core::buffers::RefPtr<InfoSme_T_DBEntityStorage,
                                      smsc::core::synchronization::Mutex> InfoSme_T_storage_ref_t;

  static void setDbFilePrefix(const std::string& dbSubDir);
  static InfoSme_T_storage_ref_t getInfoSme_T_Storage(const std::string& tableName);
  static void createInfoSme_T_Storage(const std::string& tableName);
  static void deleteInfoSme_T_Storage(const std::string& tableName, bool needDropDataFile=true);

  static InfoSme_Id_Mapping_DBEntityStorage* getInfoSme_Id_Mapping_Storage();

  static InfoSme_Generating_Tasks_DBEntityStorage* getInfoSme_GeneratingTasks_Storage();

  static InfoSme_Tasks_Stat_DBEntityStorage* getInfoSme_Tasks_Stat_Storage();
private:
  typedef std::map<std::string, InfoSme_T_storage_ref_t> storageMap_t;
  static storageMap_t _storageMap;
  static smsc::core::synchronization::Mutex _storageMapLock;

  static smsc::logger::Logger *_logger;
  static std::string _dbSubDir;
};

#endif
