#include <stdio.h>
#include "logger/Logger.h"
#include "db/DataSource.h"
#include "db/DataSourceLoader.h"
#include "profiler/profiler.hpp"
#include "util/config/Manager.h"

int main(int argc,char* argv[])
{
  using namespace smsc::profiler;
  smsc::logger::Logger::Init();
  smsc::util::config::Manager::init("config.xml");
  smsc::db::DataSource *dataSource;
  smsc::util::config::Manager& mngr=smsc::util::config::Manager::getInstance();
  {
    using namespace smsc::db;
    using smsc::util::config::ConfigView;
    const char* OCI_DS_FACTORY_IDENTITY = "OCI";


    std::auto_ptr<ConfigView> dsConfig(new smsc::util::config::ConfigView(mngr, "StartupLoader"));
    DataSourceLoader::loadup(dsConfig.get());

    dataSource = DataSourceFactory::getDataSource(OCI_DS_FACTORY_IDENTITY);
    if(!dataSource)
    {
      printf("no datasource\n");
      return -1;
    }

    std::auto_ptr<ConfigView> config(new ConfigView(mngr,"DataSource"));
    dataSource->init(config.get());
  }
  Profile prof;
  Profiler *p =new Profiler(prof,0,"profiler");

  p->loadFromDB(dataSource);

  return 0;
}
