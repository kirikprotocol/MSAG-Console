#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/ros/proto/enc/RERErrorPdu.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace enc {

const asn1::ASTag RERErrorPdu::_pduTag(asn1::ASTag::tagContextSpecific, 3);

void RERErrorPdu::setArgType(const PDUArgument & use_arg) /*throw(std::exception)*/
{
  if (!_argType) {
    _argType = new (_memArg._buf) asn1::ber::EncoderOfASType(getTSRule());
    asn1::ber::EncoderOfPlainSequence_T<3>::setField(2, *_argType);
  }

  if (use_arg._kind & PDUArgument::asvTSyntax)
    _argType->setValue(use_arg._tsEnc);
  else {
    asn1::ber::TypeEncoderAC * pEnc = 
      static_cast<asn1::ber::TypeEncoderAC *>(use_arg._asType->getEncoder(getTSRule()));
    _argType->setValue(*pEnc);
  }
}

void RERErrorPdu::setValue(const ROSErrorPdu & use_val) /*throw(std::exception)*/
{
  _invId.setValue(use_val.getHeader()._invId);
  _errCode.setValue(use_val.getHeader()._opCode);
  if (use_val.hasArgument())
    setArgType(use_val.getArg());
}

}}}}

