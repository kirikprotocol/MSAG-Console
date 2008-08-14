#ifndef _SCAG_CONFIG_CONFIGTYPE_H
#define _SCAG_CONFIG_CONFIGTYPE_H

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

} // namespace config
} // namespace scag

#endif /* !_SCAG_CONFIG_CONFIGTYPE_H */
