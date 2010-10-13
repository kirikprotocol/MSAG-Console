#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/ros/proto/dec/RDRResultPdu.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace dec {

/* ROS ReturnResult type is defined in IMPLICIT tagging environment as follow:
ReturnResultType ::= SEQUENCE {
    invokeId	InvokeIdType,
    result	    SEQUENCE {
      opcode	INTEGER,
      result	ABSTRACT-SYNTAX.&Type({Operations})
    } OPTIONAL
} */
//Initializes ElementDecoder for this type
void RDReturnResult::construct(void)
{
  asn1::ber::DecoderOfSequence_T<2>::setField(0, asn1::_tagINTEGER, asn1::ber::EDAlternative::altMANDATORY);
  asn1::ber::DecoderOfSequence_T<2>::setField(1, asn1::_tagSEQOF, asn1::ber::EDAlternative::altOPTIONAL);
}

asn1::ber::TypeDecoderAC *
  RDReturnResult::RDResultField::prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */
{
  if (!_valMask)
    throw smsc::util::Exception("ros::proto::dec::RDResultField : value isn't set");
  if (unique_idx > 1)
    throw smsc::util::Exception("ros::proto::dec::RDResultField::prepareAlternative() : undefined UId");
  
  if (!unique_idx)
    return &_opCode;
  //if (unique_idx == 1)
  return &_resType;
}

// ----------------------------------------
// -- DecoderOfStructAC interface methods
// ----------------------------------------
//If necessary, allocates optional element and initializes associated TypeDecoderAC
asn1::ber::TypeDecoderAC * 
  RDReturnResult::prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */
{
  if (!_dVal) //assertion!!!
    throw smsc::util::Exception("ros::proto::dec::RDReturnResult : value isn't set!");
  if (unique_idx > 1)
    throw smsc::util::Exception("ros::proto::dec::RDReturnResult::prepareAlternative() : undefined UId");

  if (!unique_idx) {
    _invId.setValue(_dVal->getHeader()._invId);
    return &_invId;
  }
  //if (unique_idx == 1)
  _result.init(getTSRule()).setValue(_dVal->getHeader()._opCode, _dVal->getArg());
  return _result.get();
}

}}}}

