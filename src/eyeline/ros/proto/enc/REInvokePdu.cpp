#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/ros/proto/enc/REInvokePdu.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace enc {

/* ROS::Invoke PDU is defined in IMPLICIT tagging environment as follow:

Invoke ::= [1] SEQUENCE {
  invokeId    InvokeIdType,
  linkedId    LinkedIdType OPTIONAL,
  opcode      INTEGER,
  argument    ABSTRACT-SYNTAX.&Type({Operations}) OPTIONAL
} */

void REInvokePdu::construct(void)
{
  asn1::ber::EncoderOfPlainSequence_T<4>::setField(0, _invId);
  asn1::ber::EncoderOfPlainSequence_T<4>::setField(2, _opCode);
}


void REInvokePdu::setValue(const ROSInvokePdu & use_val) /*throw(std::exception)*/
{
  _invId.setValue(use_val.getHeader()._invId);
  _opCode.setValue(use_val.getHeader()._opCode);

  if (use_val.getParam()._linked._present) {
    _linkedId.init(getTSRule()).setValue(use_val.getParam()._linked);
    setField(1, *_linkedId.get());
  } else
    clearField(1);

  if (use_val.hasArgument()) {
    _argument.init(getTSRule()).setValue(use_val.getArg());
    setField(3, *_argument.get());
  } else
    clearField(3);
}

}}}}

