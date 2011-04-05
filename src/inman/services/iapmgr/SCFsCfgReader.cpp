#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "util/csv/CSVArrayOf.hpp"
#include "inman/services/iapmgr/SCFsCfgReader.hpp"

namespace smsc {
namespace inman {
namespace iapmgr {

using util::csv::CSVArrayOfStr;

/* ************************************************************************** *
 * class SCFsCfgReader implementation:
 * ************************************************************************** */
bool SCFsCfgReader::str2UInt(uint32_t * p_val, const std::string & str)
{
  if (str.empty())
    return false;
  *p_val = atoi(str.c_str());
  if (!*p_val) { //check for all zeroes
    if (strspn(str.c_str(), "0") != str.size())
      return false;
  }
  return true;
}

//Reads service key presented as VAL algorithm
SKAlgorithmAC * SCFsCfgReader::readSkeyVal(CSIUid_e csi_type, std::string & str)
{
  uint32_t skey = 0;
  return str2UInt(&skey, str) ? new SKAlgorithm_SKVal(csi_type, skey) : NULL;
}

bool SCFsCfgReader::readSKeyMap(SKAlgorithm_SKMap * alg, XConfigView & xlt_cfg)
{
  std::auto_ptr<CStrSet> subs(xlt_cfg.getIntParamNames());
  for (CStrSet::const_iterator sit = subs->begin(); sit != subs->end(); ++sit) {
    uint32_t argKey = 0, resKey = 0;
    //check paramName
    if (str2UInt(&argKey, *sit)) {
      try { 
        resKey = (uint32_t)xlt_cfg.getInt(sit->c_str());
        alg->insert(argKey, resKey);
      } catch (const ConfigException & exc) {
        return false;
      }
    } else
      return false;
  }
  return true;
}

//Parses service key algorithm formatted as:
//  "[algId :] algArg [: algParams] "
SKAlgorithmAC * 
  SCFsCfgReader::readSkeyAlg(XConfigView & scf_cfg, CSIUid_e csi_type, const char * str)
{
  CSVArrayOfStr             algStr(3, ':');
  CSVArrayOfStr::size_type  n = algStr.fromStr(str);
  if (!n)
    return NULL;
  if (n == 1) //just a value
    return readSkeyVal(csi_type, algStr[0]);
  // n >= 2
  if (!strcmp("val", algStr[0].c_str())) //just a value
    return readSkeyVal(csi_type, algStr[1]);

  if (!strcmp("map", algStr[0].c_str())) {
    if (n < 3)
      return NULL;

    CSIUid_e argCsi = UnifiedCSI::tdp2Id(algStr[1].c_str());
    if (argCsi == UnifiedCSI::csi_UNDEFINED)
      return NULL;
    if (!scf_cfg.findSubSection(algStr[2].c_str()))
      return NULL;
    std::auto_ptr<SKAlgorithm_SKMap> alg(new SKAlgorithm_SKMap(csi_type, argCsi));
    XConfigView xltCfg;
    scf_cfg.getSubConfig(xltCfg, algStr[2].c_str());
    bool res = readSKeyMap(alg.get(), xltCfg);
    return (res && alg->size()) ? alg.release() : NULL;
  }
  return NULL;
}

//Parses service keys definition section
unsigned SCFsCfgReader::readSrvKeys(XConfigView & scf_cfg, SKAlgorithmsDb & sk_alg)
    throw(ConfigException)
{
  if (!scf_cfg.findSubSection("ServiceKeys")) //Optional 
    return 0;

  std::auto_ptr<XConfigView>  skeyCfg(scf_cfg.getSubConfig("ServiceKeys"));
  std::auto_ptr<CStrSet>      subs(skeyCfg->getStrParamNames());

  for (CStrSet::const_iterator sit = subs->begin(); sit != subs->end(); ++sit) {
    CSIUid_e csiType = UnifiedCSI::tdp2Id(sit->c_str());

    if (csiType != UnifiedCSI::csi_UNDEFINED) {
      const char *    cstr = skeyCfg->getString(sit->c_str());
      std::auto_ptr<SKAlgorithmAC> pAlg(readSkeyAlg(scf_cfg, csiType, cstr));

      if (pAlg.get()) {
        smsc_log_info(logger, "  serviceKey(%s) %s", sit->c_str(), pAlg->toString().c_str());
        sk_alg.addAlgorithm(csiType, pAlg.release());
      } else
        throw ConfigException("serviceKey(%s) value/algorithm is invalid", sit->c_str());
    } else
      smsc_log_warn(logger, " serviceKey(%s) is unknown/unsupported", sit->c_str());
  }
  return sk_alg.size();
}


void SCFsCfgReader::readSSFLocationId(XConfigView & cfg_sec, CellGlobalId & cell_gid)
  throw(ConfigException)
{
  const char * cstr = NULL;

  try { cstr = cfg_sec.getString("MCC");
  } catch (const ConfigException & exc) { }
  if (!cstr || !*cstr)
    throw ConfigException("%s.MCC value is missing", cfg_sec.relSection());
  if (!MobileCountryCode::validateChars(cstr))
    throw ConfigException("%s.MCC value is invalid: ^s", cfg_sec.relSection(), cstr);
  strncpy(cell_gid._mcc._value, cstr, MobileCountryCode::_MAX_SZ);

  cstr = NULL;
  try { cstr = cfg_sec.getString("MNC");
  } catch (const ConfigException & exc) { }
  if (!cstr || !*cstr)
    throw ConfigException("%s.MNC value is missing", cfg_sec.relSection());
  if (!MobileNetworkCode::validateChars(cstr))
    throw ConfigException("%s.MNC value is invalid: ^s", cfg_sec.relSection(), cstr);
  strncpy(cell_gid._mnc._value, cstr, MobileNetworkCode::_MAX_SZ);
  
  cstr = NULL;
  try { cstr = cfg_sec.getString("LAC");
  } catch (const ConfigException & exc) { }
  if (!cstr || !*cstr)
    throw ConfigException("%s.LAC value is missing", cfg_sec.relSection());
  if (!LocationAreaCode::validateChars(cstr))
    throw ConfigException("%s.LAC value is invalid: ^s", cfg_sec.relSection(), cstr);
  strncpy(cell_gid._lac._value, cstr, LocationAreaCode::_MAX_SZ);

  cstr = NULL;
  try { cstr = cfg_sec.getString("CI");
  } catch (const ConfigException & exc) { }
  if (!cstr || !*cstr)
    throw ConfigException("%s.CI value is missing", cfg_sec.relSection());
  if (!CellIdentity::validateChars(cstr))
    throw ConfigException("%s.CI value is invalid: ^s", cfg_sec.relSection(), cstr);
  strncpy(cell_gid._ci._value, cstr, CellIdentity::_MAX_SZ);
}

//Reads optional IN-platform parameters
void SCFsCfgReader::readSCFParms(const char * nm_sec, XConfigView & cfg_sec,
                                 INScfParams & in_parms)
  throw(ConfigException)
{
  // according to INScfCFG::IDPLocationAddr
  static const char * const _IDPLIAddr[] = { "MSC", "SMSC", "SSF" };
  static const char * const _IDPReqMode[] = { "MT", "SEQ" };

  //Read service keys
  if (!readSrvKeys(cfg_sec, in_parms._skDb)) {
    smsc_log_warn(logger, "%s: no service keys is specified", nm_sec);
  }

  //list of RP causes forcing charging denial because of low balance
  const char * cstr = NULL;
  std::string cppStr = "  RPCList_reject: ";
  try { cstr = cfg_sec.getString("RPCList_reject");
  } catch (const ConfigException & exc) { }
  if (cstr) {
    try { in_parms._capSms.rejectRPC.fromStr(cstr);
    } catch (const std::exception & exc) {
      throw ConfigException("RPCList_reject: %s", exc.what());
    }
    //NOTE: rejectRPC.fromStr() overwrites default RPC, so readd it
    in_parms._capSms.rejectRPC.push_front(RP_MO_SM_transfer_rejected);
  }
  if ((in_parms._capSms.rejectRPC.size() <= 1)
      || !in_parms._capSms.rejectRPC.toString(cppStr))
    cppStr += "<none>";
  smsc_log_info(logger, cppStr.c_str());

  //list of RP causes indicating that IN point should be
  //interacted again a bit later
  cstr = NULL; cppStr = "  RPCList_retry: ";
  try { cstr = cfg_sec.getString("RPCList_retry");
  } catch (const ConfigException & exc) { }
  if (cstr) {
    try { in_parms._capSms.retryRPC.fromStr(cstr); }
    catch (const std::exception & exc) {
      throw ConfigException("RPCList_retry: %s", exc.what());
    }
  }
  //adjust default attempt setings for given RPCauses
  for (RPCListATT::iterator it = in_parms._capSms.retryRPC.begin();
                          it != in_parms._capSms.retryRPC.end(); ++it) {
    if (!it->_att)
      ++(it->_att);
  }
  if (!in_parms._capSms.retryRPC.toString(cppStr))
    cppStr += "<none>";
  smsc_log_info(logger, cppStr.c_str());

  cstr = NULL;
  try { cstr = cfg_sec.getString("IDPLocationInfo");
  } catch (const ConfigException & exc) { }

  if (cstr && cstr[0]) {
    if (!strcmp(cstr, _IDPLIAddr[INParmsCapSms::idpLiSSF]))
      in_parms._capSms.idpLiAddr = INParmsCapSms::idpLiSSF;
    else if (!strcmp(cstr, _IDPLIAddr[INParmsCapSms::idpLiSMSC]))
      in_parms._capSms.idpLiAddr = INParmsCapSms::idpLiSMSC;
    else if (strcmp(cstr, _IDPLIAddr[INParmsCapSms::idpLiMSC]))
      throw ConfigException("IDPLocationInfo: invalid value");
  } else
    cstr = (char*)_IDPLIAddr[INParmsCapSms::idpLiMSC];
  smsc_log_info(logger, "  IDPLocationInfo: %s", cstr);

  //check for SSF CellGlobalId
  if (in_parms._capSms.idpLiAddr == INParmsCapSms::idpLiSSF) {
    if (cfg_sec.findSubSection("SSFLocationIdentification")) {
      XConfigView cfgSsfLoc;
      cfg_sec.getSubConfig(cfgSsfLoc, "SSFLocationIdentification");
      readSSFLocationId(cfgSsfLoc, in_parms._capSms._cellGId);
      smsc_log_info(logger, "  SSFLocationIdentification: %s",
                    in_parms._capSms._cellGId.toString().c_str());
    } else {
      smsc_log_info(logger, "  SSFLocationIdentification: <none>", cstr);
    }
  }

  cstr = NULL;
  try { cstr = cfg_sec.getString("IDPReqMode");
  } catch (const ConfigException & exc) { }

  if (cstr && cstr[0]) {
    if (!strcmp(cstr, _IDPReqMode[INParmsCapSms::idpReqSEQ]))
      in_parms._capSms.idpReqMode = INParmsCapSms::idpReqSEQ;
    else if (strcmp(cstr, _IDPReqMode[INParmsCapSms::idpReqMT]))
      throw ConfigException("IDPReqMode: invalid value");
  } else
    cstr = (char*)_IDPReqMode[INParmsCapSms::idpReqMT];
  smsc_log_info(logger, "  IDPReqMode: %s", cstr);

  cstr = NULL;
  try { cstr = cfg_sec.getString("defaultIMSI");
  } catch (const ConfigException & exc) { }

  //IMSI: "%[5-15][0-9]s"
  if (cstr && cstr[0]) {
    if (!in_parms._dfltImsi.fromText(cstr))
      throw ConfigException("defaultIMSI: invalid value \'%s\'", cstr);
    smsc_log_info(logger, "  defaultIMSI: %s", in_parms._dfltImsi.c_str());
  } else
    smsc_log_info(logger, "  defaultIMSI: <none>", cstr);
  /**/
}

// -- ----------------------------------------------
// -- ICSMultiSectionCfgReaderAC_T interface methods
// -- ----------------------------------------------
ICSrvCfgReaderAC::CfgState
  SCFsCfgReader::parseSection(XConfigView & cfg_sec, const std::string & nm_sec,
                              void * opaque_arg/* = NULL*/)
    throw(ConfigException)
{
  const char *      nmCfg = nm_sec.c_str();
  CfgParsingResult  state(sectionState(nm_sec));

  if (state.cfgState == ICSrvCfgReaderAC::cfgComplete) {
    smsc_log_info(logger, "Already read '%s' configuration ..", nmCfg);
    return ICSrvCfgReaderAC::cfgComplete;
  }
  smsc_log_info(logger, "reading '%s' configuration ..", nmCfg);

  std::auto_ptr<INScfCFG> pin(new INScfCFG(nmCfg));

  const char * cstr = NULL;
  try { cstr = cfg_sec.getString("scfAddress"); //throws
  } catch (const ConfigException & exc) { }
  if (!cstr || !cstr[0])
    throw ConfigException("%s.scfAddress is invalid or missing", nmCfg);

  if (!pin->_scfAdr.fromText(cstr) || !pin->_scfAdr.fixISDN())
    throw ConfigException("%s.scfAddress is invalid: %s", nmCfg, cstr);

  { //check gsmSCF address uniqueness
    const INScfCFG * pCfg = icsCfg->getScfParms(pin->_scfAdr);
    if (pCfg)
      throw ConfigException("Multiple settings for gsmSCF '%s' (%s vs %s)",
                            pin->_scfAdr.toString().c_str(), nmCfg, pCfg->_ident.c_str());
  }
  smsc_log_info(logger, "IN-platform '%s' config ..", nmCfg);
  smsc_log_info(logger, "  ISDN: %s", pin->_scfAdr.toString().c_str());

  //parameters configuration
  const INScfCFG *  pInCfg = NULL;

  cstr = NULL;
  try { cstr = cfg_sec.getString("aliasFor");
  } catch (const ConfigException & exc) { }
  if (cstr && cstr[0]) {
    //read configuration of targeted IN-platform
    smsc_log_info(logger, "  aliasFor: %s", cstr);

    XConfigView refCfg;
    _topSec.getSubConfig(refCfg, cstr);  //throws
    //read aliased configuration & insert into registry
    std::string refSec(cstr);
    parseSection(refCfg, refSec, opaque_arg); //throws if not a cfgComplete

    INScfIdent_t  nmAlias(cstr);
    pInCfg = icsCfg->insertAlias(pin.get(), nmAlias);
    /* */
  } else {
    //read parameters & insert into registry
    readSCFParms(nmCfg, cfg_sec, pin->_prm.init());
    icsCfg->insertParms(pin.get());
    pInCfg = pin.get();
  }
  smsc_log_debug(logger, "INScfsMAP: linking '%s'-> {%s, %s}",
                 pin->_scfAdr.toString().c_str(), pInCfg->_ident.c_str(),
                 pInCfg->_scfAdr.toString().c_str());
  pin.release();

  //mark section as completely parsed
  state.cfgState = ICSrvCfgReaderAC::cfgComplete;
//  state.opaqueRes = (const void *)pInCfg;
  return registerSection(nm_sec, state);
}

} //iapmgr
} //inman
} //smsc

