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
    std::auto_ptr<XConfigView> xltCfg(scf_cfg.getSubConfig(algStr[2].c_str()));
    bool res = readSKeyMap(alg.get(), *xltCfg.get());
    return (res && alg->size()) ? alg.release() : NULL;
  }
  return NULL;
}

//Parses service keys definition section
unsigned SCFsCfgReader::readSrvKeys(XConfigView & scf_cfg, SKAlgorithmsDb & sk_alg)
    throw(ConfigException)
{
  if (!scf_cfg.findSubSection("ServiceKeys"))
    throw ConfigException("'ServiceKeys' subsection is missed");
  std::auto_ptr<XConfigView> skeyCfg(scf_cfg.getSubConfig("ServiceKeys"));

  std::auto_ptr<CStrSet> subs(skeyCfg->getStrParamNames());
  for (CStrSet::iterator sit = subs->begin(); sit != subs->end(); ++sit) {
    CSIUid_e csiType = UnifiedCSI::tdp2Id(sit->c_str());

    if (csiType != UnifiedCSI::csi_UNDEFINED) {
      const char *    cstr = skeyCfg->getString(sit->c_str());
      std::auto_ptr<SKAlgorithmAC> pAlg(readSkeyAlg(scf_cfg, csiType, cstr));

      if (pAlg.get()) {
        sk_alg.addAlgorithm(csiType, pAlg.release());
        smsc_log_info(logger, "  skey %s %s", sit->c_str(), pAlg->toString().c_str());
      } else
        throw ConfigException("  %s service key value/algorithm is invalid", sit->c_str());
    } else
      smsc_log_warn(logger, " %s service key is unknown/unsupported", sit->c_str());
  }
  return sk_alg.size();
}

//Reads IN-platform configuration (not 'aliasFor' form)
INScfCFG * SCFsCfgReader::readSCFCfg(XConfigView & cfg_sec,
                                     const TonNpiAddress & scf_adr,
                                     const char * nm_scf)
    throw(ConfigException)
{
  // according to INScfCFG::IDPLocationAddr
  static const char * const _IDPLIAddr[] = { "MSC", "SMSC", "SSF" };
  static const char * const _IDPReqMode[] = { "MT", "SEQ" };

  std::auto_ptr<INScfCFG> pin(new INScfCFG(nm_scf));
  pin->_scfAdr = scf_adr;
  //Read service keys
  if (!readSrvKeys(cfg_sec, pin->_skDb)) {
    smsc_log_warn(logger, "%s: no service keys is specified", scf_adr.getSignals());
  }

  // -- OPTIONAL parameters --//

  //list of RP causes forcing charging denial because of low balance
  const char * cstr = NULL;
  std::string cppStr = "  RPCList_reject: ";
  try { cstr = cfg_sec.getString("RPCList_reject");
  } catch (const ConfigException & exc) { }
  if (cstr) {
    try { pin->_capSms.rejectRPC.fromStr(cstr);
    } catch (const std::exception & exc) {
      throw ConfigException("RPCList_reject: %s", exc.what());
    }
  }
  if ((pin->_capSms.rejectRPC.size() <= 1)
      || !pin->_capSms.rejectRPC.toString(cppStr))
    cppStr += "unsupported";
  smsc_log_info(logger, cppStr.c_str());

  //list of RP causes indicating that IN point should be
  //interacted again a bit later
  cstr = NULL; cppStr = "  RPCList_retry: ";
  try { cstr = cfg_sec.getString("RPCList_retry");
  } catch (const ConfigException & exc) { }
  if (cstr) {
    try { pin->_capSms.retryRPC.fromStr(cstr); }
    catch (const std::exception & exc) {
      throw ConfigException("RPCList_retry: %s", exc.what());
    }
  }
  //adjust default attempt setings for given RPCauses
  for (RPCListATT::iterator it = pin->_capSms.retryRPC.begin();
                          it != pin->_capSms.retryRPC.end(); ++it) {
    if (!it->_att)
      ++(it->_att);
  }
  if (!pin->_capSms.retryRPC.toString(cppStr))
    cppStr += "unsupported";
  smsc_log_info(logger, cppStr.c_str());

  cstr = NULL;
  try { cstr = cfg_sec.getString("IDPLocationInfo");
  } catch (const ConfigException & exc) { }

  if (cstr && cstr[0]) {
    if (!strcmp(cstr, _IDPLIAddr[INParmsCapSms::idpLiSSF]))
      pin->_capSms.idpLiAddr = INParmsCapSms::idpLiSSF;
    else if (!strcmp(cstr, _IDPLIAddr[INParmsCapSms::idpLiSMSC]))
      pin->_capSms.idpLiAddr = INParmsCapSms::idpLiSMSC;
    else if (strcmp(cstr, _IDPLIAddr[INParmsCapSms::idpLiMSC]))
      throw ConfigException("IDPLocationInfo: invalid value");
  } else
    cstr = (char*)_IDPLIAddr[INParmsCapSms::idpLiMSC];
  smsc_log_info(logger, "  IDPLocationInfo: %s", cstr);

  cstr = NULL;
  try { cstr = cfg_sec.getString("IDPReqMode");
  } catch (const ConfigException & exc) { }

  if (cstr && cstr[0]) {
    if (!strcmp(cstr, _IDPReqMode[INParmsCapSms::idpReqSEQ]))
      pin->_capSms.idpReqMode = INParmsCapSms::idpReqSEQ;
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
    if (!pin->_dfltImsi.fromText(cstr))
      throw ConfigException("defaultIMSI: invalid value \'%s\'", cstr);
    smsc_log_info(logger, "  defaultIMSI: %s", pin->_dfltImsi.c_str());
  } else
    smsc_log_info(logger, "  defaultIMSI: <none>", cstr);
  /**/
  return pin.release();
}

