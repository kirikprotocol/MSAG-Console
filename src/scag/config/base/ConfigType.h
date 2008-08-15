#ifndef _SCAG_CONFIG_CONFIGTYPE_H
#define _SCAG_CONFIG_CONFIGTYPE_H

namespace scag {
namespace config {

typedef enum ConfigType 
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
} ConfigType;

} // namespace config
} // namespace scag

namespace scag2 {
namespace config {

using scag::config::ConfigType;
using scag::config::SMPPMAN_CFG   ;
using scag::config::ROUTE_CFG     ;
using scag::config::ALIAS_CFG     ;
using scag::config::STATMAN_CFG   ;
using scag::config::BILLMAN_CFG   ;
using scag::config::SESSIONMAN_CFG;
using scag::config::HTTPMAN_CFG   ;
using scag::config::PERSCLIENT_CFG;
using scag::config::LONGCALLMAN_CFG;

} // namespace config
} // namespace scag

#endif /* !_SCAG_CONFIG_CONFIGTYPE_H */
