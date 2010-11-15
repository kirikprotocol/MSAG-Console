/* ************************************************************************** *
 * IAPManager: abonent providers configuration parsing.
 * ************************************************************************** */
#ifndef __INMAN_IAPMGR_PRVD_CFG_PARSING
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_IAPMGR_PRVD_CFG_PARSING

#include "inman/services/ICSCfgReader.hpp"
#include "inman/services/iapmgr/IAPrvdsRegistry.hpp"

namespace smsc {
namespace inman {
namespace iapmgr {

using smsc::inman::ICSMultiSectionCfgReaderAC_T;

class ICSProvidersCfgReader : public ICSMultiSectionCfgReaderAC_T<IAPrvdsRegistry> {
protected:
  // -- ----------------------------------------------
  // -- ICSMultiSectionCfgReaderAC_T interface methods
  // -- ----------------------------------------------
  ICSrvCfgReaderAC::CfgState 
    parseSection(XConfigView * cfg_sec, const std::string & nm_sec, void * opaque_arg = NULL)
      throw(ConfigException);

public:
  ICSProvidersCfgReader(Config & root_sec, Logger * use_log, const char * ics_sec = NULL)
      : ICSMultiSectionCfgReaderAC_T<IAPrvdsRegistry>(root_sec, use_log,
                                      ics_sec ? ics_sec : "AbonentProviders")
  { }
  ~ICSProvidersCfgReader()
  { }
};

} //iapmgr
} //inman
} //smsc
#endif /* __INMAN_IAPMGR_PRVD_CFG_PARSING */

