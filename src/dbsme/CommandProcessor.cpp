#include <sys/time.h>

#include <util/config/Manager.h>
#include "CommandProcessor.h"

namespace smsc { namespace dbsme 
{

using smsc::util::config::Manager;

Hash<JobFactory *>*  JobFactory::registry = 0;

/* ---------------------- Abstract Command Processing ---------------------- */

CommandProcessor::CommandProcessor()
    : log(Logger::getCategory("smsc.dbsme.CommandProcessor")),
        svcType(0), protocolId(0) {}
CommandProcessor::CommandProcessor(ConfigView* config)
    throw(ConfigException)
        : log(Logger::getCategory("smsc.dbsme.CommandProcessor")),
            svcType(0), protocolId(0) { init(config); }

void CommandProcessor::init(ConfigView* config)
    throw(ConfigException)
{
    messages.init(config);

    try { protocolId = config->getInt("ProtocolId"); }
    catch(ConfigException& exc) { protocolId = 0; };
    
    try { svcType = config->getString("SvcType"); }
    catch(ConfigException& exc) { svcType = 0; };
    
    std::auto_ptr<ConfigView> providersCfgGuard(config->getSubConfig("DataProviders"));
    ConfigView* providersConfig = providersCfgGuard.get();
    if (!providersConfig) throw ConfigException("DataProviders section missed !");
    std::auto_ptr< std::set<std::string> > setGuard(providersConfig->getShortSectionNames());
    std::set<std::string>* set = setGuard.get();
    
    for (std::set<std::string>::iterator i=set->begin();i!=set->end();i++)
    {
        const char* providerId = (const char *)i->c_str();
        if (!providerId || providerId[0] == '\0')
            throw ConfigException("Provider id empty or wasn't specified");
        log.info("Loading DataProvider '%s' ...", providerId);

        std::auto_ptr<ConfigView> providerCfgGuard(providersConfig->getSubConfig(providerId));
        ConfigView* providerConfig = providerCfgGuard.get();

        try
        {
            if (hasProvider(providerId))
                throw ConfigException("DataProvider '%s' already registered.", providerId);
            
            std::auto_ptr<char> addressGuard(providerConfig->getString("address"));
            const char* address = addressGuard.get();
            
            Address addr(address); 
            if (hasProvider(addr))
                throw ConfigException("Failed to bind DataProvider '%s' to address: '%s'. "
                                      "Address already in use !", 
                                      providerId, (address) ? address:"");
            
            DataProvider* provider = new DataProvider(this, providerConfig, messages); 
            if (!addProvider(providerId, addr, provider)) 
                throw ConfigException("Failed to bind DataProvider '%s' to address: '%s'. "
                                      "Address already used by internal job !",
                                      providerId, (address) ? address:"");
            
            log.info("Loaded DataProvider '%s'. Primary bind address is: %s", 
                     providerId, (address) ? address:"");
        }
        catch (ConfigException& exc)
        {
            log.error("Load of CommandProcessor failed ! %s", exc.what());
            throw;
        }
    }
}

CommandProcessor::~CommandProcessor()
{
    clean();
}

void CommandProcessor::clean()
{
    MutexGuard guard(providersLock);
    
    char* id = 0; DataProvider* provider = 0; allProviders.First();
    while (allProviders.Next(id, provider))
    {
        if (!provider) continue;
        provider->finalize();
        log.debug("Provider '%s' destructed.", id ? id:"");
    }
    allProviders.Empty();
    idxProviders.Empty();
    
    messages.clear();
    if (svcType) delete svcType; svcType=0;
}

bool CommandProcessor::addProvider(const char* id, const Address& address, DataProvider *provider)
{
    __require__(id && provider);
    {
        MutexGuard guard(providersLock);
        allProviders.Insert(id, provider);
    }
    return addProviderIndex(address, provider);
}
bool CommandProcessor::addProviderIndex(const Address& address, DataProvider *provider)
{
    __require__(provider);
    MutexGuard guard(providersLock);
    FullAddressValue fav;
    address.toString(fav, MAX_FULL_ADDRESS_VALUE_LENGTH);
    if (idxProviders.Exists(fav)) return false;
    idxProviders.Insert(fav, provider);
    return true;
}
bool CommandProcessor::delProviderIndex(const char* idx)
{
    MutexGuard guard(providersLock);
    if (!idxProviders.Exists(idx)) return false;
    idxProviders.Delete(idx);
    return true;
}
bool CommandProcessor::hasProvider(const char* id)
{
    if (!id || id[0] == '\0') return false;
    MutexGuard guard(providersLock);
    return allProviders.Exists(id);
}
bool CommandProcessor::hasProvider(const Address& address)
{
    MutexGuard guard(providersLock);
    FullAddressValue fav;
    address.toString(fav, MAX_FULL_ADDRESS_VALUE_LENGTH);
    return idxProviders.Exists(fav);
}
ProviderGuard CommandProcessor::getProvider(const char* id)
{
    if (!id || id[0] == '\0') return false;
    MutexGuard guard(providersLock);
    return ProviderGuard(allProviders.Exists(id) ? allProviders.Get(id):0);
}
ProviderGuard CommandProcessor::getProvider(const Address& address)
{
    MutexGuard guard(providersLock);
    FullAddressValue fav;
    address.toString(fav, MAX_FULL_ADDRESS_VALUE_LENGTH);
    return ProviderGuard(idxProviders.Exists(fav) ? idxProviders.Get(fav):0);
}

void CommandProcessor::process(Command& command)
    throw(CommandProcessException)
{
    const Address& address = command.getToAddress();
    ProviderGuard pg = getProvider(address);
    DataProvider* provider = pg.get();
    if (!provider)
    {
        const char* message = messages.get(PROVIDER_NOT_FOUND);
        log.error("%s Requesting: '.%d.%d.%s'", message ? message : PROVIDER_NOT_FOUND,
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
    std::auto_ptr<ConfigView> dsCfgGuard(config->getSubConfig("DataSource"));
    ConfigView* dsConfig = dsCfgGuard.get();
    
    std::auto_ptr<char> dsIdentityGuard(dsConfig->getString("type"));
    const char* dsIdentity = dsIdentityGuard.get();
    try 
    {
        ds = DataSourceFactory::getDataSource(dsIdentity);
        if (ds) ds->init(dsConfig);
        else throw ConfigException("DataSource for '%s' identity wasn't registered !",
                                   dsIdentity);
    }
    catch (ConfigException& exc)
    {
        if (ds) delete ds; ds = 0;
        throw;
    }
}

DataProvider::DataProvider(CommandProcessor* root, ConfigView* config, const MessageSet& mset)
    throw(ConfigException) 
        : log(Logger::getCategory("smsc.dbsme.DataProvider")), messages(mset, config),
            bFinalizing(false), bEnabled(false), owner(root), ds(0)
{
    createDataSource(config);

    bEnabled = config->getBool("enabled");
    std::auto_ptr<ConfigView> jobsCfgGuard(config->getSubConfig("Jobs"));
    ConfigView* jobsConfig = jobsCfgGuard.get();
    if (!jobsConfig) throw ConfigException("Jobs section missed !");
    std::auto_ptr< std::set<std::string> > setGuard(jobsConfig->getShortSectionNames());
    std::set<std::string>* set = setGuard.get();
    
    for (std::set<std::string>::iterator i=set->begin();i!=set->end();i++)
    {
        const char* jobId = (const char *)i->c_str();
        if (!jobId || jobId[0] == '\0')
            throw ConfigException("Job id empty or wasn't specified");
        
        log.info("Loading Job '%s' ...", jobId);
        std::auto_ptr<ConfigView> jobCfgGuard(jobsConfig->getSubConfig(jobId));
        ConfigView* jobConfig = jobCfgGuard.get();
        if (!jobConfig) throw ConfigException("Job section missed !");
        createJob(jobId, jobConfig);
        log.info("Loaded Job '%s'.", jobId);
    }

    usersCount[0] = 0; usersCount[1] = 0;
}

DataProvider::~DataProvider() 
{
    MutexGuard guard(jobsLock);

    char* id = 0; Job* job = 0; allJobs.First();
    while (allJobs.Next(id, job))
    {
        if (!job) continue;
        job->finalize();
        log.debug("Job '%s' destructed.", id ? id:"");
    }
    
    messages.clear();
    if (ds) delete ds;
}

JobGuard DataProvider::getJob(const Address& address)
{
    MutexGuard guard(jobsLock);
    FullAddressValue fav;
    address.toString(fav, MAX_FULL_ADDRESS_VALUE_LENGTH);
    return JobGuard(jobsByAddress.Exists(fav) ? jobsByAddress.Get(fav):0);
}
JobGuard DataProvider::getJob(const char* name)
{
    MutexGuard guard(jobsLock);
    return JobGuard(jobsByName.Exists(name) ? jobsByName.Get(name):0);
}

void DataProvider::registerJob(Job* job, const char* id, const char* address, 
                               const char* alias, const char* name)
    throw(ConfigException)
{
    __require__(job && id && owner);

    MutexGuard guard(jobsLock);
    if (allJobs.Exists(id))
        throw ConfigException("Job registration failed! Job with id '%s' already registered.");

    if (!name && !alias && !address)
        throw ConfigException("Job registration failed! Neither name, nor address, nor alias "
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
        std::auto_ptr<char> upname(strToUpperCase(name));
        if (jobsByName.Exists(upname.get())) 
            throw ConfigException("Job registration failed! Name/Alias: "
                                  "'%s' already in use.", name);
        jobsByName.Insert(upname.get(), job);
    }
    if (alias)
    {
        std::auto_ptr<char> upalias(strToUpperCase(alias));
        if (jobsByName.Exists(upalias.get())) 
            throw ConfigException("Job registration failed! Name/Alias: "
                                  "'%s' already in use.", alias);
        jobsByName.Insert(upalias.get(), job);
    }

    allJobs.Insert(id, job);
}

void DataProvider::createJob(const char* id, ConfigView* jobConfig)
    throw(ConfigException)
{
    std::auto_ptr<char> typeGuard   (jobConfig->getString("type"));
    const char* type  = typeGuard.get();
    std::auto_ptr<char> nameGuard   (jobConfig->getString("name",    0, false));
    const char* name  = nameGuard.get();
    if(name && strlen(name) == 0) name = 0;
    std::auto_ptr<char> aliasGuard  (jobConfig->getString("alias",   0, false));
    const char* alias = aliasGuard.get();
    if(alias && strlen(alias) == 0) alias = 0;
    std::auto_ptr<char> addressGuard(jobConfig->getString("address", 0, false));
    const char* address = addressGuard.get();
    if(address && strlen(address) == 0 ) address = 0;

    Job* job = JobFactory::getJob(type);
    if (!job) 
        throw ConfigException("Job creation failed! Can't find JobFactory for type '%s'", 
                              (type) ? type:"");
    try 
    {
        job->init(jobConfig, messages);
        registerJob(job, id, address, alias, name);
    } 
    catch (...) {
        if (job) job->finalize();
        throw;
    }
}
void DataProvider::removeJob(const char* id)
{
    __require__(id && owner);

    MutexGuard guard(jobsLock);

    Job* job = 0; 
    if (!allJobs.Exists(id) || !(job = allJobs.Get(id)))
        throw Exception("Job '%s' not registered", id);
    
    char* name = 0; Job* foundJob = 0; jobsByName.First();
    while (jobsByName.Next(name, foundJob)) {
        if (job == foundJob) jobsByName.Delete(name);
    }
    
    char* address = 0; jobsByAddress.First();
    while (jobsByAddress.Next(address, foundJob)) {
        if (job == foundJob) {
            jobsByAddress.Delete(address);
            owner->delProviderIndex(address);
        }
    }
    
    allJobs.Delete(id);
    job->finalize();
}

void DataProvider::process(Command& command)
    throw(CommandProcessException)
{
    const Address& destination = command.getToAddress();
    if (!isEnabled())
    {
        const char* message = messages.get(SERVICE_NOT_AVAIL);
        log.warn("%s Requesting address: '.%d.%d.%s'",
                 message ? message:SERVICE_NOT_AVAIL,
                 destination.type, destination.plan, destination.value);
        throw CommandProcessException(message ? message:SERVICE_NOT_AVAIL);
    }
    
    Job* job = 0; JobGuard jg = getJob(destination);
    if (!(job = jg.get()))
    {
        const char* name = command.getJobName();
        if (!name)
        {
            int curPos = 0;
            const char* input = command.getInData();
            std::string str = "";

            while (input && isspace(input[curPos])) curPos++; 
            while (input && isalnum(input[curPos])) str += input[curPos++];

            name = str.c_str();
            if (!name || name[0] == '\0') 
            {
                name = SMSC_DBSME_DEFAULT_JOB_NAME;
                log.warn("Job name/alias missed! Requesting address: '.%d.%d.%s'",
                         destination.type, destination.plan, destination.value);
            }
            command.setJobName(name);
            name = command.getJobName();

            str = &input[curPos];
            command.setInData(str.c_str());
        }
        
        std::auto_ptr<char> upNameGuard(strToUpperCase(name));
        char* upname = upNameGuard.get();
        jg = getJob(upname);
        if (!upname || !(job = jg.get())) 
        {
            const char* message = messages.get(JOB_NOT_FOUND);
            log.error("%s Requesting address: '.%d.%d.%s', name/alias: '%s'",
                      message ? message:JOB_NOT_FOUND,
                      destination.type, destination.plan, destination.value,
                      (name) ? name:"");
            throw CommandProcessException(message ? message:JOB_NOT_FOUND);
        }
    }

    struct timeval utime, curtime;
    if( log.isInfoEnabled() ) gettimeofday( &utime, 0 );
    try 
    {
        job->process(command, *ds);
        if( log.isInfoEnabled() ) {
            long usecs;
            gettimeofday( &curtime, 0 );
            usecs = curtime.tv_usec < utime.tv_usec?(1000000+curtime.tv_usec)-utime.tv_usec:curtime.tv_usec-utime.tv_usec;
            log.info( "job %s processed in s=%ld us=%ld", job->getName(), curtime.tv_sec-utime.tv_sec, usecs );
	    }
    }
    catch (CommandProcessException& exc)
    {
      if( log.isInfoEnabled() ) {
          long usecs;
          gettimeofday( &curtime, 0 );
          usecs = curtime.tv_usec < utime.tv_usec?(1000000+curtime.tv_usec)-utime.tv_usec:curtime.tv_usec-utime.tv_usec;
          log.info( "job %s unsuccsess in s=%ld us=%ld", job->getName(), curtime.tv_sec-utime.tv_sec, usecs );
      }
      throw;
    }
}

void CommandProcessor::addJob(std::string providerId, std::string jobId)
{
    const char* providerIdStr = providerId.c_str();
    ProviderGuard pg = getProvider(providerIdStr);
    DataProvider* provider = pg.get();
    if (!provider) 
        throw Exception("Provider '%s' not exists.", providerIdStr);

    const char* jobIdStr = jobId.c_str();
    try 
    {
        Manager::reinit();
        char jobSection[1024];
        sprintf(jobSection, "DBSme.DataProviders.%s.Jobs.%s", providerIdStr, jobIdStr);
        ConfigView jobConfig(Manager::getInstance(), jobSection);
        
        provider->createJob(jobIdStr, &jobConfig);
    
    } catch (Exception& exc) {
        log.error("Failed to add job '%s.%s'. Details: %s", 
                  providerIdStr, jobIdStr, exc.what());
        throw;
    } catch (std::exception& exc) {
        log.error("Failed to add job '%s.%s'. Details: %s",
                  providerIdStr, jobIdStr, exc.what());
        throw Exception("%s", exc.what());
    } catch (...) {
        log.error("Failed to add job '%s.%s'. Cause is unknown", 
                  providerIdStr, jobIdStr);
        throw Exception("Cause is unknown");
    }
}
void CommandProcessor::removeJob(std::string providerId, std::string jobId)
{
    const char* providerIdStr = providerId.c_str();
    ProviderGuard pg = getProvider(providerIdStr);
    DataProvider* provider = pg.get();
    if (!provider) 
        throw Exception("Provider '%s' not exists.", providerIdStr);

    const char* jobIdStr = jobId.c_str();
    try 
    {
        provider->removeJob(jobIdStr);
    
    } catch (Exception& exc) {
        log.error("Failed to remove job '%s.%s'. Details: %s",
                  providerIdStr, jobIdStr, exc.what());
        throw;
    } catch (std::exception& exc) {
        log.error("Failed to remove job '%s.%s'. Details: %s",
                  providerIdStr, jobIdStr, exc.what());
        throw Exception("%s", exc.what());
    } catch (...) {
        log.error("Failed to remove '%s.%s'. Cause is unknown",
                  providerIdStr, jobIdStr);
        throw Exception("Cause is unknown");
    }
}
void CommandProcessor::changeJob(std::string providerId, std::string jobId)
{
    const char* providerIdStr = providerId.c_str();
    ProviderGuard pg = getProvider(providerIdStr);
    DataProvider* provider = pg.get();
    if (!provider) 
        throw Exception("Provider '%s' not exists.", providerIdStr);

    const char* jobIdStr = jobId.c_str();
    try 
    {
        provider->removeJob(jobIdStr);
        
        Manager::reinit();
        char jobSection[1024];
        sprintf(jobSection, "DBSme.DataProviders.%s.Jobs.%s", providerIdStr, jobIdStr);
        ConfigView jobConfig(Manager::getInstance(), jobSection);
        
        provider->createJob(jobIdStr, &jobConfig);
    
    } catch (Exception& exc) {
        log.error("Failed to change job '%s.%s'. Details: %s",
                  providerIdStr, jobIdStr, exc.what());
        throw;
    } catch (std::exception& exc) {
        log.error("Failed to change job '%s.%s'. Details: %s",
                  providerIdStr, jobIdStr, exc.what());
        throw Exception("%s", exc.what());
    } catch (...) {
        log.error("Failed to change '%s.%s'. Cause is unknown",
                  providerIdStr, jobIdStr);
        throw Exception("Cause is unknown");
    }
}
void CommandProcessor::setProviderEnabled(std::string providerId, bool enabled)
{
    const char* providerIdStr = providerId.c_str();
    DataProvider* provider = 0;
    
    {
        ProviderGuard pg = getProvider(providerIdStr);
        provider = pg.get();
        
        if (!provider) 
            throw Exception("Provider '%s' not exists.", providerIdStr);
    }
    
    provider->setEnabled(enabled);
}

}}


