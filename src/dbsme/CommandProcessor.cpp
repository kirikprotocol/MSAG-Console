
#include "CommandProcessor.h"

namespace smsc { namespace dbsme 
{

Hash<JobFactory *>*  JobFactory::registry = 0;

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

    providers.First();
    while (providers.Next(key, provider))
    {
        //printf("Deleting provider '%s' ...\n", key);
        if (provider) delete provider;
        //printf("Provider deleted !\n");
    }
}

void CommandProcessor::process(Command& command)
    throw(ServiceNotFoundException, CommandProcessException)
{
    DataProvider* provider = 0;
    if (!providers.Exists(command.getToAddress().value))
        throw ServiceNotFoundException(
            (const char*)command.getToAddress().value);

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
                throw ConfigException("DataSource for '%s' identity"
                                      " wasn't registered !", dsIdentity);
            }
        }
        catch (ConfigException& exc)
        {
            if (ds) delete ds;
            throw;
        }
        if (dsIdentity) delete dsIdentity;
    }
    catch (ConfigException& exc)
    {
        if (dsIdentity) delete dsIdentity;
        if (dsConfig) delete dsConfig;
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
        const char* type = 0;
        try
        {
            log.info("Loading Job for '%s' section.", section);
            
            name = jobConfig->getString("name");
            type = jobConfig->getString("type");
            if (!name || !type)
                throw ConfigException("Job name or type missed !");
            
            Job* job = JobFactory::getJob(type);
            if (job)
            {
                job->init(jobConfig);
                jobs.Insert(name, job);
            }
            else
                throw ConfigException("Job '%s' wasn't registered !", name);
            
            log.info("Loaded Job for '%s' section. Job name: '%s' type: '%s'",
                     section, name, type);
        }
        catch (ConfigException& exc)
        {
            log.error(exc.what());
            if (set) delete set;
            if (name) delete name;
            if (type) delete type;
            delete jobsConfig;
            delete jobConfig;
            throw;
        }
        if (name) delete name;
        if (type) delete type;
        delete jobConfig;
    }
    if (set) delete set;
    delete jobsConfig;
}

DataProvider::~DataProvider() 
{
    hashchar*   key = 0;
    Job*        job = 0;

    jobs.First();
    while (jobs.Next(key, job))
    {
        //__trace2__("Deleting job '%s' ...\n", key);
        if (job) delete job;
        //__trace__("Job deleted !\n");
    }

    if (ds) delete ds;
}

void DataProvider::process(Command& command)
    throw(ServiceNotFoundException, CommandProcessException)
{
    const char* name = (command.getJobName()) ? 
                        command.getJobName() : SMSC_DBSME_DEFAULT_JOB_NAME;
    if (!jobs.Exists(name))
        throw ServiceNotFoundException(name);

    jobs.Get(name)->process(command, *ds);
}


}}


