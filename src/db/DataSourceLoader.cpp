
#include <dlfcn.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "DataSourceLoader.h"

namespace smsc { namespace db
{
typedef DataSourceFactory* (*getDsfInstanceFn)(void);

using smsc::logger::Logger;

smsc::logger::Logger *DataSourceLoader::logger = 0;
Array<void *>         DataSourceLoader::handles;
Mutex                 DataSourceLoader::loadupLock;

DataSourceLoader::~DataSourceLoader()
{
    //unload();
}

void DataSourceLoader::loadupDataSourceFactory(
    const char* dlpath, const char* identity)
        throw(LoadupException)
{
    MutexGuard guard(loadupLock);

    if (!logger)
      logger = Logger::getInstance("smsc.db.DataSourceLoader");
    smsc_log_info(logger, "Loading '%s' library, identity is '%s' ...", dlpath, identity);
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
                smsc_log_error(logger, "Load of '%s' library. Call to getDataSourceFactory() failed ! ", dlpath);
                dlclose(dlhandle);
                throw LoadupException();
            }
        }
        else
        {
            smsc_log_error(logger, "Load of '%s' library. Call to dlsym() failed ! ", dlpath);
            dlclose(dlhandle);
            throw LoadupException();
        }
    }
    else
    {
        char buf[256];
        smsc_log_error(logger, "Load of '%s' at '%s' library. Call to dlopen() failed:%s ! ", dlpath,getcwd(buf,sizeof(buf)),strerror(errno));
        throw LoadupException();
    }
    (void)handles.Push(dlhandle);
    smsc_log_info(logger, "Loading '%s' library done. Identity is '%s'.", dlpath, identity);
}

void DataSourceLoader::loadup(ConfigView* config)
    throw(ConfigException, LoadupException)
{
    __require__(config); // load up libraries by config
    if (!logger)
      logger = Logger::getInstance("smsc.db.DataSourceLoader");

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
            smsc_log_info(logger, "Loading DataSourceDriver for section '%s'.", section);

            std::auto_ptr<char> typeGuard(driverConfig->getString("type"));
            const char* type = typeGuard.get();
            std::auto_ptr<char> dlpathGuard(driverConfig->getString("loadup"));
            const char* dlpath = dlpathGuard.get();
            loadupDataSourceFactory(dlpath, type);

            smsc_log_info(logger, "Loaded DataSourceDriver for section '%s'. "
                        "Bind type is: %s. Imported library: '%s'", section, type, dlpath);
        }
        catch (ConfigException& exc)
        {
            smsc_log_error(logger, "Loading of DataSourceDrivers failed ! Config exception: %s", exc.what());
            throw;
        }
    }
}

void DataSourceLoader::unload()
{
   MutexGuard guard(loadupLock);
   if (!handles.Count()) return;

   DataSourceFactory::unregisterFactories();

   smsc_log_info(logger, "Unloading DataSourceDrivers ...");
   while (handles.Count())
   {
       void* handle=0L;
       (void) handles.Pop(handle);
       if (handle) dlclose(handle);
   }
   smsc_log_info(logger, "DataSourceDrivers unloaded");
}

}}
