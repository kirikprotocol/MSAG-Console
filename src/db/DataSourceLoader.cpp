
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

}}

