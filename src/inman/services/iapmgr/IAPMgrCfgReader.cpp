#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/iapmgr/IAPMgrCfgReader.hpp"
using smsc::inman::ICSIdent;

#include "util/csv/CSVArrayOf.hpp"
#include "util/csv/CSVListOf.hpp"
using smsc::util::csv::CSVArrayOfStr;
using smsc::util::csv::CSVListOfStr;

namespace smsc {
namespace inman {
namespace iapmgr {

//PolicyValue ::= "[ProvidersList] : INs_List"
//  ProvidersList ::= "Prvd_1, ... , Prvd_n"
//  INs_List ::= "*" | "IN_1, ..., IN_n"
struct IAPolicyParser {
  const char *  nmPol;
  CSVListOfStr  prvdNms;
  CSVListOfStr  scfNms;

  IAPolicyParser(const char * nm_pol) : nmPol(nm_pol)
    , prvdNms(','), scfNms(',')
  { }

  void Parse(const char * str) /*throw(ConfigException)*/
  {
    CSVArrayOfStr vPart((CSVArrayOfStr::size_type)2, ':');
    if (!vPart.fromStr(str) || vPart.size() != 2)
      throw ConfigException("%s: valie is invalid", nmPol);

    //Parse ProvidersList, NOTE it may be empty!
    prvdNms.fromStr(vPart[0].c_str());
    //verify lengths
    for (CSVListOfStr::const_iterator
         cit = prvdNms.begin(); cit != prvdNms.end(); ++cit) {
      if ((cit->length() + 1) > AbonentPolicyName_t::MAX_SZ) {
        throw ConfigException("^s: IAProvider name is too long (max %u chars): \'%s\'",
                                    nmPol, AbonentPolicyName_t::MAX_SZ-1, cit->c_str());
      }
    }
    //Parse INs_List, it cann't be empty!
    scfNms.fromStr(vPart[1].c_str());
    //verify lengths
    for (CSVListOfStr::const_iterator
         cit = scfNms.begin(); cit != scfNms.end(); ++cit) {
      if ((cit->length() + 1) > INScfIdent_t::MAX_SZ) {
        throw ConfigException("%s: IN-Platform name is too long (max %u chars): \'%s\'",
                                    nmPol, INScfIdent_t::MAX_SZ-1, cit->c_str());
      }
    }
    if (scfNms.empty())
      throw ConfigException("%s: IN-Platform name(s) is missed", nmPol);
  }

  void print(std::string & pstr) const
  {
    pstr += "Prvds{";
    if (!prvdNms.empty())
      prvdNms.toString(pstr);
    else
      pstr += "<none>";

    pstr += "}, INs{";
    if (!scfNms.empty())
      scfNms.toString(pstr);
    else
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
  icsCfg->prvdReg.reset(prvdReader.rlseConfig());
  icsDeps.exportDeps(icsCfg->deps);
  /**/
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

  try { cstr = cfg_sec->getString("active");
  } catch (const ConfigException & exc) { }

  if (cstr && cstr[0]) {
    std::string tStr(cstr);
    smsc::util::str_cut_blanks(tStr);
    if (!strcmp("false", tStr.c_str())) {
      smsc_log_info(logger, "  policy is inactive");
      //mark section as completely parsed
      state.cfgState = ICSrvCfgReaderAC::cfgComplete;
      return registerSection(nm_sec, state);
    }
    if (strcmp("true", tStr.c_str()))
      throw ConfigException("\'%s.active\' value is invalid", nm_cfg);
  }

  cstr = cfg_sec->getString("policy"); //throws

  IAPolicyParser   polXCFG(nm_cfg);
  polXCFG.Parse(cstr); //throws
  smsc_log_info(logger, "  policy is: %s", polXCFG.print().c_str());

  std::auto_ptr<AbonentPolicy> polDat(new AbonentPolicy(nm_cfg, polXCFG.prvdNms));

  //parse AddressPools
  {
    std::auto_ptr<CStrSet>  poolsSet;

    if (cfg_sec->findSubSection("AddressPools")) {
      std::auto_ptr<XConfigView> poolsCfg(cfg_sec->getSubConfig("AddressPools"));
  
      poolsSet.reset(poolsCfg->getStrParamNames());
  
      for (CStrSet::iterator it = poolsSet->begin(); it != poolsSet->end(); ++it) {
        AddressPool itPool(it->c_str());

        cstr = poolsCfg->getString(it->c_str());
        if (!itPool._mask.fromText(cstr))
          throw ConfigException("\'%s\'value is invalid: %s", it->c_str(), cstr);
        polDat->_poolsSet.insert(itPool);
      }
    }
    if (!poolsSet.get()) {
      smsc_log_warn(logger, "  'AddressPools' subsection is missed/empty");
    }
  }
    
  if (!polXCFG.scfNms.empty()) {  //lookup IN platforms configs
    scfReader.addArguments(polXCFG.scfNms);
    scfReader.readConfig(&(polDat->_scfMap)); //throws
    icsDeps.insert(ICSIdent::icsIdTCAPDisp);
  }
  if (!polXCFG.prvdNms.empty()) {      //add Abonent Provider in dependencies
    prvdReader.addArguments(polXCFG.prvdNms);
    if (prvdReader.readConfig()) //throws
      icsDeps.importDeps(prvdReader.Deps());
  }
  icsCfg->policiesReg.insert(polDat->_ident, polDat.release());
  //mark section as completely parsed
  state.cfgState = ICSrvCfgReaderAC::cfgComplete;
  return registerSection(nm_sec, state);
}


} //iapmgr
} //inman
} //smsc

