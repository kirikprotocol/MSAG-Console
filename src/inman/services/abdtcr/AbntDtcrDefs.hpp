/* ************************************************************************* *
 * Abonent Detector service types and helpers definitions
 * ************************************************************************* */
#ifndef __INMAN_ICS_ABNT_DETECTOR_DEFS__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ICS_ABNT_DETECTOR_DEFS__

#include "inman/services/ICSrvDefs.hpp"

namespace smsc {
namespace inman {
namespace abdtcr {

using smsc::inman::ICSIdsSet;

//Abonent Detector service configuration parameters (from config.xml)
struct AbntDetectorXCFG {
    std::string policyNm;       //name of default AbonenPolicy
    uint16_t    abtTimeout;     //maximum timeout on abonent type requests,
                                //(Abonentprovider interaction)
    uint16_t    maxRequests;    //maximum number of requests per connect
    bool        useCache;       //use abonents contract data cache
    uint32_t    cacheTmo;       //abonent cache data expiration timeout in secs
    ICSIdsSet   icsDeps;        //ICServices this one depends on

    AbntDetectorXCFG()
        : useCache(false)
    {
        cacheTmo = maxRequests = abtTimeout = 0;
    }
};

} //abdtcr
} //inman
} //smsc

#endif /* __INMAN_ICS_ABNT_DETECTOR_DEFS__ */

