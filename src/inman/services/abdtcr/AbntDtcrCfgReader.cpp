#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/abdtcr/AbntDtcrCfgReader.hpp"

namespace smsc {
namespace inman {
namespace abdtcr {
/* ************************************************************************** *
 * class ICSAbntDetectorCfgReader implementation:
 * ************************************************************************** */
  //Parses XML configuration entry section, updates dependencies.
  //Returns status of config parsing, 
ICSrvCfgReaderAC::CfgState
  ICSAbntDetectorCfgReader::parseConfig(void *opaque_arg) throw(ConfigException)
{
  uint32_t tmo = 0;
  const char * cstr = NULL;
  XConfigView cfgSec(rootSec, nmCfgSection());

  //abonent contract determination policy is required
  try { cstr = cfgSec.getString("abonentPolicy");
  } catch (const ConfigException & exc) { }
  if (!cstr || !cstr[0])
    throw ConfigException("default abonent policy is not set!");
  smsc_log_info(logger, "  abonent policy %s", cstr);
  icsCfg->policyNm = cstr;
  icsDeps.insert(ICSIdent::icsIdIAPManager, "*"); //icsCfg->policyNm

  tmo = 0;    //abtTimeout
  try { tmo = (uint32_t)cfgSec.getInt("abonentTypeTimeout");
  } catch (const ConfigException & exc) { tmo = _MAX_ABTYPE_TIMEOUT; }
  if (tmo >= _MAX_ABTYPE_TIMEOUT)
    throw ConfigException("'abonentTypeTimeout' should fall into the"
                                " range [1 ..%u) seconds", _MAX_ABTYPE_TIMEOUT);
  icsCfg->abtTimeout = tmo ? (uint16_t)tmo : _DFLT_ABTYPE_TIMEOUT;
  smsc_log_info(logger, "  abonentTypeTimeout: %u secs%s", icsCfg->abtTimeout,
                !tmo ? " (default)":"");

  tmo = 0;    //maxRequests
  try { tmo = (uint32_t)cfgSec.getInt("maxRequests");
  } catch (const ConfigException & exc) { tmo = _MAX_REQUESTS_NUM; }
  if (tmo >= _MAX_REQUESTS_NUM)
    throw ConfigException("'maxRequests' is invalid or missing,"
                            " allowed range [1..%u)", _MAX_REQUESTS_NUM);
  icsCfg->maxRequests = tmo ? (uint16_t)tmo : _DFLT_REQUESTS_NUM;
  smsc_log_info(logger, "  maxRequests: %u per connect%s", icsCfg->maxRequests,
                !tmo ? " (default)":"");

  //cache parameters
  {
    bool dflt = false;
    try { icsCfg->useCache = cfgSec.getBool("useCache");
    } catch (const ConfigException & exc) {
      icsCfg->useCache = dflt = true;
    }
    smsc_log_info(logger, "  useCache: %s%s", icsCfg->useCache ? "true" : "false",
                  dflt ? " (default)":"");
  }
  if (icsCfg->useCache) {
    icsDeps.insert(ICSIdent::icsIdAbntCache);

    tmo = 0;
    try { tmo = (uint32_t)cfgSec.getInt("cacheExpiration");
    } catch (const ConfigException & exc) {
      tmo = AbonentCacheCFG::_MAX_CACHE_INTERVAL;
    }
    if (tmo >= AbonentCacheCFG::_MAX_CACHE_INTERVAL)
      throw ConfigException("'cacheExpiration' is invalid or missing,"
                              " allowed range [1..%u)", AbonentCacheCFG::_MAX_CACHE_INTERVAL);
    icsCfg->cacheTmo = tmo ? tmo : AbonentCacheCFG::_DFLT_CACHE_INTERVAL;
    smsc_log_info(logger, "  cacheExpiration: %u minutes%s", icsCfg->cacheTmo,
                  !tmo ? " (default)":"");
    icsCfg->cacheTmo *= 60;         //convert minutes to seconds
  }
  /**/
  return ICSrvCfgReaderAC::cfgComplete;
}

} //abdtcr
} //inman
} //smsc


