#ifndef ConfigManager_dot_h
#define ConfigManager_dot_h

#include "scag/config/smpp/SmppManConfig.h"
#include "scag/config/route/RouteConfig.h"
#include "scag/config/stat/StatManConfig.h"
#include "scag/config/bill/BillingManagerConfig.h"
#include "scag/config/sessn/SessionManagerConfig.h"
#include "scag/config/http/HttpManagerConfig.h"
#include "scag/config/pers/PersClientConfig.h"
#include "scag/config/lcm/LongCallManagerConfig.h"

#include <string>

namespace scag {
namespace config {

enum ConfigType 
{
    SMPPMAN_CFG,
    ROUTE_CFG,
    ALIAS_CFG,
    STATMAN_CFG,
    BILLMAN_CFG,
    SESSIONMAN_CFG,
    HTTPMAN_CFG,
    PERSCLIENT_CFG,
    LONGCALLMAN_CFG
};

class ConfigManager
{
    ConfigManager(const ConfigManager& bm);
    ConfigManager& operator=(const ConfigManager& bm);
protected:
        ConfigManager() {};
        virtual ~ConfigManager() {};
public:
    static ConfigManager& Instance();
    static void Init();

    virtual void reloadConfig(ConfigType type) = 0;
    virtual void reloadAllConfigs() = 0;

    virtual SmppManConfig& getSmppManConfig() = 0;
    virtual RouteConfig& getRouteConfig() = 0;
    virtual StatManConfig& getStatManConfig() = 0;
    virtual BillingManagerConfig& getBillManConfig() = 0;
    virtual SessionManagerConfig& getSessionManConfig() = 0;
    virtual HttpManagerConfig& getHttpManConfig() = 0;    
    virtual PersClientConfig& getPersClientConfig() = 0;    
    virtual LongCallManagerConfig& getLongCallManConfig() = 0;
    virtual Hash<std::string>*& getLicConfig() = 0;
    virtual Config* getConfig() = 0;
};

}
}

#endif
