#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/comp/map_atih/MapATIHFactory.hpp"
#include "inman/comp/map_atih/MapATSIComps.hpp"

using smsc::inman::comp::CompFactory;

namespace smsc {
namespace inman {
namespace comp {

static uint8_t _octs_map_ATIH_v3[] =
//id_ac_map_anyTimeInfoHandling_v3:
//  { itu-t(0) identified-organization(4) etsi(0) mobileDomain(0) gsm-Network(1)
//    ac-Id(0) anyTimeInfoHandling(43) version3(3) }
    {7, 0x04, 0x00, 0x00, 0x01, 0x00, 0x2B, 0x03};

EncodedOID _ac_map_anyTimeInfoHandling_v3(_octs_map_ATIH_v3, "ac_map_ATIH_v3");

namespace atih {
//ROSFactoryProducer implementation
ROSComponentsFactory * initMAPATIH3Components(void)
{
    ROSComponentsFactory * fact = new ROSComponentsFactory(_ac_map_anyTimeInfoHandling_v3);
    fact->registerArg(MAP_ATIH::anyTimeSubscriptionInterrogation,
      new CompFactory::ProducerT<smsc::inman::comp::atih::ATSIArg>() );
    fact->registerRes(MAP_ATIH::anyTimeSubscriptionInterrogation,
      new CompFactory::ProducerT<smsc::inman::comp::atih::ATSIRes>() );
    fact->bindErrors(MAP_ATIH::anyTimeSubscriptionInterrogation, 10,
                     ERR_ATSI::unknownSubscriber,
                     ERR_ATSI::bearerServiceNotProvisioned,
                     ERR_ATSI::teleserviceNotProvisioned,
                     ERR_ATSI::callBarred,
                     ERR_ATSI::illegalSSOperation,
                     ERR_ATSI::ssNotAvailable,
                     ERR_ATSI::dataMissing,
                     ERR_ATSI::unexpectedDataValue,
                     ERR_ATSI::atsi_NotAllowed,
                     ERR_ATSI::informationNotAvailable
                     );
    return fact;
}

} //atih
} //comp
} //inman
} //smsc

