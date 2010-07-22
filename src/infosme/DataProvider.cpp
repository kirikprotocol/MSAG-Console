
#include "DataProvider.h"

using smsc::db::DataSource;

namespace smsc {
namespace infosme {

DataProvider::~DataProvider()
{
#ifndef INFOSME_NO_DATAPROVIDER
    MutexGuard guard(dssLock);
    
    char* key = 0; DataSource* ds = 0; dss.First();
    while (dss.Next(key, ds))
        if (ds) delete ds;
#endif
}

DataSource* DataProvider::createDataSource(ConfigView* config)
{
    DataSource* ds   = 0;
#ifndef INFOSME_NO_DATAPROVIDER
    try 
    {
        std::auto_ptr<char> dsIdentity(config->getString("type"));
        const char* dsIdentityStr = dsIdentity.get();
        ds = DataSourceFactory::getDataSource(dsIdentityStr);
        if (ds) ds->init(config);
        else throw ConfigException("DataSource for '%s' identity "
                                   "wasn't registered !", dsIdentityStr);
    }
    catch (ConfigException& exc)
    {
        if (ds) delete ds;
        throw;
    }
#endif
    return ds;
}

void DataProvider::init(ConfigView* config)
{
#ifndef INFOSME_NO_DATAPROVIDER
    MutexGuard guard(dssLock);
    
    std::auto_ptr< std::set<std::string> > setGuard(config->getShortSectionNames());
    std::set<std::string>* set = setGuard.get();
    
    for (std::set<std::string>::iterator i=set->begin();i!=set->end();i++)
    {
        try
        {
            const char* dsName = (const char *)i->c_str();
            if (!dsName)
                throw ConfigException("DataSource name missed!");
            smsc_log_info(logger, "Loading DataSource '%s'.", dsName);
            
            std::auto_ptr<ConfigView> dsConfigGuard(config->getSubConfig(dsName));
            ConfigView* dsConfig = dsConfigGuard.get();
            if (dss.Exists(dsName))
                throw ConfigException("DataSource '%s' already registered", dsName);

            DataSource* ds = createDataSource(dsConfig);
            if (!ds)
                throw ConfigException("Failed to create DataSource '%s'", dsName);

            dss.Insert(dsName, ds);
            smsc_log_info(logger, "Loaded DataSource '%s'.", dsName);
        }
        catch (ConfigException& exc)
        {
            smsc_log_error(logger, "Load of DataProvider failed ! Config exception: %s", exc.what());
            throw;
        }
    }
#endif
}



DataSource* DataProvider::getDataSource(const char* dsid)
{
#ifndef INFOSME_NO_DATAPROVIDER
    MutexGuard guard(dssLock);
    return ((dss.Exists(dsid)) ? dss.Get(dsid):0);
#else
    return 0;
#endif
}

}
}
