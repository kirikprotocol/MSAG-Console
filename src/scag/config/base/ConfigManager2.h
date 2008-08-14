#ifndef ConfigManager2_dot_h
#define ConfigManager2_dot_h

#include "ConfigType.h"
#include "scag/config/smpp/SmppManConfig.h"
#include "scag/config/route/RouteConfig.h"
#include "scag/config/stat/StatManConfig.h"
#include "scag/config/bill/BillingManagerConfig.h"
#include "scag/config/sessn/SessionManagerConfig2.h"
#include "scag/config/http/HttpManagerConfig.h"
#include "scag/config/pers/PersClientConfig.h"
#include "scag/config/lcm/LongCallManagerConfig.h"
#include "ConfigListener2.h"

#include <string>

namespace scag2 {
namespace config {

    using namespace scag::config;

struct LicenseInfo{
  LicenseInfo()
  {
    maxsms=0;
    maxhttp=0;
    maxmms=0;
    expdate=0;
  }
  int maxsms;
  int maxhttp;
  int maxmms;
  time_t expdate;
};


class ConfigManager
{
public:
    static ConfigManager& Instance();
    // static void Init();

    virtual ~ConfigManager();

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
    virtual LicenseInfo& getLicense()=0;
    virtual void checkLicenseFile()=0;
    
    virtual Config* getConfig() = 0;

protected:
    ConfigManager();

    friend class ConfigListener;
    virtual void registerListener( ConfigType t, ConfigListener* l ) = 0;
    virtual void removeListener( ConfigType t ) = 0;

private:
    ConfigManager(const ConfigManager& bm);
    ConfigManager& operator=(const ConfigManager& bm);
};

}
}

#endif
