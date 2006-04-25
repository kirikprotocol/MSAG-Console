static char const ident[] = "$Id$";

#include "inman/comp/map_atih/MapATIHFactory.hpp"
#include "inman/comp/map_atih/MapATSIComps.hpp"

using smsc::inman::comp::CompFactory;

namespace smsc {
namespace inman {
namespace comp {
namespace atih {

//FactoryInitFunction implementation
OperationFactory * initMAPATIH3Components(OperationFactory * fact)
{
    if (!fact) { //called from ApplicationContextFactory::getFactory()
        fact = MAPATIH3Factory::getInstance(); //getInstance() calls FIF in turn
    } else {
        fact->setLogger(Logger::getInstance("smsc.inman.comp.atih.Factory"));
        fact->registerArg(MAPATIH_OpCode::anyTimeSubscriptionInterrogation,
          new CompFactory::ProducerT<smsc::inman::comp::atih::ATSIArg>() );
        fact->registerRes(MAPATIH_OpCode::anyTimeSubscriptionInterrogation,
          new CompFactory::ProducerT<smsc::inman::comp::atih::ATSIRes>() );
        fact->bindErrors(MAPATIH_OpCode::anyTimeSubscriptionInterrogation, 10, 
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
    }
    return fact;
}

} //atih
} //comp
} //inman
} //smsc

