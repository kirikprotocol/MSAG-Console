
#include <util/config/Config.h>

#include "CommandProcessor.h"

namespace smsc { namespace dbsme 
{

/* ---------------------- Abstract Command Processing ---------------------- */

CommandProcessor::CommandProcessor(ConfigView* config)
    throw(ConfigException)
        : log(Logger::getCategory("smsc.dbsme.CommandProcessor"))
{
    // create Providers by config
    ConfigView* providersConfig = config->getSubConfig("DataProviders");
    std::set<std::string>* set = providersConfig->getSectionNames();
    
    for (std::set<std::string>::iterator i=set->begin();i!=set->end();i++)
    {
        const char* section = (const char *)i->c_str();
        ConfigView* providerConfig = 
            providersConfig->getSubConfig(section, true);
        const char* address = 0;
        try
        {
            log.info("Loading DataProvider for section '%s'.", section);
            DataProvider* provider = new DataProvider(providerConfig);
            address = providerConfig->getString("address");
            providers.Insert(address, provider);
            log.info("Loaded DataProvider for section '%s'."
                     " Bind address is: %s", section, address);
        }
        catch (ConfigException& exc)
        {
            log.error("Load of CommandProcessor failed !"
                      " Config exception: %s", exc.what());
            if (set) delete set;
            if (address) delete address;
            delete providersConfig;
            delete providerConfig;
            throw exc;
        }
        if (address) delete address;
        delete providerConfig;
    }
    if (set) delete set;
    delete providersConfig;
}

CommandProcessor::~CommandProcessor()
{
    hashchar*       key = 0;
    DataProvider*   provider = 0;

    while (providers.Next(key, provider))
    {
        if (provider) delete provider;
    }
}

void CommandProcessor::process(Command& command)
    throw(ServiceNotFoundException, CommandProcessException)
{
    DataProvider* provider = 0;
    if (!providers.Exists(command.getToAddress().value))
        throw ServiceNotFoundException(command.getToAddress().value);

    providers.Get(command.getToAddress().value)->process(command);
}

/* --------------------- Command Processing (DataProvider) ----------------- */

DataProvider::DataProvider(ConfigView* config)
    throw(ConfigException) 
        : ds(0), log(Logger::getCategory("smsc.dbsme.DataProvider"))
{
    ConfigView* dsConfig = config->getSubConfig("DataSource");
    char* dsIdentity = 0;
    try
    {
        dsIdentity = dsConfig->getString("type");
        try 
        {
            ds = DataSourceFactory::getDataSource(dsIdentity);
            if (ds)
            {
                ds->init(dsConfig);
            }
            else
            {
                char errorMsg[2048];
                sprintf(errorMsg, 
                        "DataSource for '%s' identity wasn't registered !",
                        dsIdentity);
                throw ConfigException(errorMsg);
            }
        }
        catch (ConfigException& exc)
        {
            if (ds) delete ds;
            throw exc;
        }
        if (dsIdentity) delete dsIdentity;
    }
    catch (ConfigException& exc)
    {
        if (dsIdentity) delete dsIdentity;
        if (dsConfig) delete dsConfig;
        throw exc;
    }
    delete dsConfig;
    
    // create Jobs by config
    ConfigView* jobsConfig = config->getSubConfig("Jobs");
    std::set<std::string>* set = jobsConfig->getSectionNames();
    
    for (std::set<std::string>::iterator i=set->begin();i!=set->end();i++)
    {
        const char* section = (const char *)i->c_str();
        ConfigView* jobConfig = 
            jobsConfig->getSubConfig(section, true);
        const char* name = 0;
        try
        {
            log.info("Loading Job for '%s' section.", section);
            
            name = jobConfig->getString("id");
            Job* job = JobFactory::getJob(name);
            if (job)
            {
                job->init(jobConfig);
                jobs.Insert(name, job);
            }
            else
            {
                char errorMsg[2048];
                sprintf(errorMsg, "Job '%s' wasn't registered !", name);
                throw ConfigException(errorMsg);
            }
            
            log.info("Loaded Job for '%s' section. Job name is: '%s'",
                     section, name);
        }
        catch (ConfigException& exc)
        {
            log.error(exc.what());
            if (set) delete set;
            if (name) delete name;
            delete jobsConfig;
            delete jobConfig;
            throw exc;
        }
        if (name) delete name;
        delete jobConfig;
    }
    if (set) delete set;
    delete jobsConfig;
}

DataProvider::~DataProvider() 
{
    hashchar*   key = 0;
    Job*        job = 0;

    while (jobs.Next(key, job))
    {
        if (job) delete job;
    }

    if (ds) delete ds;
}

void DataProvider::process(Command& command)
    throw(ServiceNotFoundException, CommandProcessException)
{
    if (!jobs.Exists(command.getJobName()))
        throw ServiceNotFoundException(command.getJobName());

    jobs.Get(command.getJobName())->process(command, *ds);
}


}}


