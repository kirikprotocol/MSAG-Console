#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/map/ac/nUSS_v2/ROSFactory.hpp"


namespace eyeline {
namespace map {
namespace ac {

static uint8_t _octs_map_networkUSS_v2[] = //id_ac_map_networkUnstructuredSs_v2:
//  { itu-t(0) identified-organization(4) etsi(0) mobileDomain(0) gsm-Network(1)
//    ac-Id(0) networkUnstructuredSs(19) version2(2) }
    {7, 0x04, 0x00, 0x00, 0x01, 0x00, 0x13, 0x02};

asn1::EncodedOID _ac_map_networkUnstructuredSs_v2(_octs_map_networkUSS_v2,
                                            "ac_map_networkUSS_v2");

namespace nuss_v2 {

//Function that creates ROSComponentsFactory for nUSS_v2
eyeline::ros::ROSComponentsFactory * initROSFactory_nUSS_v2(void)
{
  std::auto_ptr<eyeline::ros::ROSComponentsFactory> 
    pFact(new eyeline::ros::ROSComponentsFactory(_ac_map_networkUnstructuredSs_v2));

  //register ARGUMENT
  pFact->registerArg(nUSS_OpCodes_v2::oc_processUSS_Request,
                     new eyeline::ros::ROSFactory::ProducerT<AV_USSD_Arg>());
  //register RESULT
  pFact->registerRes(nUSS_OpCodes_v2::oc_processUSS_Request,
                     new eyeline::ros::ROSFactory::ProducerT<AV_USSD_Res>());
  //register ERROR codes
  pFact->bindErrors(nUSS_OpCodes_v2::oc_processUSS_Request, 5,
                     ERRORS_ProcessUSS_Request::me_callBarred,
                     ERRORS_ProcessUSS_Request::me_dataMissing,
                     ERRORS_ProcessUSS_Request::me_systemFailure,
                     ERRORS_ProcessUSS_Request::me_unexpectedDataValue,
                     ERRORS_ProcessUSS_Request::me_unknownAlphabet
                     );
  //register ERROR parameters
  /* TODO: 
  pFact->registerErr(ERRORS_ProcessUSS_Request::me_callBarred,
                     new eyeline::ros::ROSFactory::ProducerT<AV_CallBarredParam>());
  pFact->registerErr(ERRORS_ProcessUSS_Request::me_systemFailure,
                     new eyeline::ros::ROSFactory::ProducerT<AV_SystemFailureParam>());
  pFact->registerErr(ERRORS_ProcessUSS_Request::me_dataMissing,
                     new eyeline::ros::ROSFactory::ProducerT<AV_DataMissingParam>());
  pFact->registerErr(ERRORS_ProcessUSS_Request::me_unexpectedDataValue,
                     new eyeline::ros::ROSFactory::ProducerT<AV_UnexpectedDataParam>());
  */
  return pFact.release();
}

} //nUSS_v2

}}}

