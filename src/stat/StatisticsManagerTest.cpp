
#include "StatisticsManager.h"

#include <util/config/Manager.h>
#include <util/config/ConfigView.h>

#include <db/DataSource.h>
#include <db/DataSourceLoader.h>

#include <core/threads/ThreadPool.hpp>

using namespace smsc::db;
using namespace smsc::stat;
using namespace smsc::core::threads;
    
using smsc::util::config::Manager;
using smsc::util::config::ConfigView;
using smsc::util::config::ConfigException;

int process(Statistics& stat)
{
    for (int i=0; i<100; i++)
    {
        stat.updateIncoming("AAA");
        stat.updateOutgoing("BBB", "RRR", i%2);
    }
    printf("Statistics flushing ...\n");
    stat.flushStatistics();
    printf("Statistics flushed !\n");
    return 0;
}

int main(void)
{
    const char* OCI_DS_FACTORY_IDENTITY = "OCI";
    
    DataSourceLoader::loadupDataSourceFactory(
        "../db/oci/libdb_oci.so", OCI_DS_FACTORY_IDENTITY);
    
    DataSource* ds = 
        DataSourceFactory::getDataSource(OCI_DS_FACTORY_IDENTITY);
    
    if (!ds)
    {
        printf("Create DataSource failed !\n");
        return -1;
    }
    
    Manager::init("config.xml");
    ConfigView* config = 
        new ConfigView(Manager::getInstance(), "DataSource");
    ds->init(config);
    
    StatisticsManager manager(*ds);
    
    ThreadPool  pool;
    pool.startTask(&manager);

    process(manager);

    sleep(10);
    return 0;
}
