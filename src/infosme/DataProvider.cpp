
#include "DataProvider.h"

namespace smsc { namespace infosme 
{

DataProvider::~DataProvider()
{
    MutexGuard guard(dssLock);
    
    char* key = 0; DataSource* ds = 0; dss.First();
    while (dss.Next(key, ds))
        if (ds) delete ds;
}

DataSource* DataProvider::createDataSource(ConfigView* config)
{
    DataSource* ds   = 0;
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
    return ds;
}

void DataProvider::init(ConfigView* config)
{
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
            logger.info("Loading DataSource '%s'.", dsName);
            
            std::auto_ptr<ConfigView> dsConfigGuard(config->getSubConfig(dsName));
            ConfigView* dsConfig = dsConfigGuard.get();
            if (dss.Exists(dsName))
                throw ConfigException("DataSource '%s' already registered", dsName);

            DataSource* ds = createDataSource(dsConfig);
            if (!ds)
                throw ConfigException("Failed to create DataSource '%s'", dsName);

            dss.Insert(dsName, ds);
            logger.info("Loaded DataSource '%s'.", dsName);
        }
        catch (ConfigException& exc)
        {
            logger.error("Load of DataProvider failed ! Config exception: %s", exc.what());
            throw;
        }
    }
}

}}
