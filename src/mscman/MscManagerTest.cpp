#include <stdlib.h>
#include <util/config/Manager.h>
#include <util/config/ConfigView.h>

#include "MscManager.h"

#include <db/DataSource.h>
#include <db/DataSourceLoader.h>

using namespace smsc::db;
using namespace smsc::mscman;

using smsc::util::config::Manager;
using smsc::util::config::ConfigView;
using smsc::util::config::ConfigException;

int main(void)
{
    Logger::Init();
    const char* OCI_DS_FACTORY_IDENTITY = "OCI";

    try
    {
        DataSourceLoader::loadupDataSourceFactory(
            "../db/oci/libdb_oci.so", OCI_DS_FACTORY_IDENTITY);

        DataSource* ds =
            DataSourceFactory::getDataSource(OCI_DS_FACTORY_IDENTITY);

        if (!ds)
        {
            printf("Init DataSource failed !\n");
            return -1;
        }

        Manager::init("config.xml");
        ConfigView* config =
            new ConfigView(Manager::getInstance(), "DataSource");
        ds->init(config);

        MscManager::startup(*ds, Manager::getInstance());
        MscAdmin& admin = MscManager::getMscAdmin();
        MscStatus& status = MscManager::getMscStatus();

        for(int i=0;i<10000;i++)
        {
          char buf[20];
          memset(buf,0,sizeof(buf));
          for(int j=0;j<5;j++)buf[j]='0'+(rand()%10);
          status.report(buf, true);
          status.report(buf, false);
          status.check(buf);
        }

        Array<MscInfo> infos = admin.list();
        for (int i=0; i<infos.Count(); i++)
        {
            MscInfo info = infos[i];
            printf("Msc: %s mLock=%d aLock=%d fCount=%d\n",
                   info.mscNum, info.manualLock,
                   info.automaticLock, info.failureCount);
        }
        MscManager::shutdown();
    }
    catch (Exception& exc) {
        printf("Ooops! Exception: %s", exc.what());
        MscManager::shutdown();
    }

    return 0;
}
