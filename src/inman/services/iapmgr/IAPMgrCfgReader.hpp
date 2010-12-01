/* ************************************************************************** *
 * IAPManager (abonent policies/providers) service config file parsing.
 * ************************************************************************** */
#ifndef __INMAN_IAPMGR_CFG_PARSING__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_IAPMGR_CFG_PARSING__

#include "inman/services/ICSCfgReader.hpp"
#include "inman/services/iapmgr/IAPMgrCfg.hpp"
#include "inman/services/iapmgr/IAPrvdsCfgReader.hpp"
#include "inman/services/iapmgr/SCFsCfgReader.hpp"

namespace smsc {
namespace inman {
namespace iapmgr {

using smsc::inman::ICSMultiSectionCfgReaderAC_T;
using smsc::inman::XMFConfig;


class ICSIAPMgrCfgReader : public ICSMultiSectionCfgReaderAC_T<IAPManagerCFG> {
private:
  SCFsCfgReader         scfReader;    //Parses IN-platforms config
  ICSProvidersCfgReader prvdReader;   //Parses IAProviders config

protected:
  // -- ----------------------------------------------
  // -- ICSMultiSectionCfgReaderAC_T interface methods
  // -- ----------------------------------------------
  virtual ICSrvCfgReaderAC::CfgState
    parseSection(XConfigView & cfg_sec, const std::string & nm_sec,
                void * opaque_arg = NULL) throw(ConfigException);

public:
  ICSIAPMgrCfgReader(XMFConfig & xmf_cfg, Logger * use_log, const char * ics_sec = NULL)
    : ICSMultiSectionCfgReaderAC_T<IAPManagerCFG>(xmf_cfg, use_log,
                                      ics_sec ? ics_sec : "AbonentPolicies")
    , scfReader(xmf_cfg, use_log), prvdReader(xmf_cfg, use_log)
  { }
  ~ICSIAPMgrCfgReader()
  { }

  virtual IAPManagerCFG * rlseConfig(void);
};

} //iapmgr
} //inman
} //smsc
#endif /* __INMAN_IAPMGR_CFG_PARSING__ */

