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
    
    using smsc::logger::Logger;
    using smsc::util::config::Manager;
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;

    
    class DataProvider
    {
    private:

        smsc::logger::Logger *logger;

        Hash<DataSource *>   dss;
        Mutex                dssLock;
    
    public:

        DataProvider()
            : logger(Logger::getInstance("smsc.infosme.DataProvider")) {};
        virtual ~DataProvider();

        /**
         * Initializes DataProvider, loads up all specified DataSources 
         *
         * @param config
         * @exception ConfigException throws when configuration is invalid
         */
        void init(ConfigView* config);
        
        DataSource* createDataSource(ConfigView* config);

        DataSource* getDataSource(const char* dsid)
        {
            MutexGuard guard(dssLock);
            return ((dss.Exists(dsid)) ? dss.Get(dsid):0);
        }
    };

}}

#endif //SMSC_INFO_SME_DATA_PROVIDER
