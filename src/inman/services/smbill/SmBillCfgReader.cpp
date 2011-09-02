#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/smbill/SmBillCfgReader.hpp"
#include "inman/services/common/BillModesCfgReader.hpp"

#include "inman/incache/AbCacheDefs.hpp"
using smsc::inman::cache::AbonentCacheCFG;

#include "inman/inap/xcfg/TCUsrCfgParser.hpp"
using smsc::inman::inap::TCAPUsrCfgParser;

namespace smsc {
namespace inman {
namespace smbill {

#ifdef SMSEXTRA
/* ************************************************************************* *
 * class ICSXSMSCfgReader implementation
 * ************************************************************************* */
ICSrvCfgReaderAC::CfgState
  ICSXSMSCfgReader::parseSection(XConfigView & cfg_sec,
                    const std::string & nm_sec, void * opaque_arg/* = NULL*/)
    throw(ConfigException)
{
  const char * nm_cfg = nm_sec.c_str();
  CfgParsingResult state(sectionState(nm_sec));
  if (state.cfgState == ICSrvCfgReaderAC::cfgComplete) {
    smsc_log_info(logger, "Already read '%s' configuration ..", nm_cfg);
    return ICSrvCfgReaderAC::cfgComplete;
  }

  smsc_log_info(logger, "SMS Extra service '%s' config ..", nm_cfg);
  XSmsService xSrv(nm_cfg);

  try { xSrv.mask = (uint32_t)cfg_sec.getInt("serviceMask");
  } catch (const ConfigException & exc) { }
  if (!xSrv.mask || (xSrv.mask & SMSX_RESERVED_MASK))
    throw ConfigException("'serviceMask' is missed or invalid or reserved bits is used");

  SmsXServiceMap::iterator xit = icsCfg->find(xSrv.mask);
  if (xit != icsCfg->end())
    throw ConfigException("'serviceMask' %u is shared by %s and %s",
                            xSrv.mask, (xit->second).name.c_str(), nm_cfg);

  try { xSrv.svcCode = (uint32_t)cfg_sec.getInt("serviceCode");
  } catch (const ConfigException & exc) { }
  if (!xSrv.svcCode)
    throw ConfigException("'serviceCode' is missed or invalid");

  const char * cstr = NULL;
  try { cstr = cfg_sec.getString("serviceAdr"); //optional param
  } catch (const ConfigException & exc) { }
  if (!cstr || !cstr[0])
    smsc_log_warn(logger, "  'serviceAdr' is omitted");
  else if (!xSrv.adr.fromText(cstr))
    throw ConfigException("'serviceAdr' is invalid: %s", cstr);

  try { xSrv.chargeBearer = cfg_sec.getBool("chargeBearer"); //optional param
  } catch (const ConfigException & exc) { }

  icsCfg->insert(SmsXServiceMap::value_type(xSrv.mask, xSrv));
  smsc_log_info(logger, "  service[0x%x]: %u, %s%s", xSrv.mask, xSrv.svcCode,
                xSrv.adr.toString().c_str(), xSrv.chargeBearer ? ", chargeBearer" : "");

  //mark section as completely parsed
  state.cfgState = ICSrvCfgReaderAC::cfgComplete;
  return registerSection(nm_sec, state);
}
#endif /* SMSEXTRA */

/* ************************************************************************* *
 * class ICSSmBillingCfgReader implementation
 * ************************************************************************* */
//Returns true if service depends on other ones
ICSrvCfgReaderAC::CfgState
  ICSSmBillingCfgReader::parseConfig(void * opaque_obj/* = NULL*/) throw(ConfigException)
{
  //according to SmBillParams::ContractReqMode
  static const char * _abReq[] = { "onDemand", "always" };
  uint32_t tmo = 0;
  const char * cstr = NULL;

  //read BillingModes subsection
  readBillingModes(_topSec, icsCfg->prm->billMode);
  if (icsCfg->prm->billMode.useIN()) {
    icsDeps.insert(ICSIdent::icsIdTCAPDisp);
    icsDeps.insert(ICSIdent::icsIdScheduler);
  }

  cstr = NULL;
  icsCfg->prm->cntrReq = SmBillParams::reqOnDemand;
  try { cstr = _topSec.getString("abonentTypeRequest");
  } catch (const ConfigException & exc) { }
  if (cstr && cstr[0]) {
    if (!strcmp(_abReq[SmBillParams::reqAlways], cstr)) {
      icsCfg->prm->cntrReq = SmBillParams::reqAlways; 
    } else if (strcmp(_abReq[SmBillParams::reqOnDemand], cstr))
      throw ConfigException("illegal 'abonentTypeRequest' value");
  } else
    cstr = NULL;
  smsc_log_info(logger, "  abonentTypeRequest: %s%s", _abReq[icsCfg->prm->cntrReq],
                !cstr ? " (default)":"");

  if (icsCfg->prm->needIAProvider()) {
    //abonent contract determination policy is required
    cstr = NULL;
    try { cstr = _topSec.getString("abonentPolicy");
    } catch (const ConfigException & exc) { }
    if (!cstr || !cstr[0])
      throw ConfigException("default abonent policy is not set!");
    smsc_log_info(logger, "  using abonent policy %s", cstr);
    icsCfg->policyNm = cstr;
    icsDeps.insert(ICSIdent::icsIdIAPManager, "*"); //icsCfg->policyNm

    tmo = 0;    //abtTimeout
    try { tmo = (uint32_t)_topSec.getInt("abonentTypeTimeout");
    } catch (const ConfigException & exc) { tmo = _MAX_ABTYPE_TIMEOUT; }
    if (tmo >= _MAX_ABTYPE_TIMEOUT)
      throw ConfigException("'abonentTypeTimeout' should fall into the"
                                  " range [1 ..%u) seconds", _MAX_ABTYPE_TIMEOUT);
    icsCfg->abtTimeout = tmo ? (uint16_t)tmo : _DFLT_ABTYPE_TIMEOUT;
    smsc_log_info(logger, "  abonentTypeTimeout: %u secs%s", icsCfg->abtTimeout,
                  !tmo ? " (default)":"");
  }

  // -----------------------
  // CDR storage parameters
  // -----------------------
  cstr = NULL;
  try { cstr = _topSec.getString("cdrMode");
  } catch (const ConfigException & exc) {
    throw ConfigException("'cdrMode' is unknown or missing");
  }
  if (!strcmp(cstr, SmBillParams::cdrModeStr(SmBillParams::cdrALL)))
    icsCfg->prm->cdrMode = SmBillParams::cdrALL;
  else if (!strcmp(cstr, SmBillParams::cdrModeStr(SmBillParams::cdrNONE)))
    icsCfg->prm->cdrMode = SmBillParams::cdrNONE;
  else if (strcmp(cstr, SmBillParams::cdrModeStr(SmBillParams::cdrBILLMODE)))
    throw ConfigException("'cdrMode' is unknown or missing");
  smsc_log_info(logger, "  cdrMode: %s [%d]", cstr, icsCfg->prm->cdrMode);

  if (icsCfg->prm->cdrMode != SmBillParams::cdrNONE) {
    cstr = NULL;
    try { cstr = _topSec.getString("cdrDir");
    } catch (const ConfigException & exc) { }
    if (!cstr || !cstr[0])
        throw ConfigException("'cdrDir' is invalid or missing");
    icsCfg->prm->cdrDir = cstr;
    smsc_log_info(logger, "  cdrDir: %s", cstr);

    tmo = 0;
    try { tmo = (uint32_t)_topSec.getInt("cdrInterval");
    } catch (const ConfigException & exc) { }
    if (tmo && (tmo < _MIN_CDR_ROLL_INTERVAL))
        tmo = 0;
    else
        icsCfg->prm->cdrInterval = tmo;
    if (!tmo) {
        icsCfg->prm->cdrInterval = _DFLT_CDR_ROLL_INTERVAL;
        smsc_log_info(logger, "cdrInterval is invalid or missing");
    }
    smsc_log_info(logger, "  cdrInterval: %u secs%s", icsCfg->prm->cdrInterval,
                  !tmo ? " (default)":"");
  }

  tmo = 0;    //maxBillings
  try { tmo = (uint32_t)_topSec.getInt("maxBillings");
  } catch (const ConfigException & exc) { tmo = _MAX_BILLINGS_NUM + 1; }
  if (tmo > _MAX_BILLINGS_NUM)
    throw ConfigException("'maxBilling' is invalid or missing,"
                            " allowed range [1..%u)", _MAX_BILLINGS_NUM);
  icsCfg->prm->maxBilling = tmo ? tmo : _DFLT_BILLINGS_NUM;
  smsc_log_info(logger, "  maxBilling: %u per connect%s", icsCfg->prm->maxBilling,
                !tmo ? " (default)":"");

  tmo = 0;
  try { tmo = (uint32_t)_topSec.getInt("maxThreads");
  } catch (const ConfigException & exc) { tmo = _DFLT_THREADS_NUM; }
  if (tmo > _MAX_THREADS_NUM)
    throw ConfigException("maxThreads is out of range [0 ..65535]");

  icsCfg->prm->maxThreads = (uint16_t)tmo;
  if (!tmo) {
    smsc_log_info(logger, "  maxThreads: unlimited per connect");
  } else {
    smsc_log_info(logger, "  maxThreads: %u per connect%s", (unsigned)icsCfg->prm->maxThreads,
                  (tmo == _DFLT_THREADS_NUM) ? " (default)":"");
  }

  tmo = 0;    //maxTimeout
  try { tmo = (uint32_t)_topSec.getInt("maxTimeout");
  } catch (const ConfigException & exc) { tmo = _MAX_DELAY_SECS; }
  if ((tmo >= _MAX_DELAY_SECS) || (tmo < 2))
    throw ConfigException("'maxTimeout' is invalid or missing,"
                            " allowed range [2..%u)", _MAX_DELAY_SECS);
  icsCfg->maxTimeout =  tmo ? (uint16_t)tmo : _DFLT_BILL_TIMEOUT;
  smsc_log_info(logger, "  maxTimeout: %u secs%s", icsCfg->maxTimeout,
                !tmo ? " (default)":"");
  if (icsCfg->maxTimeout < _MIN_BILL_TIMEOUT) {
    smsc_log_warn(logger, "  maxTimeout is lesser than recommended minimum %u secs", _MIN_BILL_TIMEOUT);
  }

  //cache parameters
  {
    bool dflt = false;
    try { icsCfg->prm->useCache = _topSec.getBool("useCache");
    } catch (const ConfigException & exc) {
        icsCfg->prm->useCache = dflt = true;
    }
    smsc_log_info(logger, "  useCache: %s%s", icsCfg->prm->useCache ? "true" : "false",
                  dflt ? " (default)":"");
  }
  if (icsCfg->prm->useCache) {
    icsDeps.insert(ICSIdent::icsIdAbntCache);

    tmo = 0;
    try { tmo = (uint32_t)_topSec.getInt("cacheExpiration");
    } catch (const ConfigException & exc) { tmo = AbonentCacheCFG::_MAX_CACHE_INTERVAL; }
    if (tmo >= AbonentCacheCFG::_MAX_CACHE_INTERVAL)
        throw ConfigException("'cacheExpiration' is invalid or missing,"
                              " allowed range [1..%u)", AbonentCacheCFG::_MAX_CACHE_INTERVAL);
    icsCfg->prm->cacheTmo = tmo ? tmo : AbonentCacheCFG::_DFLT_CACHE_INTERVAL;
    smsc_log_info(logger, "  cacheExpiration: %u minutes%s", icsCfg->prm->cacheTmo,
                  !tmo ? " (default)":"");
    //convert minutes to seconds
    icsCfg->prm->cacheTmo *= 60;
  }

  //read CAP3Sms configuration
  if (icsDeps.LookUp(ICSIdent::icsIdTCAPDisp))
    readCAP3Sms(_topSec, icsCfg->prm->capSms.init());

#ifdef SMSEXTRA
  /* ********************************* *
   * SMS Extra services configuration: *
   * ********************************* */
  cstr = NULL;
  try { cstr = _topSec.getString("smsExtraConfig");
  } catch (const ConfigException & exc) { }
  if (cstr && cstr[0]) { //throws
    smsc_log_info(logger, "  'smsExtraConfig': %s", cstr);
    //Attempt to read and parse SMS Extra config file ..
    ICSXSMSCfgReader xReader(_xmfCfg, logger, cstr);
    xReader.readConfig();
    icsCfg->prm->smsXMap.reset(xReader.rlseConfig());
    smsc_log_info(logger, "total SMS Extra services configured: %u",
                  icsCfg->prm->smsXMap->size());
  }
#endif /* SMSEXTRA */
  /**/
  return ICSrvCfgReaderAC::cfgComplete;
}

/* Reads BillingModes subsection */
void ICSSmBillingCfgReader::readBillingModes(const XConfigView & cfg_sec, TrafficBillModes & st_bmode)
    throw(ConfigException)
{
  const char * cstr = NULL;
  try { cstr = cfg_sec.getString("billingModes");
  } catch (const ConfigException & exc) { }
  if (!cstr || !cstr[0])
    throw ConfigException("parametr 'billingModes' is invalid or missing!");
  smsc_log_info(logger, "  billingModes: '%s' ..", cstr);

  BillModesCfgReader  parser(logger);
  const XCFConfig * pBmCfg = _xmfCfg.hasSection(parser.nmCfgSection());
  if (!pBmCfg)
    throw ConfigException("section '%s' is missing!", parser.nmCfgSection());
  parser.parseSection(pBmCfg->second, cstr, st_bmode); //throws
}

//Returns true if service depends on other ones
void ICSSmBillingCfgReader::readCAP3Sms(const XConfigView & cfg_sec, TCAPUsr_CFG & cap_cfg)
    throw(ConfigException)
{
  const char * cstr = NULL;
  try { cstr = cfg_sec.getString("CAP3Sms");
  } catch (const ConfigException & exc) { }
  if (!cstr || !cstr[0])
    throw ConfigException("parametr 'CAP3Sms' is invalid or missing!");

  TCAPUsrCfgParser  parser(logger, cstr);
  const XCFConfig * pTCfg = _xmfCfg.hasSection(parser.nmCfgSection());
  if (!pTCfg)
    throw ConfigException("section '%s' is missing!", parser.nmCfgSection());

  smsc_log_info(logger, "Reading settings from '%s' ..", parser.nmCfgSection());
  parser.readConfig(pTCfg->second, cap_cfg); //throws
}

} //smbill
} //inman
} //smsc

