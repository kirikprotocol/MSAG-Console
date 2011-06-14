/* ************************************************************************* *
 * AbonentDetector request processing parameters.
 * ************************************************************************* */
#ifndef __INMAN_ABNT_DETECTOR_CFG_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ABNT_DETECTOR_CFG_HPP

#include "inman/incache/AbCacheDefs.hpp"
#include "inman/services/iapmgr/IAPMgrDefs.hpp"
#include "inman/services/tmwatch/TimeWatcher.hpp"
#include "inman/services/abdtcr/ICSAbntDtcrCfg.hpp"

namespace smsc {
namespace inman {
namespace abdtcr {

using smsc::inman::cache::AbonentCacheITF;

using smsc::inman::iapmgr::IAPManagerITF;
using smsc::inman::iapmgr::AbonentPolicy;
using smsc::inman::AbonentPolicyName_t;

using smsc::core::timers::TimeoutHDL;

struct AbonentDetectorCFG {
  bool                useCache;       //use abonents contract data cache
  AbonentCacheITF *   abCache;
  const IAPManagerITF * iapMgr;

  AbonentPolicyName_t policyNm;       //name of default AbonenPolicy
  TimeoutHDL          abtTimeout;     //maximum timeout on abonent type requests,
                                      //(Abonentprovider interaction)
  uint32_t            maxRequests;    //maximum number of requests per connect
  uint16_t            maxThreads;     //maximum number of threads per connect, 0 - means no limitation
  uint32_t            cacheTmo;       //abonent cache data expiration timeout in secs

  AbonentDetectorCFG()
    : useCache(false), abCache(0), iapMgr(0)
    , abtTimeout(0), maxRequests(0), maxThreads(0), cacheTmo(0)
  { }
  AbonentDetectorCFG(const AbntDetectorXCFG & use_xcfg)
    : useCache(use_xcfg.useCache), abCache(0), iapMgr(0), policyNm(use_xcfg.policyNm.c_str())
    , abtTimeout(use_xcfg.abtTimeout), maxRequests(use_xcfg.maxRequests)
    , maxThreads(use_xcfg.maxThreads), cacheTmo(use_xcfg.cacheTmo)
  { }
};


} //abdtcr
} //inman
} //smsc

#endif /* __INMAN_ABNT_DETECTOR_CFG_HPP */