// -- ----------------------------------------------
// -- ICSMultiSectionCfgReaderAC_T interface methods
// -- ----------------------------------------------
ICSrvCfgReaderAC::CfgState SCFsCfgReader::parseSection(XConfigView * cfg_sec, const std::string & nm_sec,
                      void * opaque_arg/* = NULL*/)
    throw(ConfigException)
{
  const char * nm_cfg = nm_sec.c_str();
  CfgParsingResult state(sectionState(nm_sec));
  if (state.cfgState == ICSrvCfgReaderAC::cfgComplete) {
    smsc_log_info(logger, "Already read '%s' configuration ..", nm_cfg);
    return ICSrvCfgReaderAC::cfgComplete;
  }

  smsc_log_info(logger, "reading '%s' configuration ..", nm_cfg);

  const char * cstr = NULL;
  try { cstr = cfg_sec->getString("scfAddress"); //throws
  } catch (const ConfigException & exc) { }
  if (!cstr || !cstr[0])
    throw ConfigException("%s.scfAddress is invalid or missing", nm_cfg);

  TonNpiAddress scfAdr;
  if (!scfAdr.fromText(cstr) || !scfAdr.fixISDN())
    throw ConfigException("%s.scfAddress is invalid: %s", nm_cfg, cstr);

  {   //check uniqueness
    const INScfCFG * pScf = icsCfg->find(scfAdr.toString());
    if (pScf)
      throw ConfigException("Multiple settings for SCF '%s' (%s vs %s)",
                            scfAdr.getSignals(), nm_cfg, pScf->_ident.c_str());
  }
  smsc_log_info(logger, "IN-platform '%s' config ..", nm_cfg);
  smsc_log_info(logger, "  ISDN: %s", scfAdr.toString().c_str());

  //read configuration
  INScfCFG *  pInCfg = NULL;

  cstr = NULL;
  try { cstr = cfg_sec->getString("aliasFor");
  } catch (const ConfigException & exc) { }
  if (cstr && cstr[0]) {
    //read configuration of targeted IN-platform
    smsc_log_info(logger, "  aliasFor: %s", cstr);
    std::string refNm(cstr);
    std::auto_ptr<XConfigView> refCfg(_cfgXCV->getSubConfig(cstr));
    parseSection(refCfg.get(), refNm, opaque_arg); //throws if not a cfgComplete
    const CfgParsingResult * refState = sectionState(refNm);
    pInCfg = (INScfCFG *)(refState->opaqueRes);
  } else {
    //insert configuration into registry
    state.opaqueRes = pInCfg = readSCFCfg(*cfg_sec, scfAdr, nm_cfg);
    icsCfg->insert(scfAdr.toString(), pInCfg);
  }

  //export configuration pointer if required
  INScfsMAP * pCRefMap = (INScfsMAP *)opaque_arg;
  if (pCRefMap) {
    smsc_log_debug(logger, "INScfsMAP: linking '%s'-> {%s, %s}",
                   scfAdr.toString().c_str(), pInCfg->_ident.c_str(),
                   pInCfg->_scfAdr.toString().c_str());
    pCRefMap->insert(INScfsMAP::value_type(scfAdr.toString(), (const INScfCFG *)pInCfg));
  }

  //mark section as completely parsed
  state.cfgState = ICSrvCfgReaderAC::cfgComplete;
  return registerSection(nm_sec, state);
}

} //iapmgr
} //inman
} //smsc

