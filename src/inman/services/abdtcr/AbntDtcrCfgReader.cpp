#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/abdtcr/AbntDtcrCfgReader.hpp"

using smsc::inman::cache::AbonentCacheCFG;

namespace smsc {
namespace inman {
namespace abdtcr {
/* ************************************************************************** *
 * class ICSAbntDetectorCfgReader implementation:
 * ************************************************************************** */
  //Parses XML configuration entry section, updates dependencies.
  //Returns status of config parsing, 
ICSrvCfgReaderAC::CfgState
  ICSAbntDetectorCfgReader::parseConfig(void * opaque_arg/* = NULL*/)
    throw(ConfigException)
{
  uint32_t      tmo = 0;
  const char *  cstr = NULL;

  //abonent contract determination policy is required
  try { cstr = _topSec.getString("abonentPolicy");
  } catch (const ConfigException & exc) { }
  if (!cstr || !cstr[0])
    throw ConfigException("default abonent policy is not set!");
  smsc_log_info(logger, "  abonent policy %s", cstr);
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

  tmo = 0;    //maxRequests
  try { tmo = (uint32_t)_topSec.getInt("maxRequests");
  } catch (const ConfigException & exc) { tmo = _MAX_REQUESTS_NUM + 1; }
  if (tmo > _MAX_REQUESTS_NUM)
    throw ConfigException("'maxRequests' is invalid or missing,"
                            " allowed range [1..%u)", _MAX_REQUESTS_NUM);
  icsCfg->maxRequests = tmo ? tmo : _DFLT_REQUESTS_NUM;
  smsc_log_info(logger, "  maxRequests: %u per connect%s", icsCfg->maxRequests,
                !tmo ? " (default)":"");

  tmo = 0;
  try { tmo = (uint32_t)_topSec.getInt("maxThreads");
  } catch (const ConfigException & exc) { tmo = _DFLT_THREADS_NUM; }
  if (tmo > _MAX_THREADS_NUM)
    throw ConfigException("maxThreads is out of range [0 ..65535]");

  icsCfg->maxThreads = (uint16_t)tmo;
  if (!tmo) {
    smsc_log_info(logger, "  maxThreads: unlimited per connect");
  } else {
    smsc_log_info(logger, "  maxThreads: %u per connect%s", (unsigned)icsCfg->maxThreads,
                  (tmo == _DFLT_THREADS_NUM) ? " (default)":"");
  }

  //cache parameters
  {
    bool dflt = false;
    try { icsCfg->useCache = _topSec.getBool("useCache");
    } catch (const ConfigException & exc) {
      icsCfg->useCache = dflt = true;
    }
    smsc_log_info(logger, "  useCache: %s%s", icsCfg->useCache ? "true" : "false",
                  dflt ? " (default)":"");
  }
  if (icsCfg->useCache) {
    icsDeps.insert(ICSIdent::icsIdAbntCache);

    tmo = 0;
    try { tmo = (uint32_t)_topSec.getInt("cacheExpiration");
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


