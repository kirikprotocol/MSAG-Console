#ifndef SMSC_DB_DATASOURCE_LOADER
#define SMSC_DB_DATASOURCE_LOADER

/**
 * 
 * @author Victor V. Makarov
 * @version 1.0
 * @see 
 */

#include "DataSource.h"

namespace smsc { namespace db
{
    using smsc::util::Logger;
    using smsc::core::buffers::Array;

    class DataSourceLoader
    {
    private:

        static log4cpp::Category    &logger;
        static Array<void *>        handles;
        static Mutex                loadupLock;

    public:

        DataSourceLoader() {};
        virtual ~DataSourceLoader();

        static void loadup(ConfigView* config)
            throw(ConfigException, LoadupException);
        static void unload();

        static void loadupDataSourceFactory(
            const char* dlpath, const char* identity)
                throw(LoadupException);
    };

}}

#endif
