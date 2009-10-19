/* ************************************************************************** *
 * IAPManager: abonent providers configuration parsing.
 * ************************************************************************** */
#ifndef __INMAN_IAPMGR_PRVD_CFG_PARSING
#ident "@(#)$Id$"
#define __INMAN_IAPMGR_PRVD_CFG_PARSING

#include "inman/services/ICSCfgReader.hpp"
using smsc::inman::ICSIdent;
using smsc::inman::ICSrvCfgReaderAC_T;

using smsc::inman::ICSMultiSectionCfgReaderAC_T;

#include "inman/services/iapmgr/IAPMgrDefs.hpp"

namespace smsc {
namespace inman {
namespace iapmgr {

struct IAPrvdConfig {
  ICSIdsSet         deps;
  std::auto_ptr<IAPrvdsRegistry>  registry;

  IAPrvdConfig() : registry(new IAPrvdsRegistry())
  { }
};

class ICSProvidersCfgReader : public ICSMultiSectionCfgReaderAC_T<IAPrvdConfig> {
protected:
  // -- ----------------------------------------------
  // -- ICSMultiSectionCfgReaderAC_T interface methods
  // -- ----------------------------------------------
  ICSrvCfgReaderAC::CfgState 
    parseSection(XConfigView * cfg_sec, const std::string & nm_sec, void * opaque_arg = NULL)
      throw(ConfigException)
  {
    const char * nm_cfg = nm_sec.c_str();
    CfgParsingResult    state(sectionState(nm_sec));
    if (state.cfgState == ICSrvCfgReaderAC::cfgComplete) {
        IAProviderInfo * pvdInfo = icsCfg->registry->find(nm_sec);
        //assert(pvdInfo);
        smsc_log_info(logger, "Configured AbonentProvider '%s': %s", nm_cfg,
                      ICSIdent::uid2Name(pvdInfo->_icsUId));
        return ICSrvCfgReaderAC::cfgComplete;
    }

    smsc_log_info(logger, "Configuring AbonentProvider '%s'", nm_cfg);

    const char * cstr = NULL;
    std::string elemNm = cfg_sec->elementName("loadup");
    try { cstr = cfg_sec->getString("loadup");
    } catch (const ConfigException & exc) { }
    if (!cstr || !cstr[0])
        throw ConfigException("'%s' is missed!", elemNm.c_str());

    if (!cfg_sec->findSubSection("Config"))
        throw ConfigException("'%s.Config' subsection is missed!", cfg_sec->relSection());

    std::auto_ptr<XConfigView> pvSec(cfg_sec->getSubConfig("Config"));

    ICSUId icsUid = processICSLoadUp(pvSec->relSection(), cstr, logger); //throws on failure
    smsc_log_info(logger, "Configured AbonentProvider '%s':  %s",
                  nm_cfg, ICSIdent::uid2Name(icsUid));

    //update service dependencies
    icsDeps.insert(icsUid);
    //reserve entry in IAProviders registry
    icsCfg->registry->insert(nm_sec, new IAProviderInfo(icsUid));
    
    //mark section as completely parsed
    state.cfgState = ICSrvCfgReaderAC::cfgComplete;
    return registerSection(nm_sec, state);
  }

public:
  ICSProvidersCfgReader(Config & root_sec, Logger * use_log, const char * ics_sec = NULL)
      : ICSMultiSectionCfgReaderAC_T<IAPrvdConfig>(root_sec, use_log,
                                      ics_sec ? ics_sec : "AbonentProviders")
  { }
  ~ICSProvidersCfgReader()
  { }

  IAPrvdConfig * rlseConfig(void)
  {
    icsDeps.exportDeps(icsCfg->deps);
    return icsCfg.release();
  }
};

} //iapmgr
} //inman
} //smsc
#endif /* __INMAN_IAPMGR_PRVD_CFG_PARSING */

