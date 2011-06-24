#include <dirent.h>
#include <unistd.h>
#include <errno.h>  
#include <vector>

#include "db/DataSource.h"
#include "db/DataSourceLoader.h"
#include "db/exceptions/DataSourceExceptions.h"
#include "logger/Logger.h"
#include "util/config/Manager.h"
#include "util/config/ConfigView.h"
#include "util/config/ConfigException.h"
#include "util/Exception.hpp"
#include "core/buffers/File.hpp"
#include "core/buffers/TmpBuf.hpp"

using smsc::util::config::Manager;
using smsc::util::config::ConfigView;
using smsc::util::config::ConfigException;
using smsc::util::Exception;
using smsc::db::DataSourceLoader;
using smsc::db::DataSource;
using smsc::db::DataSourceFactory;
using smsc::db::Connection;
using smsc::db::Statement;
using smsc::db::ResultSet;
using smsc::db::SQLException;
using smsc::core::buffers::File;
using smsc::core::buffers::TmpBuf;

DataSource* createDataSource(ConfigView* config, smsc::logger::Logger * logger);
void convert(Connection* connection, smsc::logger::Logger* logger, const std::string& loc);
void tmByPeriod(tm& tmFlushing, uint32_t period);

const char * SMSC_MCISME_STAT_DIR_NAME_FORMAT = "%04d-%02d";
const char * SMSC_MCISME_STAT_FILE_NAME_FORMAT = "%02d.rts";
const char * SMSC_MCISME_STAT_HEADER_TEXT = "SMSC.MCISME.STAT";
const uint16_t SMSC_MCISME_STAT_VERSION_INFO = 0x0001;
const uint16_t SMSC_MCISME_STAT_DUMP_INTERVAL = 3600;

class StatisticsWriter
{
protected:
  std::string     location;
  bool            bFileTM;
  tm              fileTM;
  File file;
  smsc::logger::Logger * logger;

  bool createStorageDir(const std::string loc);
  void dumpCounters(const uint8_t* buff, int buffLen, const tm& flushTM);
  bool createDir(const std::string& dir);

public:

  void  flushCounters(const tm& tmFlushTM, uint32_t missed, uint32_t delivered, uint32_t failed, uint32_t notified);
  void init(const std::string& loc);
  StatisticsWriter();
  virtual ~StatisticsWriter();
};

void StatisticsWriter::flushCounters(const tm& tmFlushTM, uint32_t missed, uint32_t delivered, uint32_t failed, uint32_t notified)
{    
  TmpBuf<uint8_t, 128> buff(128);

  // Head of record
  uint8_t value8 = 0;
  value8 = (uint8_t)(tmFlushTM.tm_hour); buff.Append((uint8_t *)&value8, sizeof(value8));

  // Counters
  int32_t value32 = 0;
  value32 = htonl(missed);         buff.Append((uint8_t *)&value32, sizeof(value32));
  value32 = htonl(delivered);      buff.Append((uint8_t *)&value32, sizeof(value32));
  value32 = htonl(failed);         buff.Append((uint8_t *)&value32, sizeof(value32));
  value32 = htonl(notified);       buff.Append((uint8_t *)&value32, sizeof(value32));

  dumpCounters(buff, buff.GetPos(), tmFlushTM);    

}

bool StatisticsWriter::createStorageDir(const std::string loc)
{
  const char * dir_ = loc.c_str();

  unsigned len = strlen(dir_);
  if(len == 0)
    return false;

  if(strcmp(dir_, "/") == 0)
    return true;

  ++len;

  TmpBuf<char, 512> tmpBuff(len);
  char * buff =tmpBuff.get();
  memcpy(buff, dir_, len);

  if(buff[len-2] == '/'){
    buff[len-2] = 0;
    if(len > 2){
      if(buff[len-3] == '/'){
        return false;
      }
    }
  }

  std::vector<std::string> dirs(0);

  char* p1 = buff+1;
  int dirlen = 0;
  char* p2 = strchr(p1, '/');
  while(p2){
    int len = p2 - p1;
    dirlen += len + 1;
    if(len == 0)
      return false;

    int direclen = dirlen + 1;
    TmpBuf<char, 512> tmpBuff(direclen);
    char * dir = tmpBuff.get();
    memcpy(dir, buff, dirlen);
    dir[dirlen] = 0;
    dirs.push_back(std::string(dir));

    p1 = p1 + len + 1;
    p2 = strchr(p1, '/');
  }
  dirs.push_back(std::string(buff));

  for(std::vector<std::string>::iterator it = dirs.begin(); it != dirs.end(); it++){

    DIR* dirp = opendir( (*it).c_str() );
    if(dirp){
      closedir(dirp);            
    }else{
      try{
        createDir(std::string( (*it).c_str() ));
      }catch(...){
        return false;
      }
    }
  }

  return true;
}

