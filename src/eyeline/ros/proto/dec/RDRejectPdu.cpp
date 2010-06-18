#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/ros/proto/dec/RDRejectPdu.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace dec {

const asn1::ASTag RDRejectPdu::_pduTag(asn1::ASTag::tagContextSpecific, 4);

/* ROS::Reject PDU is defined in IMPLICIT tagging environment as following:

Reject ::= [4] SEQUENCE {
    invokeId	InvokeId,
    problem     ProblemType
} */
//Initializes ElementDecoder for this type
void RDRejectPdu::construct(void)
{
  asn1::ber::DecoderOfSequence_T<2>::setField(0, asn1::_tagINTEGER, asn1::ber::EDAlternative::altMANDATORY);
  asn1::ber::DecoderOfSequence_T<2>::setField(1, RDProblemType::_tagOptions, asn1::ber::EDAlternative::altMANDATORY);
}

// ----------------------------------------
// -- DecoderOfStructAC interface methods
// ----------------------------------------
//If necessary, allocates optional element and initializes associated TypeDecoderAC
asn1::ber::TypeDecoderAC * 
  RDRejectPdu::prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */
{
  if (!_dVal) //assertion!!!
    throw smsc::util::Exception("ros::proto::dec::RDRejectPdu : value isn't set!");
  if (unique_idx > 1)
    throw smsc::util::Exception("ros::proto::dec::RDRejectPdu::prepareAlternative() : undefined UId");

  if (!unique_idx) {
    _invId.setValue(_dVal->getHeader()._invId);
    return &_invId;
  }
  //if (unique_idx == 1)
  _problem.setValue(_dVal->getParam());
  return &_problem;
}

}}}}

