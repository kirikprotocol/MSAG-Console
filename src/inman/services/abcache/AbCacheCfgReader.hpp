/* ************************************************************************** *
 * Abonents Cache service config file parsing.
 * ************************************************************************** */
#ifndef __INMAN_ABCACHE_CONFIG_PARSING__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ABCACHE_CONFIG_PARSING__

#include "inman/incache/AbCacheDefs.hpp"

#include "inman/services/ICSCfgReader.hpp"

namespace smsc {
namespace inman {
namespace cache {

using smsc::inman::ICSrvCfgReaderAC_T;
using smsc::inman::XMFConfig;

class ICSAbCacheCfgReader : public ICSrvCfgReaderAC_T<AbonentCacheCFG> {
protected:
  // --------------------------------------
  // ICSrvCfgReaderAC interface methods
  // ---------------------------------------
  //Parses XML configuration entry section, updates dependencies.
  //Returns status of config parsing, 
  virtual CfgState parseConfig(void * opaque_arg = NULL) throw(ConfigException);

public:
  ICSAbCacheCfgReader(XMFConfig & xmf_cfg, Logger * use_log, const char * ics_sec = NULL)
    : ICSrvCfgReaderAC_T<AbonentCacheCFG>(xmf_cfg, use_log, ics_sec ? ics_sec : "AbonentsCache")
  { }
  ~ICSAbCacheCfgReader()
  { }
};

} //cache
} //inman
} //smsc
#endif /* __INMAN_ABCACHE_CONFIG_PARSING__ */

