/* ************************************************************************** *
 * TCAP Dispatcher service config file parsing.
 * ************************************************************************** */
#ifndef __INMAN_TCDSP_CONFIG_PARSING__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_TCDSP_CONFIG_PARSING__

#include "inman/inap/xcfg/TCDspCfgParser.hpp"
#include "inman/services/ICSCfgReader.hpp"

#ifdef EIN_HD
#define NM_SS7_CFG_SECTION "SS7_HD"
#else /* EIN_HD */
#define NM_SS7_CFG_SECTION "SS7"
#endif /* EIN_HD */

namespace smsc {
namespace inman {
namespace inap {

using smsc::inman::ICSrvCfgReaderAC_T;
using smsc::inman::XMFConfig;

class ICSTCDispCfgReader : public ICSrvCfgReaderAC_T<TCDsp_CFG> {
protected:
  //Parses XML configuration entry section, updates dependencies.
  //Returns status of config parsing, 
  CfgState parseConfig(void * opaque_arg = NULL) throw(ConfigException)
  {
      TCDspCfgParser  parser(logger, nmCfgSection());
      parser.readConfig(_topSec.relConfig(), *icsCfg.get()); //throws
      /**/
      return ICSrvCfgReaderAC::cfgComplete;
  }

public:
  ICSTCDispCfgReader(XMFConfig & xmf_cfg, Logger * use_log, const char * ics_sec = NULL)
    : ICSrvCfgReaderAC_T<TCDsp_CFG>(xmf_cfg, use_log, ics_sec ? ics_sec : NM_SS7_CFG_SECTION)
  { }
  ~ICSTCDispCfgReader()
  { }
};

} //inap
} //inman
} //smsc
#endif /* __INMAN_TCDSP_CONFIG_PARSING__ */

