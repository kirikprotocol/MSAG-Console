#ifndef SMSC_INFO_SME_DATA_PROVIDER
#define SMSC_INFO_SME_DATA_PROVIDER

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <logger/Logger.h>
#include <core/buffers/Array.hpp>
#include <core/buffers/Hash.hpp>

#include <util/config/Config.h>
#include <util/config/Manager.h>
#include <util/config/ConfigView.h>
#include <util/config/ConfigException.h>

#include <core/threads/Thread.hpp>
#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>

#include <db/DataSource.h>

namespace smsc { namespace infosme 
{
    using namespace smsc::core::buffers;
    using namespace smsc::db;
    
    using smsc::core::synchronization::Mutex;
    
    using smsc::util::Logger;
    using smsc::util::config::Manager;
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;

    
    class DataProvider
    {
    private:

        log4cpp::Category  &logger;

        Hash<DataSource *>  dss;
        Mutex               dssLock;
        
        DataSource* createDataSource(ConfigView* config)
            throw(ConfigException);
    
    public:

        DataProvider()
            : logger(Logger::getCategory("smsc.infosme.DataProvider")) {};
        virtual ~DataProvider();

        /**
         * Initializes DataProvider, loads up all specified DataSources 
         *
         * @param config
         * @exception ConfigException throws when configuration is invalid
         */
        void init(ConfigView* config)
            throw(ConfigException);
        
        DataSource* getDataSource(const char* dsid)
        {
            MutexGuard guard(dssLock);
            return ((dss.Exists(dsid)) ? dss.Get(dsid):0);
        }
    };

}}

#endif //SMSC_INFO_SME_DATA_PROVIDER
