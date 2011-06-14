#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "ussman/comp/map_uss/MapUSSFactory.hpp"
#include "ussman/comp/map_uss/MapUSSComps.hpp"

using smsc::inman::comp::CompFactory;

namespace smsc {
namespace ussman {
namespace comp {

static uint8_t _octs_map_networkUSS_v2[] = //id_ac_map_networkUnstructuredSs_v2:
//  { itu-t(0) identified-organization(4) etsi(0) mobileDomain(0) gsm-Network(1)
//    ac-Id(0) networkUnstructuredSs(19) version2(2) }
    {7, 0x04, 0x00, 0x00, 0x01, 0x00, 0x13, 0x02};

EncodedOID _ac_map_networkUnstructuredSs_v2(_octs_map_networkUSS_v2,
                                            "ac_map_networkUSS_v2");

namespace uss {

//ROSFactoryProducer implementation
ROSComponentsFactory * initMAPUSS2Components(void)
{
    ROSComponentsFactory * fact = new ROSComponentsFactory(_ac_map_networkUnstructuredSs_v2);
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
    return fact;
}

} //uss
} //comp
} //ussman
} //smsc

