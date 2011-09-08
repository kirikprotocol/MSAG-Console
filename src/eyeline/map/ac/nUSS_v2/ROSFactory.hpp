/* ************************************************************************* *
 * Factory of ROS Components(ARGUMENT, RESULT, ERRORS) of OPERATIONs defined 
 * in v2 of Network Unstructured Supplementary Service Application Context.
 * ************************************************************************* */
#ifndef __EYELINE_MAP_AC_NUSS_V2_ROS_FACTORY_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_MAP_AC_NUSS_V2_ROS_FACTORY_HPP

#include "eyeline/asn1/EncodedOID.hpp"

#include "eyeline/asn1/AbstractValue.hpp"
#include "eyeline/ros/ROSCompFactory.hpp"

#include "eyeline/map/7_15_0/proto/ERR/ErrorCodes.hpp"

#include "eyeline/map/7_15_0/proto/SS/USSD-Arg.hpp"
#include "eyeline/map/7_15_0/proto/SS/enc/MEUSSD-Arg.hpp"
#include "eyeline/map/7_15_0/proto/SS/dec/MDUSSD-Arg.hpp"

#include "eyeline/map/7_15_0/proto/SS/USSD-Res.hpp"
#include "eyeline/map/7_15_0/proto/SS/enc/MEUSSD-Res.hpp"
#include "eyeline/map/7_15_0/proto/SS/dec/MDUSSD-Res.hpp"


namespace eyeline {
namespace map {
namespace ac {

//Encoded representation of Application Context OID
extern asn1::EncodedOID _ac_map_networkUnstructuredSs_v2;
//Function that creates ROSComponentsFactory for nUSS_v2
extern eyeline::ros::ROSComponentsFactory * initROSFactory_nUSS_v2(void);

namespace nuss_v2 {

//Operation codes of defined OPERATIONs
struct nUSS_OpCodes_v2 {
  enum Id_e {
    oc_processUSS_Request = 59
  , oc_USS_Request = 60
  , oc_USS_Notify = 61
  };
};

/* ------------------------------------------------------- *
 * ProcessUSS-Request OPERATION components
 * ------------------------------------------------------- */
typedef asn1::ASTypeValue_T<
  eyeline::map::ss::USSD_Arg,
  eyeline::map::ss::enc::MEUSSD_Arg, eyeline::map::ss::dec::MDUSSD_Arg
> AV_USSD_Arg;  //ARGUMENT

typedef asn1::ASTypeValue_T<
  eyeline::map::ss::USSD_Res,
  eyeline::map::ss::enc::MEUSSD_Res, eyeline::map::ss::dec::MDUSSD_Res
> AV_USSD_Res;  //RESULT

//Error codes of defined ERRORs of processUSS-Request OPERATION
struct ERRORS_ProcessUSS_Request {
  enum Id_e {
    me_callBarred          = eyeline::map::err::MAPErrorCode::me_callBarred,
    me_systemFailure       = eyeline::map::err::MAPErrorCode::me_systemFailure,
    me_dataMissing         = eyeline::map::err::MAPErrorCode::me_dataMissing,
    me_unexpectedDataValue = eyeline::map::err::MAPErrorCode::me_unexpectedDataValue,
    me_unknownAlphabet     = eyeline::map::err::MAPErrorCode::me_unknownAlphabet
  };
};


/* TODO: define ERRORs params:
 
  typedef asn1::ASTypeValue_T<
    eyeline::map::err::CallBarredParam,
    eyeline::map::err::enc::MECallBarredParam,
    eyeline::map::err::dec::MDCallBarredParam
  > AV_CallBarredParam;
 
  typedef asn1::ASTypeValue_T<
    eyeline::map::err::SystemFailureParam,
    eyeline::map::err::enc::MESystemFailureParam,
    eyeline::map::err::dec::MDSystemFailureParam
  > AV_SystemFailureParam;
 
  typedef asn1::ASTypeValue_T<
    eyeline::map::err::DataMissingParam,
    eyeline::map::err::enc::MEDataMissingParam,
    eyeline::map::err::dec::MDDataMissingParam
  > AV_DataMissingParam;
 
  typedef asn1::ASTypeValue_T<
    eyeline::map::err::UnexpectedDataParam,
    eyeline::map::err::enc::MEUnexpectedDataParam,
    eyeline::map::err::dec::MDUnexpectedDataParam
  > AV_UnexpectedDataParam;
*/

/* ------------------------------------------------------- *
 * USS-Request OPERATION components
 * ------------------------------------------------------- */
//TODO:

/* ------------------------------------------------------- *
 * USS-Notify OPERATION components
 * ------------------------------------------------------- */
//TODO:

} // nUSS_v2

}}}

#endif /* __EYELINE_MAP_AC_NUSS_V2_ROS_FACTORY_HPP */

