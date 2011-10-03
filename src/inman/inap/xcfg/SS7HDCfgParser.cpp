#ifdef EIN_HD
#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/inap/xcfg/SS7HDCfgParser.hpp"

#include "util/csv/CSVListOf.hpp"
using smsc::util::csv::CSVListOfStr;

namespace smsc {
namespace inman {
namespace inap {

//Returns zero if unit_nm is unknown or is not supported
//Unit's ids defined in <portss7.hpp>
static uint16_t unitIdByName(const char * unit_nm)
{
  if (unit_nm && unit_nm[0]) {
    if (!strcmp(unit_nm, "TCAP_ID"))
      return 4; //TCAP_ID;
    if (!strcmp(unit_nm, "SCCP_ID"))
      return 5; //SCCP_ID;
  }
  return 0;
}

/* ************************************************************************* *
 * class SS7HDCfgParser implementation
 * ************************************************************************* */
void SS7HDCfgParser::readConfig(const Config & root_sec, SS7HDConnParms & st_cfg)
  throw(ConfigException)
{
  XConfigView cfgSec(root_sec, nmSec);

  uint32_t      tmo = 0;
  const char *  cstr = NULL; //cpMgrHost

  try {
    cstr = cfgSec.getString("cpMgrHosts");
  } catch (const ConfigException & exc) {
  }
  if (!cstr || !cstr[0])
    throw ConfigException("Remote CommonParts Manager"
                          " host addresses are missing");

  { //validate addresses format and recompose it without blanks
    CSVListOfStr  hosts;
    if (!hosts.fromStr(cstr))
      throw ConfigException("Remote CommonParts Manager"
                            " host addresses are invalid: %s", cstr);
    unsigned i = 0;
    for (CSVListOfStr::iterator it = hosts.begin(); it != hosts.end(); ++it, ++i) {
      CSVListOfStr hp(':');
      if (hp.fromStr(it->c_str()) != 2)
        throw ConfigException("Remote CommonParts Manager"
                              " host address is invalid: %s", it->c_str());
      if (i)
        st_cfg.rcpMgrAdr += ',';
      hp.toString(st_cfg.rcpMgrAdr, 0);
    }
  }
  smsc_log_info(logger, "  cpMgrHosts: \'%s\'", st_cfg.rcpMgrAdr.c_str());

  tmo = 0;    //cpMgrInstId
  try {
    tmo = (uint32_t)cfgSec.getInt("cpMgrInstId");
  } catch (const ConfigException & exc) {
    tmo = SS7HDConnParms::_MAX_CPMGR_INSTANCE_ID + 1;
  }
  if (tmo > SS7HDConnParms::_MAX_CPMGR_INSTANCE_ID)
    throw ConfigException("Remote CommonParts Manager instanceId"
                          " is missing or invalid, allowed range [0..%u]",
                          SS7HDConnParms::_MAX_CPMGR_INSTANCE_ID);
  st_cfg.rcpMgrInstId = (uint8_t)tmo;
  smsc_log_info(logger, "  cpMgrInstId: %u", st_cfg.rcpMgrInstId);


  tmo = 0;    //mpUserId
  try {
    tmo = (uint32_t)cfgSec.getInt("mpUserId");
  } catch (const ConfigException & exc) {
    tmo = SS7HDConnParms::_MAX_USER_ID + 1;
  }
  if (!tmo || (tmo >= SS7HDConnParms::_MAX_USER_ID))
    throw ConfigException("'mpUserId' is missing or invalid,"
                          " allowed range [1..%u]", (unsigned)SS7HDConnParms::_MAX_USER_ID);
  st_cfg.mpUserId = (uint8_t)tmo;
  smsc_log_info(logger, "  mpUserId: %u", st_cfg.mpUserId);

  tmo = 0;    //maxMsgNum, default _DFLT_MSG_ENTRIES
  try {
    tmo = (uint32_t)cfgSec.getInt("maxMsgNum");
  } catch (const ConfigException & exc) {
  }
  if (tmo && (tmo >= SS7HDConnParms::_MAX_MSG_ENTRIES))
    throw ConfigException("'maxMsgNum' is invalid, allowed range [1..%u]",
                          (unsigned)SS7HDConnParms::_MAX_MSG_ENTRIES);
  st_cfg.maxMsgNum = tmo ? (uint16_t)tmo : SS7HDConnParms::_DFLT_MSG_ENTRIES;
  smsc_log_info(logger, "  maxMsgNum: %u%s", st_cfg.maxMsgNum, !tmo ? " (default)":"");


  tmo = 0;    //reconnectTimeout, default _DFLT_TMO_RECONN
  try {
    tmo = (uint32_t)cfgSec.getInt("reconnectTimeout");
  } catch (const ConfigException & exc) {
  }
  if (tmo && ((tmo > SS7HDConnParms::_MAX_TMO_RECONN) || (tmo < SS7HDConnParms::_MIN_TMO_RECONN)))
    throw ConfigException("'reconnectTimeout' is invalid, allowed range [%u..%u]",
                          (unsigned)SS7HDConnParms::_MIN_TMO_RECONN,
                          (unsigned)SS7HDConnParms::_MAX_TMO_RECONN);
  st_cfg.tmoReconn = tmo ? (uint16_t)tmo : SS7HDConnParms::_DFLT_TMO_RECONN;
  smsc_log_info(logger, "  reconnectTimeout: %u%s", st_cfg.tmoReconn, !tmo ? " (default)":"");

  cstr = NULL; //dfltLayout
  try {
    cstr = cfgSec.getString("dfltLayout");
  } catch (const ConfigException & exc) {
  }
  if (!cstr || !cstr[0])
    throw ConfigException("SS7 units layout name is not specified");
  st_cfg.nmLayout = cstr;
  smsc_log_info(logger, "  dfltLayout: \'%s\'", cstr);
  /**/
  readLayout(cfgSec, st_cfg);
  return;
}

void SS7HDCfgParser::readLayout(XConfigView & root_sec, SS7HDConnParms & st_cfg)
  throw(ConfigException)
{
  if (!root_sec.findSubSection("Layouts"))
    throw ConfigException("\'Layouts\' subsection is missing");
  std::auto_ptr<XConfigView> loSec(root_sec.getSubConfig("Layouts"));

  const char * cstr = st_cfg.nmLayout.c_str();
  if (!loSec->findSubSection(cstr))
    throw ConfigException("SS7 units layout subsection is missing: \'%s\'", cstr);

  smsc_log_info(logger, "Reading \'%s\'  layout", cstr);
  std::auto_ptr<XConfigView> cfgSec(loSec->getSubConfig(cstr));

  uint32_t tmo = 0;    //instanceId
  try {
    tmo = (uint32_t)cfgSec->getInt("instanceId");
  } catch (const ConfigException & exc) {
    tmo = SS7HDConnParms::_MAX_APPINSTANCE_ID + 1;
  }
  if (!tmo || (tmo > SS7HDConnParms::_MAX_APPINSTANCE_ID))
    throw ConfigException("application instanceId"
                          " is missing or invalid, allowed range [1..%u]",
                          SS7HDConnParms::_MAX_APPINSTANCE_ID);
  st_cfg.appInstId = (uint8_t)tmo;
  smsc_log_info(logger, "  instanceId: %u", st_cfg.appInstId);

  cstr = NULL; //ss7Units
  try {
    cstr = cfgSec->getString("ss7Units");
  } catch (const ConfigException & exc) {
  }
  if (!cstr || !cstr[0])
    throw ConfigException("SS7 Unit(s) subsection list is missing");

  CSVListOfStr nmUnits;
  if (!nmUnits.fromStr(cstr))
    throw ConfigException("SS7 Unit(s) subsection list is invalid: %s", cstr);
  smsc_log_info(logger, "  SS7Units: %s", nmUnits.toString().c_str());


  if (!root_sec.findSubSection("SS7Units"))
    throw ConfigException("\'SS7Units\' subsection is missing");
  std::auto_ptr<XConfigView> unitSec(root_sec.getSubConfig("SS7Units"));

  for (CSVListOfStr::const_iterator cit = nmUnits.begin();
      cit != nmUnits.end(); ++cit) {
    readUnits(*unitSec.get(), st_cfg, cit->c_str());
  }
  return;
}

void SS7HDCfgParser::readUnits(XConfigView & xcfg_sec, SS7HDConnParms & st_cfg, const char * nm_subs)
  throw(ConfigException)
{
  if (!xcfg_sec.findSubSection(nm_subs))
    throw ConfigException("SS7Units subsection is missing: %s", nm_subs);

  smsc_log_info(logger, "Reading \'%s\' settings", nm_subs);
  std::auto_ptr<XConfigView> cfgSec(xcfg_sec.getSubConfig(nm_subs));

  const char * cstr = NULL; //unitId
  try {
    cstr = cfgSec->getString("unitId");
  } catch (const ConfigException & exc) {
  }
  if (!cstr || !cstr[0])
    throw ConfigException("SS7Unit id is missing");

  uint16_t unitId = unitIdByName(cstr);
  if (!unitId)
    throw ConfigException("SS7Unit id is invalid or unsupported: %s", cstr);
  smsc_log_info(logger, "  unitId: %s", cstr);


  cstr = NULL; //instancesList
  try {
    cstr = cfgSec->getString("instancesList");
  } catch (const ConfigException & exc) {
  }
  if (!cstr || !cstr[0])
    throw ConfigException("SS7Unit instances list is missing");

  CSVListOfStr nmIds;
  if (!nmIds.fromStr(cstr))
    throw ConfigException("SS7Unit instances list is invalid: %s", cstr);

  SS7Unit_CFG & unitCfg = st_cfg.ss7Units[unitId];
  unitCfg.unitId = unitId; //if it's just created

  for (CSVListOfStr::const_iterator cit = nmIds.begin(); cit != nmIds.end(); ++cit) {
    int instId = atoi(cit->c_str());
    if (!instId || (instId > SS7HDConnParms::_MAX_UNIT_INSTANCE_ID))
      throw ConfigException("SS7Unit instanceId is invalid: %s", cit->c_str());
    if (unitCfg.instIds.findInstance((uint8_t)instId))
      throw ConfigException("SS7Unit instanceId is duplicated: %s", cit->c_str());
    unitCfg.instIds.insert(SS7UnitInstance((uint8_t)instId));
  }
  smsc_log_info(logger, "  instancesList: %s", nmIds.toString().c_str());
  return;
}

} //inap
} //inman
} //smsc
#endif /* EIN_HD */

