#ifndef MOD_IDENT_OFF
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/services/iapmgr/IAPMgrCfgReader.hpp"

#include "util/csv/CSVArrayOf.hpp"
using smsc::util::csv::CSVArrayOfStr;

namespace smsc {
namespace inman {
namespace iapmgr {

struct IAPolicyParser {
  std::string prvdNm;
  CStrSet     scfNms;

  bool Parse(const char * str)
  {
    CSVArrayOfStr vlist((CSVArrayOfStr::size_type)2);
    if (!vlist.fromStr(str))
      return false;

    prvdNm = vlist[0]; //provider section name may be empty!
    //prepare SCF sections name set
    for (CSVArrayOfStr::size_type n = 1; n < vlist.size(); ++n) {
      if (!vlist[n].empty())
        scfNms.insert(vlist[n]);
    }
    return !(prvdNm.empty() && scfNms.empty());
  }

  const char * nmPrvd(void) const
  {
    return prvdNm.empty() ? NULL : prvdNm.c_str();
  }

  void print(std::string & pstr) const
  {
    pstr += "Prvd{";
    pstr += prvdNm.empty() ? "<none>" : prvdNm.c_str();
    pstr += "}, INs{";
    if (!scfNms.empty()) {
      CStrSet::const_iterator it = scfNms.begin();
      pstr += *it;
      for (++it; it != scfNms.end(); ++it) {
        pstr += ",  "; pstr += *it;
      }
    } else
      pstr += "<none>";
    pstr += "}";
  }

  std::string print(void) const
  {
    std::string pstr;
    print(pstr);
    return pstr;
  }
};

/* ************************************************************************** *
 * class ICSIAPMgrCfgReader implementation:
 * ************************************************************************** */
IAPManagerCFG * ICSIAPMgrCfgReader::rlseConfig(void)
{
  icsCfg->scfReg.reset(scfReader.rlseConfig());
  std::auto_ptr<IAPrvdConfig> prvdCfg(prvdReader.rlseConfig());
  icsCfg->prvdReg.reset(prvdCfg->registry.release());
  //combine dependencies
  icsDeps.exportDeps(icsCfg->deps);
  icsCfg->deps.insert(prvdCfg->deps.begin(), prvdCfg->deps.end());
  return icsCfg.release();
}

// -- ----------------------------------------------
// -- ICSMultiSectionCfgReaderAC_T interface methods
// -- ----------------------------------------------
ICSrvCfgReaderAC::CfgState 
  ICSIAPMgrCfgReader::parseSection(XConfigView * cfg_sec, const std::string & nm_sec,
                                   void * opaque_arg/* = NULL*/)
    throw(ConfigException)
{
  const char * nm_cfg = nm_sec.c_str();
  CfgParsingResult state(sectionState(nm_sec));
  if (state.cfgState == ICSrvCfgReaderAC::cfgComplete) {
    smsc_log_info(logger, "Already read '%s' configuration ..", nm_cfg);
    return ICSrvCfgReaderAC::cfgComplete;
  }

  smsc_log_info(logger, "Reading %s policy config ..", nm_cfg);
  const char * cstr = NULL;
  cstr = cfg_sec->getString("policy"); //throws

  IAPolicyParser   polXCFG;
  if (!polXCFG.Parse(cstr))
    throw ConfigException("'%s' policy value is invalid: %s", nm_cfg, cstr ? cstr : "");
  smsc_log_info(logger, "  policy is: %s", polXCFG.print().c_str());

  std::auto_ptr<AbonentPolicy> polDat(new AbonentPolicy(nm_cfg, polXCFG.nmPrvd()));
    
  if (!polXCFG.scfNms.empty()) {  //lookup IN platforms configs
    scfReader.addArguments(polXCFG.scfNms);
    scfReader.readConfig(&(polDat->scfMap)); //throws
    icsDeps.insert(ICSIdent::icsIdTCAPDisp);
  }
  if (polXCFG.nmPrvd()) {      //add Abonent Provider in dependencies
    prvdReader.addArgument(polDat->prvdNm);
    if (prvdReader.readConfig()) { //throws
      ICSIdsSet ids;
      prvdReader.Deps().exportDeps(ids);
      icsDeps.insert(ids);
    }
  }
  icsCfg->polReg.insert(polDat->ident, polDat.release());
  //mark section as completely parsed
  state.cfgState = ICSrvCfgReaderAC::cfgComplete;
  return registerSection(nm_sec, state);
}


} //iapmgr
} //inman
} //smsc

