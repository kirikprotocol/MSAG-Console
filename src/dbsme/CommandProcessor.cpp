
#include <util/config/Config.h>

#include "CommandProcessor.h"

namespace smsc { namespace dbsme 
{

/* ---------------------- Abstract Command Processing ---------------------- */

CommandProcessor::CommandProcessor(ConfigView* config)
    throw(ConfigException)
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
            printf("Loading DataProvider for section '%s'.\n", section);
            DataProvider* provider = new DataProvider(providerConfig);
            address = providerConfig->getString("address");
            providers.Insert(address, provider);
            printf("Loaded DataProvider for section '%s'."
                   " Bind address is: %s\n", section, address);
        }
        catch (ConfigException& exc)
        {
            if (set) delete set;
            if (address) delete address;
            delete providersConfig;
            delete providerConfig;
            throw;
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
    DataProvider* provider = providers.Get(command.getToAddress().value);
    if (!provider) throw ServiceNotFoundException();
    
    provider->process(command);
}

/* --------------------- Command Processing (DataProvider) ----------------- */

DataProvider::DataProvider(ConfigView* config)
    throw(ConfigException) : ds(0) 
{
    ConfigView* dsConfig = config->getSubConfig("DataSource");
    char* dsIdentity = 0;
    try
    {
        dsIdentity = dsConfig->getString("type");
        if (dsIdentity)
        {
            try 
            {
                ds = DataSourceFactory::getDataSource(dsIdentity);
                if (ds) ds->init(dsConfig);
                delete dsIdentity;
            }
            catch (ConfigException& exc)
            {
                if (ds) delete ds;
                throw;
            }
        }
    }
    catch (ConfigException& exc)
    {
        if (dsIdentity) delete dsIdentity;
        throw;
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
            printf("Loading section %s\n", section);
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
            printf("Loaded section '%s'."
                   " Job name is: %s\n", section, name);
        }
        catch (ConfigException& exc)
        {
            if (set) delete set;
            if (name) delete name;
            delete jobsConfig;
            delete jobConfig;
            throw;
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
    Job* job = jobs.Get(command.getJobName());
    if (!job) throw ServiceNotFoundException();
    
    job->process(command, *ds);
}


}}


