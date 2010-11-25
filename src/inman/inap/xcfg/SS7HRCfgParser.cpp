#ifndef EIN_HD  //EIN SS7 HR
#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/inap/xcfg/SS7HRCfgParser.hpp"

#include "util/config/XCFView.hpp"
using smsc::util::config::XConfigView;

namespace smsc {
namespace inman {
namespace inap {

/* ************************************************************************* *
 * class SS7HRCfgParser implementation
 * ************************************************************************* */
void SS7HRCfgParser::readConfig(Config & root_sec, SS7HRConnParms & st_cfg)
  throw(ConfigException)
{
  XConfigView cfgSec(root_sec, nmSec);

  uint32_t tmo = 0;    //ss7UserId
  try {
    tmo = (uint32_t)cfgSec.getInt("ss7UserId");
  } catch (const ConfigException & exc) {
    tmo = SS7HRConnParms::_MAX_USER_ID + 1;
  }
  if (!tmo || (tmo >= SS7HRConnParms::_MAX_USER_ID))
    throw ConfigException("'ss7UserId' is missing or invalid,"
                          " allowed range [1..%u]", (unsigned)SS7HRConnParms::_MAX_USER_ID);
  st_cfg.mpUserId = (uint8_t)tmo;
  smsc_log_info(logger, "  ss7UserId: %u", st_cfg.mpUserId);

  tmo = 0;    //maxMsgNum, default _DFLT_MSG_ENTRIES
  try {
    tmo = (uint32_t)cfgSec.getInt("maxMsgNum");
  } catch (const ConfigException & exc) {
  }
  if (tmo && (tmo >= SS7HRConnParms::_MAX_MSG_ENTRIES))
    throw ConfigException("'maxMsgNum' is invalid, allowed range [1..%u]",
                          (unsigned)SS7HRConnParms::_MAX_MSG_ENTRIES);
  st_cfg.maxMsgNum = tmo ? (uint16_t)tmo : SS7HRConnParms::_DFLT_MSG_ENTRIES;
  smsc_log_info(logger, "  maxMsgNum: %u%s", st_cfg.maxMsgNum, !tmo ? " (default)":"");

  tmo = 0;    //reconnectTimeout, default _DFLT_TMO_RECONN
  try {
    tmo = (uint32_t)cfgSec.getInt("reconnectTimeout");
  } catch (const ConfigException & exc) {
  }
  if (tmo && ((tmo > SS7HRConnParms::_MAX_TMO_RECONN) || (tmo < SS7HRConnParms::_MIN_TMO_RECONN)))
    throw ConfigException("'reconnectTimeout' is invalid, allowed range [%u..%u]",
                          (unsigned)SS7HRConnParms::_MIN_TMO_RECONN,
                          (unsigned)SS7HRConnParms::_MAX_TMO_RECONN);
  st_cfg.tmoReconn = tmo ? (uint16_t)tmo : SS7HRConnParms::_DFLT_TMO_RECONN;
  smsc_log_info(logger, "  reconnectTimeout: %u%s", st_cfg.tmoReconn, !tmo ? " (default)":"");
  /**/
  return;
}

} //inap
} //inman
} //smsc
#endif /* EIN_HD */

