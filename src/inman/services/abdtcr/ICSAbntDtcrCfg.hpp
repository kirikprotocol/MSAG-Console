/* ************************************************************************* *
 * Abonent Detector service configuration parameters definition.
 * ************************************************************************* */
#ifndef __INMAN_ICS_ABNT_DETECTOR_DEFS__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ICS_ABNT_DETECTOR_DEFS__

#include "inman/services/ICSrvIDs.hpp"

namespace smsc {
namespace inman {
namespace abdtcr {

using smsc::inman::ICSIdsSet;

//Abonent Detector service configuration parameters (from config.xml)
struct AbntDetectorXCFG {
  std::string policyNm;       //name of default AbonenPolicy
  uint16_t    abtTimeout;     //maximum timeout on abonent type requests,
                              //(Abonentprovider interaction)
  uint32_t    maxRequests;    //maximum number of requests per connect
  uint16_t    maxThreads;     //maximum number of threads per connect, 0 - means no limitation
  bool        useCache;       //use abonents contract data cache
  uint32_t    cacheTmo;       //abonent cache data expiration timeout in secs
  ICSIdsSet   icsDeps;        //ICServices this one depends on

  AbntDetectorXCFG() : abtTimeout(0), maxRequests(0), maxThreads(0)
    , useCache(false), cacheTmo(0)
  { }
};

} //abdtcr
} //inman
} //smsc

#endif /* __INMAN_ICS_ABNT_DETECTOR_DEFS__ */

