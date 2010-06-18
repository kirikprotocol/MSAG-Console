#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/ros/proto/enc/REProblemType.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace enc {

const asn1::ASTagging
  REProblemType::_tagsGeneralProblem(asn1::ASTag::tagContextSpecific,
                             0, asn1::ASTagging::tagsIMPLICIT);

const asn1::ASTagging
  REProblemType::_tagsInvokeProblem(asn1::ASTag::tagContextSpecific,
                             1, asn1::ASTagging::tagsIMPLICIT);

const asn1::ASTagging
  REProblemType::_tagsResultProblem(asn1::ASTag::tagContextSpecific,
                               2, asn1::ASTagging::tagsIMPLICIT);

const asn1::ASTagging
  REProblemType::_tagsErrorProblem(asn1::ASTag::tagContextSpecific,
                              3, asn1::ASTagging::tagsIMPLICIT);


void REProblemType::setValue(const ros::RejectProblem & use_val) /*throw(std::exception)*/
{
  switch (use_val.getProblemKind()) {
  case ros::RejectProblem::rejInvoke:
    initAlt(_tagsInvokeProblem)->setValue((uint8_t)use_val.getInvokeProblem());
    break;
  case ros::RejectProblem::rejResult:
    initAlt(_tagsResultProblem)->setValue((uint8_t)use_val.getRResultProblem());
    break;
  case ros::RejectProblem::rejError:
    initAlt(_tagsErrorProblem)->setValue((uint8_t)use_val.getRErrorProblem());
    break;
  default: //ros::RejectProblem::rejGeneral:
    initAlt(_tagsGeneralProblem)->setValue((uint8_t)use_val.getGeneralProblem());
  }
}


}}}}
