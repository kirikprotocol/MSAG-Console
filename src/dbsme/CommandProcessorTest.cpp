
#include <stdio.h>
#include <stdlib.h>

#include <util/config/Manager.h>
#include <util/config/ConfigView.h>

#include <db/DataSourceLoader.h>
#include "CommandProcessor.h"

#include <dbsme/jobs/SampleJob.h>
#include <dbsme/jobs/SQLJob.h>

int main(int argc, char* argv[]) 
{
    /*printf("Args : %d\n", argc);
    for (int x=0; x<argc; x++)
    {
        printf("%d: %s\n", x, argv[x]);
    }*/
    if (argc != 3)
    {
        printf("Usage: ComProcTest <destination_address> \"<command>\"\n");
        return -1;
    }
    
    using namespace smsc::dbsme;

    using smsc::db::DataSourceLoader;
    using smsc::util::config::Manager;
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;
    
    SQLJobFactory       _sqlJobFactory;
    PLSQLJobFactory     _plsqlJobFactory;

    JobFactory::registerFactory(&_plsqlJobFactory,
                                SMSC_DBSME_PLSQL_JOB_IDENTITY);
    JobFactory::registerFactory(&_sqlJobFactory,
                                SMSC_DBSME_SQL_JOB_IDENTITY);
    try 
    {
        Manager::init("config.xml");
        Manager& manager = Manager::getInstance();
        
        ConfigView dsConfig(manager, "StartupLoader");
        DataSourceLoader::loadup(&dsConfig);

        ConfigView cpConfig(manager, "DBSme");
        CommandProcessor processor(&cpConfig);
        
        // --------------------- Command Processing --------------------

        Command command;
        Address toAddress(argv[1]);
        command.setToAddress(toAddress);
        command.setJobName(0);
        command.setInData(argv[2]);
        
        printf("\nInput  : '%s'\n",
               (command.getInData() ? command.getInData():""));
        
        processor.process(command);

        printf("\nOutput : '%s'\n", 
               (command.getOutData() ? command.getOutData():""));

        // --------------------- Command Processing --------------------
    }
    catch (Exception& exc) 
    {
        printf("Exception : %s\n", exc.what());
        return -1;
    }
    
    return 0;
}
