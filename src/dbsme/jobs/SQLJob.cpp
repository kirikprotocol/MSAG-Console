
#include "SQLJob.h"
#include "SQLAdapters.h"

namespace smsc { namespace dbsme 
{

using namespace smsc::dbsme::io;

void SQLJob::init(ConfigView* config)
    throw(ConfigException)
{
    //Job::init(config);
    __require__(config);

    printf("SQLJob loading ... ");

    sql = config->getString("sql", "SQL request wasn't defined !");
    outputFormat = config->getString("output", 
                                    "Output data format wasn't defined !");
    isQuery = config->getBool("query", "Type of SQL request undefined !");

    ConfigView* inputConfig = config->getSubConfig("input");
    try 
    {
        parser = new InputParser(inputConfig);
    }
    catch(Exception& exc)
    {
        if (inputConfig) delete inputConfig;
        throw ConfigException("SQL Job init failed !");
    }
    if (inputConfig) delete inputConfig;

    printf("SQLJob loaded !\n");
}

void SQLJob::process(Command& command, Statement& stmt) 
    throw(CommandProcessException)
{
    printf("SQL Job: Process command called !!!\n");
    throw CommandProcessException();
}

}}

