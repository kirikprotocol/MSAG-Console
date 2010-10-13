#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/ros/proto/enc/RERErrorPdu.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace enc {

/* ROS ReturnError PDU is defined in IMPLICIT tagging environment as follow:
ReturnError ::= SEQUENCE {
    invokeId	InvokeIdType,
    errcode	    INTEGER,
    parameter	ABSTRACT-SYNTAX.&Type({Errors}) OPTIONAL
} */

void RERErrorPdu::construct(void)
{
  asn1::ber::EncoderOfPlainSequence_T<3>::setField(0, _invId);
  asn1::ber::EncoderOfPlainSequence_T<3>::setField(1, _errCode);
}

void RERErrorPdu::setValue(const ROSErrorPdu & use_val) /*throw(std::exception)*/
{
  _invId.setValue(use_val.getHeader()._invId);
  _errCode.setValue(use_val.getHeader()._opCode);

  if (use_val.hasArgument()) {
    _argType.init(getTSRule()).setValue(use_val.getArg());
    setField(3, *_argType.get());
  } else
    clearField(3);
}

}}}}

