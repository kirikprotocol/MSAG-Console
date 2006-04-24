static char const ident[] = "$Id$";

#include "inman/comp/map_uss/MapUSSFactory.hpp"
#include "inman/comp/map_uss/MapUSSComps.hpp"

namespace smsc {
namespace inman {
namespace comp {
namespace uss {

//FactoryInitFunction implementation
OperationFactory * initMAPUSS2Components(OperationFactory * fact)
{
    if (!fact) { //called from ApplicationContextFactory::getFactory()
        fact = MAPUSS2Factory::getInstance(); //getInstance() calls FIF in turn
    } else {
        fact->setLogger(Logger::getInstance("smsc.inman.usscomp.ComponentFactory"));
        fact->registerArg(MAPUSS_OpCode::processUSS_Request,
          new CompFactory::ProducerT<ProcessUSSRequestArg>() );
        fact->registerRes(MAPUSS_OpCode::processUSS_Request,
          new CompFactory::ProducerT<ProcessUSSRequestRes>() );
        fact->bindErrors(MAPUSS_OpCode::processUSS_Request, 5, 
                         ERR_ProcessUSS_Request::callBarred,
                         ERR_ProcessUSS_Request::dataMissing,
                         ERR_ProcessUSS_Request::systemFailure,
                         ERR_ProcessUSS_Request::unexpectedDataValue,
                         ERR_ProcessUSS_Request::unknownAlphabet
                         );
    }
    return fact;
}

} //uss
} //comp
} //inman
} //smsc

