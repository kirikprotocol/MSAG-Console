#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/ros/proto/enc/REProblemType.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace enc {

const asn1::ASTag REProblemType::_tagGeneralProblem(asn1::ASTag::tagContextSpecific, 0);
const asn1::ASTag REProblemType::_tagInvokeProblem(asn1::ASTag::tagContextSpecific, 1);
const asn1::ASTag REProblemType::_tagResultProblem(asn1::ASTag::tagContextSpecific, 2);
const asn1::ASTag REProblemType::_tagErrorProblem(asn1::ASTag::tagContextSpecific, 3);

//asn1::ASTagging::tagsIMPLICIT

void REProblemType::setValue(const ros::RejectProblem & use_val) /*throw(std::exception)*/
{
  switch (use_val.getProblemKind()) {
  case ros::RejectProblem::rejInvoke:
    _pEnc.init(_tagInvokeProblem, asn1::ASTagging::tagsIMPLICIT, getTSRule()).setValue(use_val.getProblem());
    break;
  case ros::RejectProblem::rejResult:
    _pEnc.init(_tagResultProblem, asn1::ASTagging::tagsIMPLICIT, getTSRule()).setValue(use_val.getProblem());
    break;
  case ros::RejectProblem::rejError:
    _pEnc.init(_tagErrorProblem, asn1::ASTagging::tagsIMPLICIT, getTSRule()).setValue(use_val.getProblem());
    break;
  default: //ros::RejectProblem::rejGeneral:
    _pEnc.init(_tagGeneralProblem, asn1::ASTagging::tagsIMPLICIT, getTSRule()).setValue(use_val.getProblem());
  }
}


}}}}
