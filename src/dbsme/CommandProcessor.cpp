
#include "CommandProcessor.h"

namespace smsc { namespace dbsme 
{

Hash<JobFactory *>*  JobFactory::registry = 0;

/* ---------------------- Abstract Command Processing ---------------------- */

CommandProcessor::CommandProcessor()
    : log(Logger::getCategory("smsc.dbsme.CommandProcessor")),
        svcType(0), protocolId(0)
{
}

CommandProcessor::CommandProcessor(ConfigView* config)
    throw(ConfigException)
        : log(Logger::getCategory("smsc.dbsme.CommandProcessor")),
            svcType(0), protocolId(0)
{
    init(config);
}

void CommandProcessor::init(ConfigView* config)
    throw(ConfigException)
{
    messages.init(config);

    try { protocolId = config->getInt("ProtocolId"); }
    catch(ConfigException& exc) { protocolId = 0; };
    
    try { svcType = config->getString("SvcType"); }
    catch(ConfigException& exc) { svcType = 0; };
    
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
            DataProvider* provider = new DataProvider(providerConfig, messages);
            address = providerConfig->getString("address");
            Address addr(address); AddressValue addr_val; addr.getValue(addr_val);
            if (providers.Exists(addr_val)) 
                throw ConfigException("Failed to bind DataProvider to address "
                                      "Address already in use !");
            providers.Insert(addr_val, provider);
            log.info("Loaded DataProvider for section '%s'."
                     " Bind address is: %s (%s)", section, addr_val, address);
        }
        catch (ConfigException& exc)
        {
            log.error("Load of CommandProcessor failed ! "
                      "Config exception: %s", exc.what());
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
        if (provider) delete provider;
     
    if (svcType) delete svcType;
}

void CommandProcessor::process(Command& command)
    throw(CommandProcessException)
{
    DataProvider* provider = 0;
    if (!providers.Exists(command.getToAddress().value))
    {
        const char* message = messages.get(PROVIDER_NOT_FOUND);
        log.error("%s Requesting: '%s'", 
                  message ? message : PROVIDER_NOT_FOUND,
                  (const char*)command.getToAddress().value);
        throw CommandProcessException(message ? message : PROVIDER_NOT_FOUND);
    }

    providers.Get(command.getToAddress().value)->process(command);
}

/* --------------------- Command Processing (DataProvider) ----------------- */

char* strToUpperCase(const char* str)
{
    if (!str) return 0;
    char* up = new char[strlen(str)+1];
    int cur = 0;
    while (*str) up[cur++] = (char)toupper(*str++);
    up[cur] = '\0';
    return up;
}

DataProvider::DataProvider(ConfigView* config, const MessageSet& set)
    throw(ConfigException) 
        : log(Logger::getCategory("smsc.dbsme.DataProvider")), 
            messages(set, config), ds(0)
{
    ConfigView* dsConfig = config->getSubConfig("DataSource");
    char* dsIdentity = 0;
    try
    {
        dsIdentity = dsConfig->getString("type");
        try 
        {
            ds = DataSourceFactory::getDataSource(dsIdentity);
            if (ds) ds->init(dsConfig);
            else throw ConfigException("DataSource for '%s' identity"
                                       " wasn't registered !", dsIdentity);
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
                job->init(jobConfig, messages);
                char* upname = strToUpperCase(name);
                if (upname)
                {
                    jobs.Insert(upname, job);
                    delete upname;
                }
                else
                {
                    delete job; job = 0;
                    throw ConfigException("Job with empty name can't be registered !");
                }
                
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
        if (job) delete job;

    if (ds) delete ds;
}

void DataProvider::process(Command& command)
    throw(CommandProcessException)
{
    const char* name = command.getJobName();

    if (!name)
    {
        int curPos = 0;
        const char* input = command.getInData();
        std::string str = "";
        
        while (input && isspace(input[curPos])) curPos++; 
        
        while (input && isalnum(input[curPos]))
            str += input[curPos++];
        
        if (!(name = str.c_str()) || name[0] == '\0')
            name = SMSC_DBSME_DEFAULT_JOB_NAME;
        else
            command.setInData(&input[curPos]);

        command.setJobName(name);
    }
    
    char* upname = strToUpperCase(name);
    Job* job = 0;
    if (!jobs.Exists(upname) || !(job = jobs.Get(upname))) 
    {
        const char* message = messages.get(JOB_NOT_FOUND);
        log.error("%s Requesting: '%s'",
                  message ? message:JOB_NOT_FOUND, name);
        if (upname) delete upname;
        throw CommandProcessException(message ? message:JOB_NOT_FOUND);
    }
    
    if (upname) delete upname;
    job->process(command, *ds);
}

}}


