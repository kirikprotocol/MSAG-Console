
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

        status.report("12345", true);
        status.report("54321", false);
        status.check("54321");

        Array<MscInfo> infos = admin.list();
        for (int i=0; i<infos.Count(); i++)
        {
            MscInfo info = infos[i];
            printf("Msc: %s mLock=%d aLock=%d fCount=%d\n",
                   info.mscNum.c_str(), info.manualLock, 
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

