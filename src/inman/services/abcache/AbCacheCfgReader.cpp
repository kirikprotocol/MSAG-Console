#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/abcache/AbCacheCfgReader.hpp"

using smsc::inman::ICSrvCfgReaderAC;

namespace smsc {
namespace inman {
namespace cache {
/* ************************************************************************** *
 * class ICSAbCacheCfgReader implementation:
 * ************************************************************************** */

//Parses XML configuration entry section, updates dependencies.
//Returns status of config parsing, 
ICSrvCfgReaderAC::CfgState
  ICSAbCacheCfgReader::parseConfig(void * opaque_arg/* = NULL*/)
    throw(ConfigException)
{
  uint32_t tmo = 0;
  //cache parameters
  try {
    tmo = (uint32_t)_topSec.getInt("cacheExpiration");
  } catch (const ConfigException & exc) {
  }
  if (!tmo || (tmo > AbonentCacheCFG::_MAX_CACHE_INTERVAL))
    icsCfg->interval = AbonentCacheCFG::_DFLT_CACHE_INTERVAL;
  else
    icsCfg->interval = tmo;
  smsc_log_info(logger, "  cacheExpiration: %u minutes%s", icsCfg->interval,
                !tmo ? " (default)":"");
  //convert minutes to seconds
  icsCfg->interval *= 60;

  try {
    icsCfg->RAM = (uint32_t)_topSec.getInt("cacheRAM");
  } catch (const ConfigException & exc) {
  }
  if (!icsCfg->RAM)
    throw ConfigException("'cacheRAM' is missing or invalid");
  smsc_log_info(logger, "  cacheRAM: %u Mb", icsCfg->RAM);

  const char * cstr = NULL;
  try {
    cstr = _topSec.getString("cacheDir");
  } catch (const ConfigException & exc) {
  }
  if (!cstr || !cstr[0])
    throw ConfigException("'cacheDir' is missing or invalid");
  icsCfg->nmDir += cstr;
  smsc_log_info(logger, "  cacheDir: %s", cstr);

  tmo = 0;
  try {
    tmo = (uint32_t)_topSec.getInt("cacheRecords");
  } catch (const ConfigException & exc) {
  }
  if (tmo)
    icsCfg->fileRcrd = (int)tmo;
  smsc_log_info(logger, "  cacheRecords: %d%s", icsCfg->fileRcrd,
                !tmo ? " (default)":"");
  /**/
  return ICSrvCfgReaderAC::cfgComplete;
}

} //cache
} //inman
} //smsc