void StatisticsWriter::dumpCounters(const uint8_t* buff, int buffLen, const tm& flushTM)
{
  //smsc_log_debug(logger, "Statistics dump called for %02d:%02d GMT", 
  //               flushTM.tm_hour, flushTM.tm_min);

  try {

    char dirName[128]; bool hasDir = false;

    if (!bFileTM || fileTM.tm_mon != flushTM.tm_mon || fileTM.tm_year != flushTM.tm_year)
    {
      sprintf(dirName, SMSC_MCISME_STAT_DIR_NAME_FORMAT, flushTM.tm_year+1900, flushTM.tm_mon+1);
      std::string loc = location + "/" + dirName;
      //smsc_log_debug(logger, "New dir '%s' created", loc.c_str());
      createDir(location + "/" + dirName); bFileTM = false; hasDir = true;
      smsc_log_debug(logger, "New dir '%s' created", dirName);
    }

    if (!bFileTM || fileTM.tm_mday != flushTM.tm_mday)
    {
      char fileName[128]; 
      std::string fullPath = location;
      if (!hasDir) sprintf(dirName, SMSC_MCISME_STAT_DIR_NAME_FORMAT, flushTM.tm_year+1900, flushTM.tm_mon+1);
      sprintf(fileName, SMSC_MCISME_STAT_FILE_NAME_FORMAT, flushTM.tm_mday);
      fullPath += '/'; fullPath += (const char*)dirName; 
      fullPath += '/'; fullPath += (const char*)fileName; 
      const char* fullPathStr = fullPath.c_str();

      if (file.isOpened()) file.Close();

      bool needHeader = true;
      if (File::Exists(fullPathStr)) { 
        needHeader = false;
        file.WOpen(fullPathStr);
      } else {
        file.RWCreate(fullPathStr);
        smsc_log_debug(logger, "New file '%s' created", fullPathStr);
      }

      if (needHeader) { // create header (if new file created)
        file.Write(SMSC_MCISME_STAT_HEADER_TEXT, strlen(SMSC_MCISME_STAT_HEADER_TEXT));
        uint16_t version = htons(SMSC_MCISME_STAT_VERSION_INFO);
        file.Write(&version, sizeof(version));
        file.Flush();
      }
      fileTM = flushTM; bFileTM = true;
      //smsc_log_debug(logger, "%s file '%s' %s", (needHeader) ? "New":"Existed",
      //               fileName, (needHeader) ? "created":"opened");
    }

    //smsc_log_debug(logger, "Statistics data dump...");
    uint32_t value32 = htonl(buffLen);
    file.Write((const void *)&value32, sizeof(value32));
    file.Write((const void *)buff, buffLen); // write dump to it
    file.Write((const void *)&value32, sizeof(value32));
    file.Flush();
    //smsc_log_debug(logger, "Statistics data dumped.");

  }catch(std::exception & exc){
    if (file.isOpened()) file.Close();
    bFileTM = false;
    throw;
  }
}

bool StatisticsWriter::createDir(const std::string& dir)
{
  if (mkdir(dir.c_str(), S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH) != 0) {
    if (errno == EEXIST) return false;
    throw Exception("Failed to create directory '%s'. Details: %s", 
                    dir.c_str(), strerror(errno));
  }
  return true;
}

StatisticsWriter::StatisticsWriter()
  : location(""), logger(smsc::logger::Logger::getInstance("writer"))
{
}

void StatisticsWriter::init(const std::string& loc)
{
  location = loc; 
  if(!createStorageDir(location)){
    //smsc_log_warn(logger, "Can't create directory: '%s'", location.c_str());
    throw Exception("Can't create directory: '%s'", location.c_str());
  }
}

StatisticsWriter::~StatisticsWriter()
{
}
                                    
