
#include <stdio.h>
#include <stdlib.h>

#include <util/config/Manager.h>
#include <util/config/ConfigView.h>

#include <db/DataSourceLoader.h>
#include "CommandProcessor.h"

#include <dbsme/jobs/SampleJob.h>
#include <dbsme/jobs/SQLJob.h>

int main(void) 
{
    using namespace smsc::dbsme;

    using smsc::db::DataSourceLoader;
    using smsc::util::config::Manager;
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;
    
    const char* DEFAULT_JOB_IDENTITY = "default";
    const char* SAMPLE_JOB_IDENTITY = "sample1";
    const char* SQL_JOB_IDENTITY = "sql-job";
    
    SampleJobFactory _sampleJobFactory;
    SQLJobFactory _sqlJobFactory;
    JobFactory::registerFactory(&_sampleJobFactory, SAMPLE_JOB_IDENTITY);
    JobFactory::registerFactory(&_sampleJobFactory, DEFAULT_JOB_IDENTITY);
    JobFactory::registerFactory(&_sqlJobFactory, SQL_JOB_IDENTITY);

    ConfigView  *dsConfig, *cpConfig;
    try 
    {
        Manager::init("config.xml");
        Manager& manager = Manager::getInstance();
        
        dsConfig = new ConfigView(manager, "StartupLoader");
        DataSourceLoader::loadup(dsConfig);

        cpConfig = new ConfigView(manager, "Applications.DBSme");
        CommandProcessor processor(cpConfig);
        
        Command command;
        const char* toAddressStr = "1111111";
        Address toAddress(strlen(toAddressStr), 0, 0, toAddressStr);
        command.setToAddress(toAddress);
        //command.setJobName(SAMPLE_JOB_IDENTITY);
        command.setJobName(SQL_JOB_IDENTITY);
        command.setInData("Sample input !!!");

        printf("\nInput  : '%s'\n",
               (command.getInData() ? command.getInData():""));
        
        processor.process(command);

        printf("\nOutput : '%s'\n", 
               (command.getOutData() ? command.getOutData():""));
    } 
    catch (Exception& exc) 
    {
        if (dsConfig) delete dsConfig;
        if (cpConfig) delete cpConfig;
        printf("Exception : %s\n", exc.what());
        return -1;
    }
    
    if (dsConfig) delete dsConfig;
    if (cpConfig) delete cpConfig;
    return 0;
}
