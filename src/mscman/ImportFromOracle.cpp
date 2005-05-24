#include <stdio.h>
#include "mscman/MscManager.h"
#include "util/config/Manager.h"
#include "util/findConfigFile.h"
#include "db/DataSourceLoader.h"
#include "logger/Logger.h"
#include "core/buffers/File.hpp"

using namespace smsc::mscman;
using namespace smsc::util;
using namespace smsc::store;
using namespace smsc::db;
using namespace smsc::core::buffers;
using smsc::util::config::Manager;
using smsc::util::config::ConfigView;
using smsc::util::config::ConfigException;

class ConnectionGuard{
  smsc::db::DataSource *ds;
  smsc::db::Connection *conn;
public:
  ConnectionGuard(smsc::db::DataSource *_ds):ds(_ds)
  {
    conn=ds->getConnection();
  }
  ~ConnectionGuard()
  {
    ds->freeConnection(conn);
  }
  smsc::db::Connection* operator->()
  {
    return conn;
  }
  smsc::db::Connection* get()
  {
    return conn;
  }
};




int main(int argc,char* argv[])
{
  Logger::Init();
  Logger* log=Logger::getInstance("ora2file");
  smsc::logger::Logger *logger = Logger::getInstance("smscmain");
  smsc::util::config::Manager::init(findConfigFile("config.xml"));
  using namespace smsc::db;
  using smsc::util::config::ConfigView;
  const char* OCI_DS_FACTORY_IDENTITY = "OCI";

  config::Manager& cfgman=config::Manager::getInstance();

  std::auto_ptr<ConfigView> dsConfig(new config::ConfigView(config::Manager::getInstance(), "StartupLoader"));
  DataSourceLoader::loadup(dsConfig.get());

  DataSource* dataSource = DataSourceFactory::getDataSource(OCI_DS_FACTORY_IDENTITY);
  std::auto_ptr<ConfigView> config(new ConfigView(cfgman,"DataSource"));

  dataSource->init(config.get());

  ConnectionGuard  connection(dataSource);
  if (!connection.get())
  {
      InitException exc("Get connection to DB failed");
      smsc_log_error(log, exc.what());
      throw exc;
  }

  const char* selectSql ="SELECT MSC, M_LOCK, A_LOCK, F_COUNT FROM MSC_LOCK";

  File storeFile;
  const char sig[]="SMSCMSCM";
  uint32_t ver=0x00010000;
  string fileName=cfgman.getString("MscManager.storeFile");
  if(!File::Exists(fileName.c_str()))
  {
    storeFile.RWCreate(fileName.c_str());
    storeFile.Write(sig,8);
    storeFile.WriteNetInt32(ver);
  }else
  {
    storeFile.RWOpen(fileName.c_str());
  }


  int count=0;
  // Loadup msc info here
  Statement* statement = 0;
  ResultSet* rs = 0;
  try
  {
    statement = connection->createStatement(selectSql);
    if (!statement)
        throw InitException("Create statement failed");
    rs = statement->executeQuery();
    if (!rs)
        throw InitException("Get result set failed");

    while (rs->fetchNext())
    {
      const char* mscNum = rs->getString(1);
      if (!mscNum || !mscNum[0]) continue;
      const char* mLockStr = rs->isNull(2) ? 0:rs->getString(2);
      const char* aLockStr = rs->isNull(3) ? 0:rs->getString(3);
      int fc = rs->isNull(4) ? 0:rs->getInt32(4);
      bool mLock = (mLockStr && mLockStr[0]=='Y');
      bool aLock = (aLockStr && aLockStr[0]=='Y');
      MscInfo info(mscNum, mLock, aLock, fc);
      storeFile.WriteByte(1);
      info.Write(storeFile);
      storeFile.Flush();
      count++;
    }

    if (rs) delete rs;
    if (statement) delete statement;
  }
  catch (Exception& exc)
  {
      if (rs) delete rs;
      if (statement) delete statement;
      smsc_log_error(log, exc.what());
  }
  printf("%d records imported ok\n",count);

  return 0;
}
