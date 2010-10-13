#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/ros/proto/dec/RDInvokePdu.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace dec {

/* ROS::Invoke PDU is defined in IMPLICIT tagging environment as follow:

Invoke ::= SEQUENCE {
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
    _linkedId.init(getTSRule()).setValue(_dVal->getParam()._linked);
    return _linkedId.get();
  }
  if (unique_idx == 2) {
    _opCode.setValue(_dVal->getHeader()._opCode);
    return &_opCode;
  }
  //if (unique_idx == 3)
  _argument.init(getTSRule()).setValue(_dVal->getArg());
  return _argument.get();
}

}}}}

