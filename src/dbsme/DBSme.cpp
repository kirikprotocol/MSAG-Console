
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
    
    SampleJobFactory    _sampleJobFactory;
    SQLJobFactory       _sqlJobFactory;

    JobFactory::registerFactory(&_sampleJobFactory,
                                SMSC_DBSME_SAMPLE_JOB_IDENTITY);
    JobFactory::registerFactory(&_sqlJobFactory,
                                SMSC_DBSME_SQL_JOB_IDENTITY);

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
        //command.setJobName("test-sql-job");
        command.setJobName(0);
        /*command.setInData("-55 \t \t 666 \n -1573175230 129 3533 4364643 "
                          "first \"second value\""
                          " 243684.875874387835"
                          " +5753.14e-10"
                          " -45454.5e+7");*/
        command.setInData("\"Testing string value\" 1 "
                          "25 January, 2003 05:34 PM");
        
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
