#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/smbill/SmBillCfgReader.hpp"

#include "inman/incache/AbCacheDefs.hpp"
using smsc::inman::cache::AbonentCacheCFG;

#include "inman/inap/xcfg/TCUsrCfgParser.hpp"
using smsc::inman::inap::TCAPUsrCfgParser;

#include "util/csv/CSVArrayOf.hpp"
using smsc::util::csv::CSVArrayOfStr;


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
  //according to ChargeParm::ContractReqMode
  static const char * _abReq[] = { "onDemand", "always" };
  uint32_t tmo = 0;
  const char * cstr = NULL;

  //read BillingModes subsection
  readBillingModes(_topSec);
  if (icsCfg->prm->mo_billMode.useIN() || icsCfg->prm->mt_billMode.useIN()) {
    icsDeps.insert(ICSIdent::icsIdTCAPDisp);
    icsDeps.insert(ICSIdent::icsIdScheduler);
  }

  cstr = NULL;
  icsCfg->prm->cntrReq = ChargeParm::reqOnDemand;
  try { cstr = _topSec.getString("abonentTypeRequest");
  } catch (const ConfigException & exc) { }
  if (cstr && cstr[0]) {
    if (!strcmp(_abReq[ChargeParm::reqAlways], cstr)) {
      icsCfg->prm->cntrReq = ChargeParm::reqAlways; 
    } else if (strcmp(_abReq[ChargeParm::reqOnDemand], cstr))
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
  if (!strcmp(cstr, _CDRmodes[ChargeParm::cdrALL]))
    icsCfg->prm->cdrMode = ChargeParm::cdrALL;
  else if (!strcmp(cstr, _CDRmodes[ChargeParm::cdrNONE]))
    icsCfg->prm->cdrMode = ChargeParm::cdrNONE;
  else if (strcmp(cstr, _CDRmodes[ChargeParm::cdrBILLMODE]))
    throw ConfigException("'cdrMode' is unknown or missing");
  smsc_log_info(logger, "  cdrMode: %s [%d]", cstr, icsCfg->prm->cdrMode);

  if (icsCfg->prm->cdrMode != ChargeParm::cdrNONE) {
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
  } catch (const ConfigException & exc) { tmo = _MAX_BILLINGS_NUM; }
  if (tmo >= _MAX_BILLINGS_NUM)
    throw ConfigException("'maxBilling' is invalid or missing,"
                            " allowed range [1..%u)", _MAX_BILLINGS_NUM);
  icsCfg->prm->maxBilling = tmo ? (uint16_t)tmo : _DFLT_BILLINGS_NUM;
  smsc_log_info(logger, "  maxBilling: %u per connect%s", icsCfg->prm->maxBilling,
                !tmo ? " (default)":"");

  tmo = 0;    //maxTimeout
  try { tmo = (uint32_t)_topSec.getInt("maxTimeout");
  } catch (const ConfigException & exc) { tmo = _MAX_DELAY_SECS; }
  if ((tmo >= _MAX_DELAY_SECS) || (tmo < 2))
    throw ConfigException("'maxTimeout' is invalid or missing,"
                            " allowed range [2..%u)", _MAX_DELAY_SECS);
  icsCfg->maxTimeout =  tmo ? (uint16_t)tmo : _DFLT_BILL_TIMEOUT;
  smsc_log_info(logger, "  maxTimeout: %u secs%s", icsCfg->maxTimeout,
                !tmo ? " (default)":"");

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
    icsCfg->prm->capSms.reset(readCAP3Sms(_topSec));

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

void ICSSmBillingCfgReader::str2BillMode(const char * m_str,
                                    ChargeParm::BILL_MODE (& pbm)[2])
    throw(ConfigException)
{
  if (!m_str || !m_str[0])
    throw ConfigException("Invalid billMode");

  CSVArrayOfStr  bmList((CSVArrayOfStr::size_type)2);
  if (!bmList.fromStr(m_str) || (bmList.size() > 2))
    throw ConfigException("Invalid billMode '%s'", m_str);

  pbm[0] = pbm[1] = ChargeParm::billOFF;
  CSVArrayOfStr::size_type i = 0;
  do {
    if (!strcmp(_BILLmodes[ChargeParm::bill2IN], bmList[i].c_str()))
      pbm[i] = ChargeParm::bill2IN;
    else if (!strcmp(_BILLmodes[ChargeParm::bill2CDR], bmList[i].c_str())) {
      pbm[i] = ChargeParm::bill2CDR; // no need to check next value
      pbm[++i] = ChargeParm::bill2CDR; 
    } else if (!strcmp(_BILLmodes[ChargeParm::billOFF], bmList[i].c_str()))
      ++i; // no need to check next value
    else
      throw ConfigException("Invalid billMode '%s'", bmList[i].c_str());
  } while (++i < bmList.size());

  //check bill2IN setting ..
  if ((pbm[0] == ChargeParm::bill2IN) && (pbm[0] == pbm[1]))
    throw ConfigException("Invalid billMode '%s'", m_str);
}

void ICSSmBillingCfgReader::readBillMode(ChargeParm::MSG_TYPE msg_type,
                                    const char * mode, bool mt_bill)
    throw(ConfigException)
{
  BillModes * b_map = mt_bill ? &icsCfg->prm->mt_billMode : &icsCfg->prm->mo_billMode;
  if (b_map->isAssigned(msg_type))
    throw ConfigException("Multiple settings for '%s'", _MSGtypes[msg_type]);

  ChargeParm::BILL_MODE    pbm[2];
  str2BillMode(mode, pbm);

  //according to #B2501:
  if ((msg_type == ChargeParm::msgSMS) && (pbm[0] == ChargeParm::bill2IN)) {
    smsc_log_warn(logger, "Unsafe billMode '%s' for messageType '%s'",
                          _BILLmodes[ChargeParm::bill2IN],
                          _MSGtypes[ChargeParm::msgSMS]);
    smsc_log_warn(logger, "  consider double CDR creation for prepaid abonents by serving IN-point");
/*    throw ConfigException("Forbidden billMode '%s' for messageType '%s'",
                          _BILLmodes[ChargeParm::bill2IN],
                          _MSGtypes[ChargeParm::msgSMS]);*/
  }
  b_map->assign(msg_type, pbm[0], pbm[1]);
  smsc_log_info(logger, "    %s -> %s, %s", mt_bill ? "MT" : "MO",
                ChargeParm::billModeStr(pbm[0]), ChargeParm::billModeStr(pbm[1]));
}

void ICSSmBillingCfgReader::readModesFor(ChargeParm::MSG_TYPE msg_type, XConfigView * m_cfg)
    throw(ConfigException)
{
  const char * mode = NULL;
  smsc_log_info(logger, "  %s ..", ChargeParm::msgTypeStr(msg_type));

  try { mode = m_cfg->getString("MO"); }
  catch (const ConfigException& exc) { }
  if (!mode || !mode[0])
    throw ConfigException("%s parameter 'MO' is invalid or missing!",
                              ChargeParm::msgTypeStr(msg_type));
  readBillMode(msg_type, mode, false);

  try { mode = m_cfg->getString("MT"); }
  catch (const ConfigException& exc) { }
  if (!mode || !mode[0])
    throw ConfigException("%s parameter 'MT' is invalid or missing!",
                              ChargeParm::msgTypeStr(msg_type));
  readBillMode(msg_type, mode, true);
}

/* Reads BillingModes subsection */
void ICSSmBillingCfgReader::readBillingModes(XConfigView & cfg)
    throw(ConfigException)
{
  if (!cfg.findSubSection("BillingModes"))
    throw ConfigException("'BillingModes' subsection is missed");

  XConfigView   bmCfg;
  cfg.getSubConfig(bmCfg, "BillingModes");

  std::auto_ptr<CStrSet>  msgs(bmCfg.getShortSectionNames());
  if (msgs->empty())
    throw ConfigException("no billing modes set");

  for (CStrSet::iterator sit = msgs->begin(); sit != msgs->end(); ++sit) {
    std::auto_ptr<XConfigView> curMsg(bmCfg.getSubConfig(sit->c_str()));
    if (!strcmp(_MSGtypes[ChargeParm::msgSMS], sit->c_str()))
      readModesFor(ChargeParm::msgSMS, curMsg.get());
    else if (!strcmp(_MSGtypes[ChargeParm::msgUSSD], sit->c_str()))
      readModesFor(ChargeParm::msgUSSD, curMsg.get());
    else if (!strcmp(_MSGtypes[ChargeParm::msgXSMS], sit->c_str()))
      readModesFor(ChargeParm::msgXSMS, curMsg.get());
    else
      throw ConfigException("Illegal section for messageType %s", sit->c_str());
  }
}

//Returns true if service depends on other ones
TCAPUsr_CFG * ICSSmBillingCfgReader::readCAP3Sms(XConfigView & cfg_sec)
    throw(ConfigException)
{
  const char * cstr = NULL;
  try { cstr = cfg_sec.getString("CAP3Sms");
  } catch (const ConfigException & exc) { }

  TCAPUsrCfgParser    parser(logger, cstr);
  if (!_xmfCfg.hasSection(parser.nmCfgSection()))
    throw ConfigException("section %s' is missing!", parser.nmCfgSection());

  smsc_log_info(logger, "Reading settings from '%s' ..", parser.nmCfgSection());
  XCFConfig * pTCfg = _xmfCfg.getSectionConfig(cstr);

  std::auto_ptr<TCAPUsr_CFG>  capCfg(new TCAPUsr_CFG());
  parser.readConfig(pTCfg->second, *capCfg.get()); //throws
  /**/
  return capCfg.release();
}

} //smbill
} //inman
} //smsc

