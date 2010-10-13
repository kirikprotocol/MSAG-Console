#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/ros/proto/dec/RDRErrorPdu.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace dec {

/* ROS ReturnError PDU is defined in IMPLICIT tagging environment as follow:
ReturnError ::= SEQUENCE {
    invokeId	InvokeIdType,
    errcode	INTEGER,
    parameter	ABSTRACT-SYNTAX.&Type({Errors}) OPTIONAL
} */
//Initializes ElementDecoder for this type
void RDRErrorPdu::construct(void)
{
  asn1::ber::DecoderOfSequence_T<3>::setField(0, asn1::_tagINTEGER, asn1::ber::EDAlternative::altMANDATORY);
  asn1::ber::DecoderOfSequence_T<3>::setField(1, asn1::_tagINTEGER, asn1::ber::EDAlternative::altMANDATORY);
  asn1::ber::DecoderOfSequence_T<3>::setField(2, asn1::ber::EDAlternative::altOPTIONAL);
}

// ----------------------------------------
// -- DecoderOfStructAC interface methods
// ----------------------------------------
//If necessary, allocates optional element and initializes associated TypeDecoderAC
asn1::ber::TypeDecoderAC * 
  RDRErrorPdu::prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */
{
  if (!_dVal) //assertion!!!
    throw smsc::util::Exception("ros::proto::dec::RDRErrorPdu : value isn't set!");
  if (unique_idx > 2)
    throw smsc::util::Exception("ros::proto::dec::RDRErrorPdu::prepareAlternative() : undefined UId");

  if (!unique_idx) {
    _invId.setValue(_dVal->getHeader()._invId);
    return &_invId;
  }
  if (unique_idx == 1) {
    _errCode.setValue(_dVal->getHeader()._opCode);
    return &_errCode;
  }
  //if (unique_idx == 2)
  _argument.init(getTSRule()).setValue(_dVal->getArg());
  return _argument.get();
}

}}}}

