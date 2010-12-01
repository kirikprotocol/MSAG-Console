/* ************************************************************************** *
 * IAProvider utilizing MAP service ATSI config file parsing.
 * ************************************************************************** */
#ifndef __INMAN_IAPRVD_ATSI_CONFIG_PARSING__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_IAPRVD_ATSI_CONFIG_PARSING__

#include "inman/services/ICSCfgReader.hpp"
#include "inman/services/iapATSI/IAPrvdATSIDefs.hpp"

namespace smsc {
namespace inman {
namespace iaprvd {
namespace atih {

using smsc::inman::ICSrvCfgReaderAC_T;
using smsc::inman::XMFConfig;

class ICSIAPrvdATSICfgReader : public ICSrvCfgReaderAC_T<IAProviderATSI_XCFG> {
protected:
  // --------------------------------------------------
  // ICSrvCfgReaderAC interface methods implementation
  // --------------------------------------------------
  //Parses XML configuration entry section, updates dependencies.
  //Returns status of config parsing
  virtual CfgState parseConfig(void * opaque_arg = NULL) throw(ConfigException);

public:
  ICSIAPrvdATSICfgReader(XMFConfig & xmf_cfg, Logger * use_log, const char * ics_sec)
    : ICSrvCfgReaderAC_T<IAProviderATSI_XCFG>(xmf_cfg, use_log, ics_sec)
  {
    //unconditional dependencies:
    icsDeps.insert(ICSIdent::icsIdTCAPDisp);
  }
  ~ICSIAPrvdATSICfgReader()
  { }

  IAProviderATSI_XCFG * rlseConfig(void)
  {
    icsDeps.exportDeps(icsCfg->deps);
    return icsCfg.release();
  }
};

} //atih
} //iaprvd
} //inman
} //smsc
#endif /* __INMAN_IAPRVD_ATSI_CONFIG_PARSING__ */

