#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/comp/map_chsri/MapCHSRIFactory.hpp"
#include "inman/comp/map_chsri/MapCHSRIComps.hpp"

using smsc::inman::comp::CompFactory;

namespace smsc {
namespace inman {
namespace comp {

static uint8_t _octs_map_locInfoRetrieval_v3[] = //id_ac_map_locInfoRetrieval_v3:
//  { itu-t(0) identified-organization(4) etsi(0) mobileDomain(0) gsm-Network(1)
//    ac-Id(0) locInfoRetrieval(5) version3(3) }
    {7, 0x04, 0x00, 0x00, 0x01, 0x00, 0x05, 0x03};

EncodedOID _ac_map_locInfoRetrieval_v3(_octs_map_locInfoRetrieval_v3,
                                       "ac_map_locInfoRetrieval_v3");

namespace chsri {
//ROSFactoryProducer implementation
ROSComponentsFactory * initMAPCHSRI3Components(void)
{
    ROSComponentsFactory * fact = new ROSComponentsFactory(_ac_map_locInfoRetrieval_v3);

    fact->registerArg(MAP_CH_SRI_OpCode::sendRoutingInfo,
      new CompFactory::ProducerT<smsc::inman::comp::chsri::CHSendRoutingInfoArg>() );
    fact->registerRes(MAP_CH_SRI_OpCode::sendRoutingInfo,
      new CompFactory::ProducerT<smsc::inman::comp::chsri::CHSendRoutingInfoRes>() );

    fact->bindErrors(MAP_CH_SRI_OpCode::sendRoutingInfo, 15, 
                     ERR_CHSendRountigInfo::systemFailure,
                     ERR_CHSendRountigInfo::dataMissing,
                     ERR_CHSendRountigInfo::unexpectedDataValue,
                     ERR_CHSendRountigInfo::facilityNotSupported,
                     ERR_CHSendRountigInfo::or_NotAllowed,
                     ERR_CHSendRountigInfo::unknownSubscriber,
                     ERR_CHSendRountigInfo::numberChanged,
                     ERR_CHSendRountigInfo::bearerServiceNotProvisioned,
                     ERR_CHSendRountigInfo::teleserviceNotProvisioned,
                     ERR_CHSendRountigInfo::absentSubscriber,
                     ERR_CHSendRountigInfo::busySubscriber,
                     ERR_CHSendRountigInfo::noSubscriberReply,
                     ERR_CHSendRountigInfo::callBarred,
                     ERR_CHSendRountigInfo::cug_Reject,
                     ERR_CHSendRountigInfo::forwardingViolation
                     );
    return fact;
}

} //chsri
} //comp
} //inman
} //smsc

