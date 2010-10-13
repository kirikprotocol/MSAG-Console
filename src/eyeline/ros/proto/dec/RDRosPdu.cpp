#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/ros/proto/dec/RDRosPdu.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace dec {

const asn1::ASTag  RDRosPdu::_tagInvoke(asn1::ASTag::tagContextSpecific, 1);
const asn1::ASTag  RDRosPdu::_tagResult(asn1::ASTag::tagContextSpecific, 2);
const asn1::ASTag  RDRosPdu::_tagError(asn1::ASTag::tagContextSpecific, 3);
const asn1::ASTag  RDRosPdu::_tagReject(asn1::ASTag::tagContextSpecific, 4);
const asn1::ASTag  RDRosPdu::_tagResultNL(asn1::ASTag::tagContextSpecific, 7);

/* ROS PDU is defined in IMPLICIT TAGS environment as following CHOICE:

ROS ::= CHOICE {
  invoke              [1]  Invoke,
  returnResult        [2]  ReturnResult,
  returnError         [3]  ReturnError,
  reject              [4]  Reject
  returnResultNotLast [7]  ReturnResult
} */
//Initializes ElementDecoder for this type
void RDRosPdu::construct(void)
{
  asn1::ber::DecoderOfChoice_T<5>::setAlternative(0, _tagInvoke, asn1::ASTagging::tagsIMPLICIT);
  asn1::ber::DecoderOfChoice_T<5>::setAlternative(1, _tagResult, asn1::ASTagging::tagsIMPLICIT);
  asn1::ber::DecoderOfChoice_T<5>::setAlternative(2, _tagError, asn1::ASTagging::tagsIMPLICIT);
  asn1::ber::DecoderOfChoice_T<5>::setAlternative(3, _tagReject, asn1::ASTagging::tagsIMPLICIT);
  asn1::ber::DecoderOfChoice_T<5>::setAlternative(4, _tagResultNL, asn1::ASTagging::tagsIMPLICIT);
}

// ----------------------------------------
// -- DecoderOfChoiceAC interface methods
// ----------------------------------------
//If necessary, allocates alternative and initializes associated TypeDecoderAC
asn1::ber::TypeDecoderAC *
  RDRosPdu::prepareAlternative(uint16_t unique_idx) /*throw(std::exception)*/
{
  if (!_dVal) //assertion!!!
    throw smsc::util::Exception("ros::proto::dec::RDRosPdu : value isn't set!");
  if (unique_idx > 4) //assertion!!!
    throw smsc::util::Exception("ros::proto::dec::RDRosPdu::prepareAlternative() : undefined UId");

  switch (unique_idx) {
  case 4:
    _alt.returnResultNL().init(_tagResultNL, asn1::ASTagging::tagsIMPLICIT, getTSRule()).setValue(_dVal->resultNL().init());
    break;
  case 3:
    _alt.reject().init(_tagReject, asn1::ASTagging::tagsIMPLICIT, getTSRule()).setValue(_dVal->reject().init());
    break;
  case 2:
    _alt.returnError().init(_tagError, asn1::ASTagging::tagsIMPLICIT, getTSRule()).setValue(_dVal->error().init());
    break;
  case 1:
    _alt.returnResult().init(_tagResult, asn1::ASTagging::tagsIMPLICIT, getTSRule()).setValue(_dVal->result().init());
    break;
  default: //case 0:
    _alt.invoke().init(_tagInvoke, asn1::ASTagging::tagsIMPLICIT, getTSRule()).setValue(_dVal->invoke().init());
  }
  return _alt.get();
}

}}}}
