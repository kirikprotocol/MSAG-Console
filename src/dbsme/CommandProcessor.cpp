
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
            
            address = providerConfig->getString("address");
            Address addr(address); 
            DataProvider* provider = getProvider(addr);
            if (provider)
                throw ConfigException("Failed to bind DataProvider to address: "
                                      "'%s'. Address already in use !",
                                      (address) ? address:"");
            provider = new DataProvider(this, providerConfig, messages); 
            if (!addProvider(addr, provider)) 
                throw ConfigException("Failed to bind DataProvider to address: "
                                      "'%s'. Address already used by internal job !",
                                      (address) ? address:"");

            log.info("Loaded DataProvider for section '%s'. "
                     "Primary bind address is: %s", 
                     section, (address) ? address:"");
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
    if (svcType) delete svcType;
    for (int i=0; i<allProviders.Count(); i++) 
    {
        DataProvider* provider = allProviders[i];
        if (provider) delete provider;
        __trace__("Provider destructed.");
    }
}

bool CommandProcessor::addProvider(const Address& address,
                                   DataProvider *provider)
{
    __require__(provider);
    allProviders.Push(provider);
    return addProviderIndex(address, provider);
}
bool CommandProcessor::addProviderIndex(const Address& address, 
                                        DataProvider *provider)
{
    __require__(provider);
    FullAddressValue fav;
    address.toString(fav, MAX_FULL_ADDRESS_VALUE_LENGTH);
    if (idxProviders.Exists(fav)) return false;
    idxProviders.Insert(fav, provider);
    return true;
}
DataProvider* CommandProcessor::getProvider(const Address& address)
{
    FullAddressValue fav;
    address.toString(fav, MAX_FULL_ADDRESS_VALUE_LENGTH);
    if (idxProviders.Exists(fav))
        return idxProviders.Get(fav);
    return 0;
}

void CommandProcessor::process(Command& command)
    throw(CommandProcessException)
{
    const Address& address = command.getToAddress();
    DataProvider* provider = getProvider(address);
    if (!provider)
    {
        const char* message = messages.get(PROVIDER_NOT_FOUND);
        log.error("%s Requesting: '.%d.%d.%s'", 
                  message ? message : PROVIDER_NOT_FOUND,
                  address.type, address.plan, address.value);
        throw CommandProcessException(message ? message : PROVIDER_NOT_FOUND);
    }
    provider->process(command);
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

void DataProvider::createDataSource(ConfigView* config)
    throw(ConfigException)
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
            else throw ConfigException("DataSource for '%s' identity "
                                       "wasn't registered !", dsIdentity);
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
}

void DataProvider::registerJob(Job* job,
    const char* address, const char* alias, const char* name)
        throw(ConfigException)
{
    __require__(owner);

    if (!name && !alias && !address)
        throw ConfigException("Job registration failed! "
                              "Neither name, nor address, nor alias "
                              "parameters specified.");
    if (address)
    {
        Address addr(address); FullAddressValue fav;
        addr.toString(fav, MAX_FULL_ADDRESS_VALUE_LENGTH);
        if (jobsByAddress.Exists(fav)) 
            throw ConfigException("Job registration failed! Job with address: "
                                  "'%s' already registered.", fav);
        if (!owner->addProviderIndex(addr, this))
            throw ConfigException("Job registration failed! Address: "
                                  "'%s' already in use.", fav);
        jobsByAddress.Insert(fav, job);
    }
    if (name)
    {
        char* upname = strToUpperCase(name);
        if (jobsByName.Exists(upname)) 
        {
            delete upname;
            throw ConfigException("Job registration failed! Name/Alias: "
                                  "'%s' already in use.", name);
        }
        jobsByName.Insert(upname, job);
        delete upname;
    }
    if (alias)
    {
        char* upalias = strToUpperCase(alias);
        if (jobsByName.Exists(upalias)) 
        {                       
            delete upalias;
            throw ConfigException("Job registration failed! Name/Alias: "
                                  "'%s' already in use.", alias);
        }
        jobsByName.Insert(upalias, job);
        delete alias;
    }
}

