/* ************************************************************************** *
 * TCAP Dispatcher service config file parsing.
 * ************************************************************************** */
#ifndef __INMAN_IAPRVD_SRI_CONFIG_PARSING__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_IAPRVD_SRI_CONFIG_PARSING__

#include "inman/services/ICSCfgReader.hpp"
#include "inman/services/iapSRI/IAPrvdSRIDefs.hpp"

namespace smsc {
namespace inman {
namespace iaprvd {
namespace sri {

using smsc::inman::ICSrvCfgReaderAC_T;
using smsc::inman::XMFConfig;

class ICSIAPrvdSRICfgReader : public ICSrvCfgReaderAC_T<IAProviderSRI_XCFG> {
protected:
  // --------------------------------------------------
  // ICSrvCfgReaderAC interface methods implementation
  // --------------------------------------------------
  //Parses XML configuration entry section, updates dependencies.
  //Returns status of config parsing
  virtual CfgState parseConfig(void * opaque_arg = NULL) throw(ConfigException);

public:
  static const int32_t    _MAX_THREADS_NUM = 0xFFFF;
  static const uint16_t   _DFLT_THREADS_NUM = 32;

  ICSIAPrvdSRICfgReader(XMFConfig & xmf_cfg, Logger * use_log, const char * ics_sec)
    : ICSrvCfgReaderAC_T<IAProviderSRI_XCFG>(xmf_cfg, use_log, ics_sec)
  {
    //unconditional dependencies:
    icsDeps.insert(ICSIdent::icsIdTCAPDisp);
  }
  ~ICSIAPrvdSRICfgReader()
  { }

  IAProviderSRI_XCFG * rlseConfig(void)
  {
    icsDeps.exportDeps(icsCfg->_deps);
    return icsCfg.release();
  }
};

} //sri
} //iaprvd
} //inman
} //smsc
#endif /* __INMAN_IAPRVD_SRI_CONFIG_PARSING__ */

