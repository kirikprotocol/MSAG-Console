
#include <stdio.h>
#include <stdlib.h>

#include <util/config/Manager.h>
#include <util/config/ConfigView.h>

#include "DataSource.h"

int main(void) 
{
    using smsc::util::config::Manager;
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;
    using namespace smsc::dbsme;

    try 
    {
        Manager::init("config.xml");
        DataSource* ds = DataSourceFactory::getDataSource("OCI");
        if (ds)
        {
            ConfigView* config = 
                new ConfigView(Manager::getInstance(),
                    "Applications.DBSme.DataProvider.DataSource");
            
            ds->init(config);
            printf("Init Ok !\n");
            if (config) delete config;
        }
    } 
    catch (exception& exc) 
    {
        printf("Exception : %s\n", exc.what());
        return -1;
    }
    
    return 0;
}
