/* ************************************************************************** *
 * IAPManager: IN-Platforms configuration parsing.
 * ************************************************************************** */
#ifndef __INMAN_IAPMGR_SCF_CFG_PARSING__
#ident "@(#)$Id$"
#define __INMAN_IAPMGR_SCF_CFG_PARSING__

#include "inman/services/ICSCfgReader.hpp"
using smsc::inman::ICSMultiSectionCfgReaderAC_T;

#include "inman/services/iapmgr/IAPMgrDefs.hpp"

namespace smsc {
namespace inman {
namespace iapmgr {

class SCFsCfgReader : public ICSMultiSectionCfgReaderAC_T<SCFRegistry> {
private:
  typedef std::map<TDPCategory::Id, const char *> TDPNames;

  TDPNames    tdpNames;

protected:
  TDPCategory::Id str2tdp(const char * str);
  //
  bool str2UInt(uint32_t * p_val, const std::string & str);
  //Reads service key presented as VAL algorithm
  SKAlgorithmAC * readSkeyVal(TDPCategory::Id tdp_type, std::string & str);

  //Reads service key mapping data used in MAP algorithm
  bool readSKeyMap(SKAlgorithm_SKMap * alg, XConfigView & xlt_cfg);

  //Parses service key algorithm formatted as:
  //  "[algId :] algArg [: algParams] "
  SKAlgorithmAC * readSkeyAlg(XConfigView & scf_cfg, TDPCategory::Id tdp_type, const char * str);

  //Parses service keys definition section
  unsigned readSrvKeys(XConfigView & scf_cfg, SKAlgorithmMAP & sk_alg)
      throw(ConfigException);

  //Reads IN-platform configuration (not 'aliasFor' form)
  INScfCFG * readSCFCfg(XConfigView & cfg_sec,
                        const TonNpiAddress & scf_adr, const char * nm_scf)
      throw(ConfigException);

  // -- ----------------------------------------------
  // -- ICSMultiSectionCfgReaderAC_T interface methods
  // -- ----------------------------------------------
  CfgState parseSection(XConfigView * cfg_sec, const std::string & nm_sec,
                        void * opaque_arg = NULL)
      throw(ConfigException);

public:
  SCFsCfgReader(Config & root_sec, Logger * use_log, const char * ics_sec = NULL)
    : ICSMultiSectionCfgReaderAC_T<SCFRegistry>(root_sec, use_log,
                                  ics_sec ? ics_sec : "IN-platforms")
  {
    tdpNames[TDPCategory::dpMO_BC] = TDPCategory::Name(TDPCategory::dpMO_BC);
    tdpNames[TDPCategory::dpMO_SM] = TDPCategory::Name(TDPCategory::dpMO_SM);
  }
  ~SCFsCfgReader()
  { }
  //NOTE: opaque_arg type is: INScfsMAP *
  //NOTE: CfgParsingResult.opaqueRes type is: INScfCFG *
};


} //iapmgr
} //inman
} //smsc
#endif /* __INMAN_IAPMGR_SCF_CFG_PARSING__ */

