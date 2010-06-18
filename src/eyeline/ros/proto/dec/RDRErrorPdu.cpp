#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/ros/proto/dec/RDRErrorPdu.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace dec {

const asn1::ASTag RDRErrorPdu::_pduTag(asn1::ASTag::tagContextSpecific, 3);

/* ROS ReturnError PDU is defined in IMPLICIT tagging environment as follow:
ReturnError ::= [3] SEQUENCE {
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

void RDRErrorPdu::setArgType(PDUArgument & use_arg) /*throw(std::exception)*/
{
  if (!_argType)
    _argType = new (_memArg._buf) asn1::ber::DecoderOfASType(getTSRule());
  _argType->setValue(use_arg._tsEnc);
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
  setArgType(_dVal->getArg());
  return _argType;
}

//Performs actions upon successfull optional element decoding
void RDRErrorPdu::markDecodedOptional(uint16_t unique_idx) /*throw() */
{
  if (unique_idx == 2)
    _dVal->getArg()._kind = ros::PDUArgument::asvTSyntax;
}

}}}}

