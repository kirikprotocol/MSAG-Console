#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/iapmgr/IAPrvdsCfgReader.hpp"

using smsc::inman::ICSIdent;

namespace smsc {
namespace inman {
namespace iapmgr {
/* ************************************************************************** *
 * class ICSProvidersCfgReader implementation:
 * ************************************************************************** */

// -- ----------------------------------------------
// -- ICSMultiSectionCfgReaderAC_T interface methods
// -- ----------------------------------------------
ICSrvCfgReaderAC::CfgState
  ICSProvidersCfgReader::parseSection(XConfigView & cfg_sec,
                                      const std::string & nm_sec,
                                      void * opaque_arg/* = NULL*/)
  throw(ConfigException)
{
  const char *      nmPrvd = nm_sec.c_str();
  CfgParsingResult  state(sectionState(nm_sec));
  std::auto_ptr<IAProviderInfo> pPrvd(new IAProviderInfo(nmPrvd));

  if (state.cfgState == ICSrvCfgReaderAC::cfgComplete) {
    IAProviderInfo * pvdInfo = icsCfg->find(pPrvd->_ident);
    //assert(pvdInfo);
    smsc_log_info(logger, "Configured AbonentProvider '%s': %s", nmPrvd,
                  ICSIdent::uid2Name(pvdInfo->_icsUId));
    return ICSrvCfgReaderAC::cfgComplete;
  }
  smsc_log_info(logger, "Configuring AbonentProvider '%s'", nmPrvd);

  const char * cstr = NULL;
  std::string elemNm = cfg_sec.elementName("loadup");
  try { cstr = cfg_sec.getString("loadup");
  } catch (const ConfigException & exc) { }
  if (!cstr || !cstr[0])
    throw ConfigException("'%s' is missed!", elemNm.c_str());

  if (!cfg_sec.findSubSection("Config"))
    throw ConfigException("'%s.Config' subsection is missed!", cfg_sec.relSection());
  {
    XConfigView pvSec;
    cfg_sec.getSubConfig(pvSec, "Config");
    pPrvd->_icsUId = processICSLoadUp(pvSec.relSection(), cstr, logger); //throws on failure
    smsc_log_info(logger, "Configured AbonentProvider '%s':  %s",
                  nmPrvd, ICSIdent::uid2Name(pPrvd->_icsUId));
  }

  //update service dependencies
  icsDeps.insert(pPrvd->_icsUId);
  //reserve entry in IAProviders registry
  icsCfg->insert(pPrvd->_ident, pPrvd.release());

  //mark section as completely parsed
  state.cfgState = ICSrvCfgReaderAC::cfgComplete;
  return registerSection(nm_sec, state);
}

} //iapmgr
} //inman
} //smsc