int main(int argc, char **argv)
{
  smsc::logger::Logger::Init();
  smsc::logger::Logger * logger = smsc::logger::Logger::getInstance("convert");

  Manager::init("config.xml");
  Manager& manager = Manager::getInstance();

  ConfigView dsConfig(manager, "StartupLoader");
  smsc_log_info(logger, "DataSourceLoader loading...");
  DataSourceLoader::loadup(&dsConfig);
  smsc_log_info(logger, "DataSourceLoader loaded");

  smsc_log_info(logger, "MCISme.DataSource reading...");
  ConfigView config(manager, "MCISme");

  std::string statLocation;       
  try { statLocation = config.getString("statisticsDir"); } catch (ConfigException& exc) {
    smsc_log_error(logger, "Parameter <MCISme.statisticsDir> missed.");
    return -1;
  }
  smsc_log_info(logger, "Statistics dir: '%s'", statLocation.c_str());

  {
    std::auto_ptr<ConfigView> dsIntCfgGuard(config.getSubConfig("DataSource"));
    smsc_log_info(logger, "DataSource config is read, initing...");
    std::auto_ptr<DataSource> dsGuard(createDataSource(dsIntCfgGuard.get(), logger));
    DataSource* ds = dsGuard.get();
    if (!ds) { 
      smsc_log_error(logger, "DataSource undefined"); 
      return -2;
    }
    smsc_log_info(logger, "DataSource init ok");


    Connection* dsStatConnection = 0;
    try {
      dsStatConnection = ds->getConnection();
      smsc_log_info(logger, "Data converting...");
      convert(dsStatConnection, logger, statLocation);
      smsc_log_info(logger, "Data converted");
    } catch (Exception& exc) {
      smsc_log_error(logger, "Convert failed. Cause: %s", exc.what());
    }

    if (ds && dsStatConnection) 
      ds->freeConnection(dsStatConnection);
  }

  DataSourceLoader::unload();
}

DataSource* createDataSource(ConfigView* config, smsc::logger::Logger* logger)
{
  DataSource* ds = 0;
  try  {
    std::auto_ptr<char> dsIdentity(config->getString("type"));
    const char* dsIdentityStr = dsIdentity.get();
    ds = DataSourceFactory::getDataSource(dsIdentityStr);
    if (!ds) 
      throw ConfigException("DataSource for '%s' identity wasn't registered !", dsIdentityStr);
    ds->init(config);
    smsc_log_info(logger, "DS init ok");
  } 
  catch (Exception& exc) {
    smsc_log_error(logger, "DataSource init failed: %s", exc.what());
    if (ds) delete ds; ds = 0;
  }
  return ds;
}

void convert(Connection* connection, smsc::logger::Logger* logger, const std::string& loc)
{
  const char* SELECT_EVENTS_STAT_SQL = (const char*)
    "SELECT period, missed, delivered, failed, notified FROM MCISME_STAT ORDER BY period asc";

  smsc_log_info(logger, "StatisticsWriter initialization ...");
  StatisticsWriter sw; sw.init(loc);
  smsc_log_info(logger, "StatisticsWriter is initialized ok");
  tm tmFlushTM;
  tmFlushTM.tm_min = 0;
  tmFlushTM.tm_sec = 0;

  std::auto_ptr<Statement> stmtGuard(connection->createStatement(SELECT_EVENTS_STAT_SQL));
  Statement* statement = stmtGuard.get();
  if (!statement)
    throw Exception("Failed to obtain statement for statistics converting");

  smsc_log_info(logger, "Statement created");

  std::auto_ptr<ResultSet> rsGuard(statement->executeQuery());
  ResultSet* rs = rsGuard.get();
  if (!rs)
    throw Exception("Failed to obtain query result");

  while(rs->fetchNext())
  {
    uint32_t period     = rs->getUint32(1);
    uint32_t missed     = rs->getUint32(2);
    uint32_t delivered  = rs->getUint32(3);
    uint32_t failed     = rs->getUint32(4);
    uint32_t notified   = rs->getUint32(5);

    tmByPeriod(tmFlushTM, period);
    smsc_log_debug(logger, "Flushing statistics for %02d.%02d.%04d %02d:%02d:%02d %d GMT",
                   tmFlushTM.tm_mday, tmFlushTM.tm_mon+1, tmFlushTM.tm_year+1900,
                   tmFlushTM.tm_hour, tmFlushTM.tm_min, tmFlushTM.tm_sec, period);
    sw.flushCounters(tmFlushTM, period, missed, delivered, notified);
  }
}

void tmByPeriod(tm& tmFlushTM, uint32_t period)
{
  int val = period/1000000;
  tmFlushTM.tm_year = val - 1900;    period -= val*1000000; val = period/10000;
  tmFlushTM.tm_mon =  val - 1;       period -= val*10000;   val = period/100;
  tmFlushTM.tm_mday = val;           period -= val*100;     
  tmFlushTM.tm_hour = period;
}
