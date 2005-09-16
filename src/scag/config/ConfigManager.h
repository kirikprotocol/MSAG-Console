#ifndef ConfigManager_dot_h
#define ConfigManager_dot_h

#include "scag/config/alias/aliasconf.h"
#include "scag/config/smppman/SmppManConfig.h"
#include "scag/config/route/RouteConfig.h"
#include "scag/config/statman/StatManConfig.h"

#include <string>

namespace scag {
namespace config {

enum ConfigType 
{
	SMPPMAN_CFG,
	ROUTE_CFG,
	ALIAS_CFG,
    STATMAN_CFG
};

class ConfigManager
{
public:
	static ConfigManager& Instance();
    static void Init();

	virtual void reloadConfig(ConfigType type) = 0;
	virtual void reloadAllConfigs() = 0;

    virtual AliasConfig getAliasConfig() = 0;
    virtual SmppManConfig getSmppManConfig() = 0;
    virtual RouteConfig getRouteConfig() = 0;
    virtual StatManConfig getStatManConfig() = 0;
    virtual Hash<std::string>*& getLicConfig() = 0;
    virtual Config* getConfig() = 0;
};

}
}

#endif
