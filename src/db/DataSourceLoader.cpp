
#include <dlfcn.h>

#include "DataSourceLoader.h"

namespace smsc { namespace db
{
typedef DataSourceFactory* (*getDsfInstanceFn)(void);

using smsc::util::Logger;

log4cpp::Category&  DataSourceLoader::logger = Logger::getCategory("smsc.db.DataSourceLoader");
Array<void *>       DataSourceLoader::handles;
Mutex               DataSourceLoader::loadupLock;

static DataSourceLoader _dataSourceLoader;

DataSourceLoader::~DataSourceLoader()
{
    unload();
}

void DataSourceLoader::loadupDataSourceFactory(
    const char* dlpath, const char* identity)
        throw(LoadupException)
{
    MutexGuard guard(loadupLock);
    
    logger.info("Loading '%s' library, identity is '%s' ...", dlpath, identity);
    void* dlhandle = dlopen(dlpath, RTLD_LAZY);
    if (dlhandle)
    {
        getDsfInstanceFn fnhandle = 
            (getDsfInstanceFn)dlsym(dlhandle, "getDataSourceFactory");
        if (fnhandle)
        {
            DataSourceFactory* dsf = (*fnhandle)();
            if (dsf)
            {
                DataSourceFactory::registerFactory(dsf, identity);
            }
            else 
            {
                logger.error("Load of '%s' library. Call to getDataSourceFactory() failed ! ", dlpath); 
                dlclose(dlhandle);
                throw LoadupException();
            }
        }
        else
        {
            logger.error("Load of '%s' library. Call to dlsym() failed ! ", dlpath); 
            dlclose(dlhandle);
            throw LoadupException();
        }
    }
    else
    {
        logger.error("Load of '%s' library. Call to dlopen() failed ! ", dlpath); 
        throw LoadupException();
    }
    (void)handles.Push(dlhandle);
    logger.info("Loading '%s' library done. Identity is '%s'.", dlpath, identity);
}

void DataSourceLoader::loadup(ConfigView* config)
    throw(ConfigException, LoadupException)
{
    __require__(config); // load up libraries by config
    
    std::auto_ptr<ConfigView> driversConfigGuard(config->getSubConfig("DataSourceDrivers"));
    ConfigView* driversConfig = driversConfigGuard.get();
    std::auto_ptr< std::set<std::string> > setGuard(driversConfig->getSectionNames());
    std::set<std::string>* set = setGuard.get();
    
    for (std::set<std::string>::iterator i=set->begin();i!=set->end();i++)
    {
        const char* section = (const char *)i->c_str();
        std::auto_ptr<ConfigView> driverConfigGuard(driversConfig->getSubConfig(section, true));
        ConfigView* driverConfig = driverConfigGuard.get();

        try
        {
            logger.info("Loading DataSourceDriver for section '%s'.", section);
            
            std::auto_ptr<char> typeGuard(driverConfig->getString("type"));
            const char* type = typeGuard.get();
            std::auto_ptr<char> dlpathGuard(driverConfig->getString("loadup"));
            const char* dlpath = dlpathGuard.get();
            loadupDataSourceFactory(dlpath, type);

            logger.info("Loaded DataSourceDriver for section '%s'. "
                        "Bind type is: %s. Imported library: '%s'", section, type, dlpath);
        }
        catch (ConfigException& exc)
        {
            logger.error("Loading of DataSourceDrivers failed ! Config exception: %s", exc.what());
            throw;
        }
    }
}

void DataSourceLoader::unload()
{
   MutexGuard guard(loadupLock);
   if (!handles.Count()) return;

   DataSourceFactory::unregisterFactories();

   logger.info("Unloading DataSourceDrivers ..."); 
   while (handles.Count())
   {
       void* handle=0L;
       (void) handles.Pop(handle);
       if (handle) dlclose(handle);
   }
   logger.info("DataSourceDrivers unloaded"); 
}

}}

