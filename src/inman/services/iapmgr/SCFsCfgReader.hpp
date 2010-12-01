/* ************************************************************************** *
 * IAPManager: IN-Platforms configuration parsing.
 * ************************************************************************** */
#ifndef __INMAN_IAPMGR_SCF_CFG_PARSING__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_IAPMGR_SCF_CFG_PARSING__

#include "inman/services/ICSCfgReader.hpp"
#include "inman/services/iapmgr/SCFRegistry.hpp"

namespace smsc {
namespace inman {
namespace iapmgr {

using smsc::inman::ICSMultiSectionCfgReaderAC_T;
using smsc::inman::XMFConfig;

class SCFsCfgReader : public ICSMultiSectionCfgReaderAC_T<SCFRegistry> {
protected:
  //
  bool str2UInt(uint32_t * p_val, const std::string & str);
  //Reads service key presented as VAL algorithm
  SKAlgorithmAC * readSkeyVal(CSIUid_e tdp_type, std::string & str);

  //Reads service key mapping data used in MAP algorithm
  bool readSKeyMap(SKAlgorithm_SKMap * alg, XConfigView & xlt_cfg);

  //Parses service key algorithm formatted as:
  //  "[algId :] algArg [: algParams] "
  SKAlgorithmAC * readSkeyAlg(XConfigView & scf_cfg, CSIUid_e tdp_type, const char * str);

  //Parses service keys definition section
  unsigned readSrvKeys(XConfigView & scf_cfg, SKAlgorithmsDb & sk_alg)
      throw(ConfigException);

  //Reads IN-platform configuration parameters
  void readSCFParms(const char * nm_sec, XConfigView & cfg_sec, INScfParams & in_cfg)
      throw(ConfigException);

  // -- ----------------------------------------------
  // -- ICSMultiSectionCfgReaderAC_T interface methods
  // -- ----------------------------------------------
  virtual ICSrvCfgReaderAC::CfgState
    parseSection(XConfigView & cfg_sec, const std::string & nm_sec,
                 void * opaque_arg = NULL) throw(ConfigException);

public:
  SCFsCfgReader(XMFConfig & xmf_cfg, Logger * use_log, const char * ics_sec = NULL)
    : ICSMultiSectionCfgReaderAC_T<SCFRegistry>(xmf_cfg, use_log,
                                                ics_sec ? ics_sec : "IN-platforms")
  { }
  ~SCFsCfgReader()
  { }
};


} //iapmgr
} //inman
} //smsc
#endif /* __INMAN_IAPMGR_SCF_CFG_PARSING__ */