void DataProvider::createJob(ConfigView* jobConfig)
    throw(ConfigException)
{
    const char* type = 0; const char* name = 0;
    const char* alias = 0; const char* address = 0;
    
    try
    {
        type = jobConfig->getString("type");
        name = jobConfig->getString("name", 0, false);
        alias = jobConfig->getString("alias", 0, false);
        if( alias && strlen( alias ) == 0 ) alias = 0;
        address = jobConfig->getString("address", 0, false);
        if( address && strlen( address ) == 0 ) address = 0;
        
        Job* job = JobFactory::getJob(type);
        if (!job) 
            throw ConfigException("Job creation failed! "
                                  "Can't find JobFactory for type '%s'", 
                                  (type) ? type:"");
        allJobs.Push(job);
        job->init(jobConfig, messages);
        registerJob(job, address, alias, name);
    }
    catch (ConfigException& exc)
    {
        if (type) delete type; if (name) delete name;
        if (alias) delete alias; if (address) delete address;
        throw;
    }
}
DataProvider::DataProvider(CommandProcessor* root,
                           ConfigView* config, const MessageSet& mset)
    throw(ConfigException) 
        : log(Logger::getCategory("smsc.dbsme.DataProvider")), 
            messages(mset, config), owner(root), ds(0)
{
    createDataSource(config);

    ConfigView* jobsConfig = config->getSubConfig("Jobs");
    std::set<std::string>* set = (jobsConfig) ? jobsConfig->getSectionNames():0;
    if (!set) throw ConfigException("Jobs section missed !");
    
    // create Jobs by config
    for (std::set<std::string>::iterator i=set->begin();i!=set->end();i++)
    {
        const char* section = (const char *)i->c_str();
        ConfigView* jobConfig = jobsConfig->getSubConfig(section, true);
        try
        {
            if (!jobConfig) throw ConfigException("Job section missed !");
            log.info("Loading Job for '%s' section.", section);
            createJob(jobConfig);
            log.info("Loaded Job for '%s' section.", section);
        }
        catch (ConfigException& exc)
        {
            log.error(exc.what());
            if (set) delete set;
            if (jobsConfig) delete jobsConfig;
            throw;
        }
        delete jobConfig;
    }
    if (set) delete set;
    delete jobsConfig;
}

DataProvider::~DataProvider() 
{
    for (int i=0; i<allJobs.Count(); i++) 
    {
        Job* job = allJobs[i];
        if (job) delete job;
        __trace__("Job destructed.");
    }
    if (ds) delete ds;
}

Job* DataProvider::getJob(const Address& address)
{
    FullAddressValue fav;
    address.toString(fav, MAX_FULL_ADDRESS_VALUE_LENGTH);
    return (jobsByAddress.Exists(fav) ? jobsByAddress.Get(fav):0);
}
Job* DataProvider::getJob(const char* name)
{
    return (jobsByName.Exists(name) ? jobsByName.Get(name):0);
}

void DataProvider::process(Command& command)
    throw(CommandProcessException)
{
    Job* job = 0;
    const Address& destination = command.getToAddress();
    if (!(job = getJob(destination)))
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

            name = str.c_str();
            if (!name || name[0] == '\0') 
            {
                name = SMSC_DBSME_DEFAULT_JOB_NAME;
                log.warn("Job name/alias missed! "
                         "Requesting address: '.%d.%d.%s'",
                         destination.type, destination.plan, destination.value);
            }
            command.setJobName(name);
            name = command.getJobName();

            str = &input[curPos];
            command.setInData(str.c_str());
        }
        
        char* upname = strToUpperCase(name);
        if (!upname || !(job = getJob(upname))) 
        {
            const char* message = messages.get(JOB_NOT_FOUND);
            log.error("%s Requesting address: '.%d.%d.%s', name/alias: '%s'",
                      message ? message:JOB_NOT_FOUND,
                      destination.type, destination.plan, destination.value,
                      (name) ? name:"");
            if (upname) delete upname;
            throw CommandProcessException(message ? message:JOB_NOT_FOUND);
        }
        if (upname) delete upname;
    }

    job->process(command, *ds);
}

}}


