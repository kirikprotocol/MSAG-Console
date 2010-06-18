#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/ros/proto/dec/RDRResultPdu.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace dec {


const asn1::ASTag RDRResultPdu::_pduTag(asn1::ASTag::tagContextSpecific, 2);
const asn1::ASTag RDRResultNLPdu::_pduTag(asn1::ASTag::tagContextSpecific, 7);

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
  _valMask = false;
  return &_resType;
}

void RDReturnResult::setResultField(ros::LocalOpCode & op_code, 
                                    ros::PDUArgument & use_arg)
{
  if (!_result)
    _result = new (_memRes._buf) RDResultField(getVALRule());
  _result->setValue(op_code, use_arg._tsEnc);
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
  setResultField(_dVal->getHeader()._opCode, _dVal->getArg());
  return _result;

}
//Performs actions upon successfull optional element decoding
void RDReturnResult::markDecodedOptional(uint16_t unique_idx) /*throw() */
{
  if (unique_idx == 1)
    _dVal->getArg()._kind = ros::PDUArgument::asvTSyntax;
}

}}}}

