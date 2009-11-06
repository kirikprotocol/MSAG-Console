#ifndef SMSC_INFOSME2_DATAPROVIDER_H
#define SMSC_INFOSME2_DATAPROVIDER_H

#include "logger/Logger.h"
#include "core/buffers/Hash.hpp"
#include "core/synchronization/Mutex.hpp"

/*
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <core/buffers/Array.hpp>

#include <util/config/Config.h>
#include <util/config/Manager.h>
#include <util/config/ConfigView.h>
#include <util/config/ConfigException.h>

#include <core/threads/Thread.hpp>
#include <core/synchronization/Event.hpp>

#include <db/DataSource.h>
 */

namespace smsc {
namespace db { class DataSource; }
namespace util { namespace config { class ConfigView; } }
namespace infosme2 {

class DataProvider
{
public:

    DataProvider();
    // : logger(Logger::getInstance("smsc.infosme.DataProvider")) {};
    ~DataProvider();

    /**
     * Initializes DataProvider, loads up all specified DataSources 
     *
     * @param config
     * @exception ConfigException throws when configuration is invalid
     */
    void init( smsc::util::config::ConfigView* config );
        
    smsc::db::DataSource* getDataSource( const char* dsid );
    /*
    {
        MutexGuard guard(dssLock);
        return ((dss.Exists(dsid)) ? dss.Get(dsid):0);
    }
     */

private:
    smsc::db::DataSource* createDataSource(smsc::util::config::ConfigView* config);

private:
    smsc::logger::Logger*                              log_;
    smsc::core::synchronization::Mutex                 lock_;
    smsc::core::buffers::Hash< smsc::db::DataSource *> dss_;
};

}
}

#endif //SMSC_INFO_SME_DATA_PROVIDER
