#ifndef ConfigManager2impl_dot_h
#define ConfigManager2impl_dot_h

#include <xercesc/dom/DOM.hpp>
#include "core/buffers/Array.hpp"
#include "scag/config/base/ConfigListener2.h"
#include "scag/config/base/ConfigManager2.h"

namespace scag2 {
namespace config {

class ConfigManagerImpl : public ConfigManager
{
protected:
    virtual ~ConfigManagerImpl() {
        smsc_log_info(logger, "ConfigManager released");
    }
public:
    ConfigManagerImpl() throw(ConfigException);
    static void Init() throw(ConfigException);

    virtual void registerListener(ConfigType type, ConfigListener* listener);
    virtual void removeListener(ConfigType type);

    virtual void reloadConfig(ConfigType type);
    virtual void reloadAllConfigs();
    virtual RouteConfig&  getRouteConfig(){ return getRouteConfig_(); }
    virtual SmppManConfig& getSmppManConfig(){ return getSmppManConfig(); }
    virtual StatManConfig& getStatManConfig(){ return getStatManConfig_(); }
    virtual BillingManagerConfig& getBillManConfig(){ return getBillManConfig_(); }
    virtual SessionManagerConfig& getSessionManConfig(){ return getSessionManConfig_(); }
    virtual HttpManagerConfig& getHttpManConfig(){ return getHttpManConfig_(); }  
    virtual PersClientConfig& getPersClientConfig(){ return getPersClientConfig_(); }
    virtual LongCallManagerConfig& getLongCallManConfig(){ return getLongCallManConfig_(); }

    virtual LicenseInfo& getLicense(){return license;}
    virtual void checkLicenseFile();
    virtual Config* getConfig(){return &config;};

protected:
    static RouteConfig&  getRouteConfig_()
    {
        static RouteConfig routeCfg;
        return routeCfg;
    }
    static SmppManConfig& getSmppManConfig_()
    {
        static SmppManConfig smppManCfg;
        return smppManCfg;
    }
    static StatManConfig& getStatManConfig_()
    {
        static StatManConfig statManCfg;
        return statManCfg;
    }
    static BillingManagerConfig& getBillManConfig_()
    {
        static BillingManagerConfig billManCfg;
        return billManCfg;
    }
    static SessionManagerConfig& getSessionManConfig_()
    {
        static SessionManagerConfig sessionManCfg;
        return sessionManCfg;
    };
    static HttpManagerConfig& getHttpManConfig_()
    {
        static HttpManagerConfig httpManCfg;
        return httpManCfg;
    };
    static PersClientConfig& getPersClientConfig_()
    {
        static PersClientConfig persCfg;
        return persCfg;
    };
    static LongCallManagerConfig& getLongCallManConfig_()
    {
        static LongCallManagerConfig cfg;
        return cfg;
    };

    void save();

protected:
private:

    static void findConfigFile();

protected:
    static Config config;

private:
    static std::auto_ptr<char> config_filename;
    void writeHeader(std::ostream &out);
    void writeFooter(std::ostream &out);
    void reload( smsc::core::buffers::Array<int>& changedConfigs ) throw(ConfigException);
    smsc::core::buffers::IntHash<ConfigListener*> listeners;
    smsc::core::synchronization::Mutex listenerLock;
    smsc::logger::Logger* logger;

    static smsc::core::buffers::Hash<std::string> licconfig;
    static LicenseInfo license;
    static time_t licenseFileMTime;
    static std::string licenseFile;
    static std::string licenseSig;

    xercesc::DOMDocument * parse( const char * const filename ) throw (ConfigException);
};

}
}

#endif
