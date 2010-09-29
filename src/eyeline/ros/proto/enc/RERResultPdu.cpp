#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/ros/proto/enc/RERResultPdu.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace enc {

const asn1::ASTag RERResultPdu::_pduTag(asn1::ASTag::tagContextSpecific, 2);
const asn1::ASTag RERResultNLPdu::_pduTag(asn1::ASTag::tagContextSpecific, 7);


void REReturnResult::setResultField(ros::LocalOpCode op_code)
{
  if (!_result) {
    _result = new (_memRes._buf) REResultField(op_code, getVALRule());
    asn1::ber::EncoderOfPlainSequence_T<2>::setField(1, *_result);
  } else
    _result->setOpCode(op_code);
}

void REReturnResult::setArgType(ros::LocalOpCode op_code, 
                                const PDUArgument & use_arg) /*throw(std::exception)*/
{
  setResultField(op_code);

  if (use_arg._kind & PDUArgument::asvTSyntax)
    _result->setValue(use_arg._tsEnc);
  else {
    asn1::ber::TypeEncoderAC * pEnc = 
      static_cast<asn1::ber::TypeEncoderAC *>(use_arg._asType->getEncoder(getTSRule()));
    _result->setValue(*pEnc);
  }
}

void REReturnResult::setValue(const ros::ROSPduWithArgument & use_val) /*throw(std::exception)*/
{
  _invId.setValue(use_val.getHeader()._invId);
  if (use_val.hasArgument())
    setArgType(use_val.getHeader()._opCode, use_val.getArg());
}


}}}}

