
#include <dlfcn.h>

#include "DataSourceLoader.h"

namespace smsc { namespace db
{
typedef DataSourceFactory* (*getDsfInstanceFn)(void);

using smsc::util::Logger;

/* --------- Abstract Connection Management (ConnectionPool) --------------- */

Array<void *>       DataSourceLoader::handles;
log4cpp::Category&  DataSourceLoader::log 
    = Logger::getCategory("smsc.db.DataSourceLoader");

static DataSourceLoader _dataSourceLoader;

DataSourceLoader::~DataSourceLoader()
{
    while (handles.Count())
    {
        void* handle=0L;
        (void) handles.Pop(handle);
        if (handle) dlclose(handle);
    }
}

void DataSourceLoader::loadupDataSourceFactory(
    const char* dlpath, const char* identity)
        throw(LoadupException)
{
    log.info("Loading '%s' library, identity is '%s' ...", dlpath, identity);
    
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
                log.error("Load of '%s' library."
                          "Call to getDataSourceFactory() failed ! ", dlpath); 
                dlclose(dlhandle);
                throw LoadupException();
            }
        }
        else
        {
            log.error("Load of '%s' library."
                      "Call to dlsym() failed ! ", dlpath); 
            dlclose(dlhandle);
            throw LoadupException();
        }
    }
    else
    {
        log.error("Load of '%s' library."
                  "Call to dlopen() failed ! ", dlpath); 
        throw LoadupException();
    }
    
    (void)handles.Push(dlhandle);
    log.info("Loading '%s' library done. Identity is '%s'.", dlpath, identity);
}

void DataSourceLoader::loadup(ConfigView* config)
    throw(ConfigException, LoadupException)
{
    __require__(config);
    
    // load up libraries by config
    ConfigView* driversConfig = config->getSubConfig("DataSourceDrivers");
    std::set<std::string>* set = driversConfig->getSectionNames();
    
    for (std::set<std::string>::iterator i=set->begin();i!=set->end();i++)
    {
        const char* section = (const char *)i->c_str();
        ConfigView* driverConfig = 
            driversConfig->getSubConfig(section, true);
        const char* type = 0;
        const char* dlpath = 0;
        try
        {
            log.info("Loading DataSourceDriver for section '%s'.", section);
            
            type = driverConfig->getString("type");
            dlpath = driverConfig->getString("loadup");
            loadupDataSourceFactory(dlpath, type);

            log.info("Loaded DataSourceDriver for section '%s'."
                     " Bind type is: %s. Imported library: '%s'",
                      section, type, dlpath);
        }
        catch (ConfigException& exc)
        {
            log.error("Loading of DataSourceDrivers failed !"
                      " Config exception: %s", exc.what());
            if (set) delete set;
            if (type) delete type;
            if (dlpath) delete dlpath;
            delete driversConfig;
            delete driverConfig;
            throw;
        }
        if (type) delete type;
        if (dlpath) delete dlpath;
        delete driverConfig;
    }
    if (set) delete set;
    delete driversConfig;
}

}}

