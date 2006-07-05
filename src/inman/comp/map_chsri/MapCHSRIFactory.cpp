static char const ident[] = "$Id$";

#include "inman/comp/map_chsri/MapCHSRIFactory.hpp"
#include "inman/comp/map_chsri/MapCHSRIComps.hpp"

using smsc::inman::comp::CompFactory;

namespace smsc {
namespace inman {
namespace comp {
namespace chsri {

//FactoryInitFunction implementation
OperationFactory * initMAPCHSRI3Components(OperationFactory * fact)
{
    if (!fact) { //called from ApplicationContextFactory::getFactory()
        fact = MAPCHSRI3Factory::getInstance(); //getInstance() calls FIF in turn
    } else {
        fact->setLogger(Logger::getInstance("smsc.inman.comp.chsri"));
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
    }
    return fact;
}

} //chsri
} //comp
} //inman
} //smsc

