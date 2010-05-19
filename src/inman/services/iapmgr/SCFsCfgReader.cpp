#ifndef MOD_IDENT_OFF
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "util/csv/CSVArrayOf.hpp"
#include "inman/services/iapmgr/SCFsCfgReader.hpp"

namespace smsc {
namespace inman {
namespace iapmgr {

using util::csv::CSVArrayOfStr;

/* ************************************************************************** *
 * class SCFsCfgReader implementation:
 * ************************************************************************** */
TDPCategory::Id SCFsCfgReader::str2tdp(const char * str)
{
  for (TDPNames::const_iterator it = tdpNames.begin(); it != tdpNames.end(); ++it) {
    if (!strcmp(str, it->second))
      return it->first;
  }
  return TDPCategory::dpUnknown;
}

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
SKAlgorithmAC * SCFsCfgReader::readSkeyVal(TDPCategory::Id tdp_type, std::string & str)
{
  uint32_t skey = 0;
  return str2UInt(&skey, str) ? new SKAlgorithm_SKVal(tdp_type, skey) : NULL;
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
  SCFsCfgReader::readSkeyAlg(XConfigView & scf_cfg, TDPCategory::Id tdp_type, const char * str)
{
  CSVArrayOfStr             algStr(3, ':');
  CSVArrayOfStr::size_type  n = algStr.fromStr(str);
  if (!n)
    return NULL;
  if (n == 1) //just a value
    return readSkeyVal(tdp_type, algStr[0]);
  // n >= 2
  if (!strcmp("val", algStr[0].c_str())) //just a value
    return readSkeyVal(tdp_type, algStr[1]);

  if (!strcmp("map", algStr[0].c_str())) {
    if (n < 3)
      return NULL;

    TDPCategory::Id argTdp = str2tdp(algStr[1].c_str());
    if (argTdp == TDPCategory::dpUnknown)
      return NULL;
    if (!scf_cfg.findSubSection(algStr[2].c_str()))
      return NULL;
    std::auto_ptr<SKAlgorithm_SKMap> alg(new SKAlgorithm_SKMap(tdp_type, argTdp));
    std::auto_ptr<XConfigView> xltCfg(scf_cfg.getSubConfig(algStr[2].c_str()));
    bool res = readSKeyMap(alg.get(), *xltCfg.get());
    return (res && alg->size()) ? alg.release() : NULL;
  }
  return NULL;
}

//Parses service keys definition section
unsigned SCFsCfgReader::readSrvKeys(XConfigView & scf_cfg, SKAlgorithmMAP & sk_alg)
    throw(ConfigException)
{
  if (!scf_cfg.findSubSection("ServiceKeys"))
    throw ConfigException("'ServiceKeys' subsection is missed");
  std::auto_ptr<XConfigView> skeyCfg(scf_cfg.getSubConfig("ServiceKeys"));

  std::auto_ptr<CStrSet> subs(skeyCfg->getStrParamNames());
  for (CStrSet::iterator sit = subs->begin(); sit != subs->end(); ++sit) {
    TDPCategory::Id tdpType = str2tdp(sit->c_str());
    if (tdpType != TDPCategory::dpUnknown) {
      const char *    cstr = skeyCfg->getString(sit->c_str());
      SKAlgorithmAC * alg = readSkeyAlg(scf_cfg, tdpType, cstr);
      if (alg) {
        sk_alg[tdpType] = alg;
        smsc_log_info(logger, "  skey %s %s", sit->c_str(), alg->toString().c_str());
      } else
        throw ConfigException("  %s service key value/algorithm is invalid",
                                  TDPCategory::Name(tdpType));
    } else
      smsc_log_warn(logger, " %s service key is unknown/unsupported", sit->c_str());
  }
  return (unsigned)sk_alg.size();
}

//Reads IN-platform configuration (not 'aliasFor' form)
INScfCFG * SCFsCfgReader::readSCFCfg(XConfigView & cfg_sec,
                                     const TonNpiAddress & scf_adr, const char * nm_scf)
    throw(ConfigException)
{
  // according to INScfCFG::IDPLocationAddr
  static const char * const _IDPLIAddr[] = { "MSC", "SMSC", "SSF" };
  static const char * const _IDPReqMode[] = { "MT", "SEQ" };

  std::auto_ptr<INScfCFG> pin(new INScfCFG(nm_scf));
  pin->scfAdr = scf_adr;
  //Read service keys
  if (!readSrvKeys(cfg_sec, pin->skAlg)) {
    smsc_log_warn(logger, "%s: no service keys is specified", scf_adr.getSignals());
  }

  // -- OPTIONAL parameters --//

  //list of RP causes forcing charging denial because of low balance
  const char * cstr = NULL;
  std::string cppStr = "  RPCList_reject: ";
  try { cstr = cfg_sec.getString("RPCList_reject");
  } catch (const ConfigException & exc) { }
  if (cstr) {
    try { pin->rejectRPC.fromStr(cstr);
    } catch (const std::exception & exc) {
      throw ConfigException("RPCList_reject: %s", exc.what());
    }
  }
  if ((pin->rejectRPC.size() <= 1) || !pin->rejectRPC.toString(cppStr))
    cppStr += "unsupported";
  smsc_log_info(logger, cppStr.c_str());

  //list of RP causes indicating that IN point should be
  //interacted again a bit later
  cstr = NULL; cppStr = "  RPCList_retry: ";
  try { cstr = cfg_sec.getString("RPCList_retry");
  } catch (const ConfigException & exc) { }
  if (cstr) {
    try { pin->retryRPC.fromStr(cstr); }
    catch (std::exception& exc) {
      throw ConfigException("RPCList_retry: %s", exc.what());
    }
  }
  //adjust default attempt setings for given RPCauses
  for (RPCListATT::iterator it = pin->retryRPC.begin();
                          it != pin->retryRPC.end(); ++it) {
    if (!it->_att)
      ++(it->_att);
  }
  if (!pin->retryRPC.toString(cppStr))
    cppStr += "unsupported";
  smsc_log_info(logger, cppStr.c_str());

  cstr = NULL; cppStr = "IDPLocationInfo: ";
  try { cstr = cfg_sec.getString("IDPLocationInfo");
  } catch (const ConfigException & exc) { }

  if (cstr && cstr[0]) {
    if (!strcmp(cstr, _IDPLIAddr[INScfCFG::idpLiSSF]))
      pin->idpLiAddr = INScfCFG::idpLiSSF;
    else if (!strcmp(cstr, _IDPLIAddr[INScfCFG::idpLiSMSC]))
      pin->idpLiAddr = INScfCFG::idpLiSMSC;
    else if (strcmp(cstr, _IDPLIAddr[INScfCFG::idpLiMSC]))
      throw ConfigException("IDPLocationInfo: invalid value");
  } else
    cstr = (char*)_IDPLIAddr[INScfCFG::idpLiMSC];
  smsc_log_info(logger, "  IDPLocationInfo: %s", cstr);

  cstr = NULL; cppStr = "IDPReqMode: ";
  try { cstr = cfg_sec.getString("IDPReqMode");
  } catch (const ConfigException & exc) { }

  if (cstr && cstr[0]) {
    if (!strcmp(cstr, _IDPReqMode[INScfCFG::idpReqSEQ]))
      pin->idpReqMode = INScfCFG::idpReqSEQ;
    else if (strcmp(cstr, _IDPReqMode[INScfCFG::idpReqMT]))
      throw ConfigException("IDPReqMode: invalid value");
  } else
    cstr = (char*)_IDPReqMode[INScfCFG::idpReqMT];
  smsc_log_info(logger, "  IDPReqMode: %s", cstr);
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
                   pInCfg->scfAdr.toString().c_str());
    pCRefMap->insert(INScfsMAP::value_type(scfAdr.toString(), (const INScfCFG *)pInCfg));
  }

  //mark section as completely parsed
  state.cfgState = ICSrvCfgReaderAC::cfgComplete;
  return registerSection(nm_sec, state);
}

} //iapmgr
} //inman
} //smsc

