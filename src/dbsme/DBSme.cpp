
#include <stdio.h>
#include <stdlib.h>

#include <util/config/Manager.h>
#include <util/config/ConfigView.h>

#include <db/DataSourceLoader.h>
#include "CommandProcessor.h"

int main(void) 
{
    using namespace smsc::db;
    using namespace smsc::dbsme;

    using smsc::util::config::Manager;
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;

    const char* OCI_DS_FACTORY_IDENTITY = "OCI";
    
    DataSourceLoader::loadupDataSourceFactory(
        "../db/oci/libdb_oci.so", OCI_DS_FACTORY_IDENTITY);

    ConfigView* config;
    try 
    {
        Manager::init("config.xml");
        config = new ConfigView(Manager::getInstance(), "Applications.DBSme");
        
        CommandProcessor processor(config);
        Command command;
        
        const char* toAddressStr = "1111111";
        Address toAddress(strlen(toAddressStr), 0, 0, toAddressStr);
        command.setToAddress(toAddress);
        command.setJobName("sample");

        processor.process(command);
    } 
    catch (Exception& exc) 
    {
        if (config) delete config;
        printf("Exception : %s\n", exc.what());
        return -1;
    }
    
    if (config) delete config;
    return 0;
}
