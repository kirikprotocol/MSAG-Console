
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
        Array<MscInfo> info = admin.list();
    }
    catch (Exception& exc) {
        printf("Ooops! Exception: %s", exc.what());
    }
    
    return 0;
}

