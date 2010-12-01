#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/inap/xcfg/TCUsrCfgParser.hpp"

#include "util/config/XCFView.hpp"
using smsc::util::config::XConfigView;

namespace smsc {
namespace inman {
namespace inap {
/* ************************************************************************* *
 * class TCAPUsrCfgParser implementation
 * ************************************************************************* */
void TCAPUsrCfgParser::readConfig(const Config & root_sec, TCAPUsr_CFG & st_cfg)
  throw(ConfigException)
{
  XConfigView cfgSec(root_sec, nmSec);

  const char * cstr = NULL;
  try {
    cstr = cfgSec.getString("ownAddress");
  } catch (const ConfigException & exc) {
  }
  if (!cstr || !cstr[0])
    throw ConfigException("TCAPUser ISDN address is missing");
  if (!st_cfg.ownAddr.fromText(cstr) || !st_cfg.ownAddr.fixISDN())
    throw ConfigException("TCAPUser ISDN address is invalid: %s", cstr);

  uint32_t tmo = 0;
  try {
    tmo = (uint32_t)cfgSec.getInt("ownSsn");
  } catch (const ConfigException & exc) {
  }
  if (tmo && (tmo > 255))
    tmo = 0;
  else
    st_cfg.ownSsn = (uint8_t)tmo;
  if (!tmo)
    throw ConfigException("TCAPUser SSN number must belong to range [1..255]");

  tmo = 0;    //fakeSsn, optional
  try {
    tmo = (uint32_t)cfgSec.getInt("fakeSsn");
  } catch (const ConfigException & exc) {
  }
  if (tmo && (tmo > 255))
    throw ConfigException("TCAPUser fakeSSN number must belong to range [1..255]");
  st_cfg.fakeSsn = (uint8_t)tmo;

  smsc_log_info(logger, "  GT: %s", st_cfg.toString().c_str());

  tmo = 0;    //maxTimeout, optional
  try {
    tmo = (uint32_t)cfgSec.getInt("rosTimeout");
  } catch (const ConfigException & exc) {
  }
  if (tmo && (tmo > TCAPUsr_CFG::_MAX_ROS_TIMEOUT))
    throw ConfigException("'rosTimeout' must belong to range [1..%u] seconds",
                          (unsigned)TCAPUsr_CFG::_MAX_ROS_TIMEOUT);
  st_cfg.rosTimeout = tmo ? (uint16_t)tmo : TCAPUsr_CFG::_DFLT_ROS_TIMEOUT;
  smsc_log_info(logger, "  rosTimeout: %u secs%s",
                st_cfg.rosTimeout, !tmo ? " (default)":"");

  tmo = 0;    //maxDialogs, optional
  try {
    tmo = (uint32_t)cfgSec.getInt("maxDialogs");
  } catch (const ConfigException & exc) {
  }
  if ((tmo >= TCAPUsr_CFG::_MAX_TCAP_DIALOGS) || (tmo < 1))
    throw ConfigException("'maxDialogs' must belong to range [1..%u]",
                          (unsigned)TCAPUsr_CFG::_MAX_TCAP_DIALOGS);
  st_cfg.maxDlgId = tmo ? (uint16_t)tmo : TCAPUsr_CFG::_DFLT_TCAP_DIALOGS;
  smsc_log_info(logger, "  maxDialogs: %u%s",
                st_cfg.maxDlgId, !tmo ? " (default)":"");
  /**/
  return;
}

} //inap
} //inman
} //smsc

