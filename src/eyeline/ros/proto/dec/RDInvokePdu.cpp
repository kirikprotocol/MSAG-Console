#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/ros/proto/dec/RDInvokePdu.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace dec {

const asn1::ASTag RDInvokePdu::_pduTag(asn1::ASTag::tagContextSpecific, 1);

/* ROS::Invoke PDU is defined in IMPLICIT tagging environment as follow:

Invoke ::= [1] SEQUENCE {
  invokeId    InvokeIdType,
  linkedId    LinkedIdType OPTIONAL,
  opcode      INTEGER,
  argument    ABSTRACT-SYNTAX.&Type({Operations}) OPTIONAL
} */
//Initializes ElementDecoder for this type
void RDInvokePdu::construct(void)
{
  asn1::ber::DecoderOfSequence_T<4>::setField(0, asn1::_tagINTEGER, asn1::ber::EDAlternative::altMANDATORY);
  asn1::ber::DecoderOfSequence_T<4>::setField(1, RDLinkedIdType::_tagOptions, asn1::ber::EDAlternative::altOPTIONAL);
  asn1::ber::DecoderOfSequence_T<4>::setField(2, asn1::_tagINTEGER, asn1::ber::EDAlternative::altMANDATORY);
  asn1::ber::DecoderOfSequence_T<4>::setField(3, asn1::ber::EDAlternative::altOPTIONAL);
}

void RDInvokePdu::setLinkedId(ros::LinkedId & link_id) /*throw(std::exception)*/
{
  if (!_linkedId)
    _linkedId = new (_memLinkId._buf) RDLinkedIdType(link_id, getVALRule());
  else
    _linkedId->setValue(link_id);
}

void RDInvokePdu::setArgType(PDUArgument & use_arg) /*throw(std::exception)*/
{
  if (!_argType)
    _argType = new (_memArgType._buf) asn1::ber::DecoderOfASType(getTSRule());
  _argType->setValue(use_arg._tsEnc);
}

// ----------------------------------------
// -- DecoderOfStructAC interface methods
// ----------------------------------------
//If necessary, allocates optional element and initializes associated TypeDecoderAC
asn1::ber::TypeDecoderAC * 
  RDInvokePdu::prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */
{
  if (!_dVal) //assertion!!!
    throw smsc::util::Exception("ros::proto::dec::RDInvokePdu : value isn't set!");

  if (unique_idx > 3)
    throw smsc::util::Exception("ros::proto::dec::RDInvokePdu::prepareAlternative() : undefined UId");

  if (!unique_idx) {
    _invId.setValue(_dVal->getHeader()._invId);
    return &_invId;
  }
  if (unique_idx == 1) {
    setLinkedId(_dVal->getParam()._linked);
    return _linkedId;
  }
  if (unique_idx == 2) {
    _opCode.setValue(_dVal->getHeader()._opCode);
    return &_opCode;
  }
  //if (unique_idx == 3)
  setArgType(_dVal->getArg());
  return _argType;
}
//Performs actions upon successfull optional element decoding
void RDInvokePdu::markDecodedOptional(uint16_t unique_idx) /*throw() */
{
  if (unique_idx == 3)
    _dVal->getArg()._kind = ros::PDUArgument::asvTSyntax;
}

}}}}

