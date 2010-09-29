#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/ros/proto/enc/REInvokePdu.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace enc {

const asn1::ASTag REInvokePdu::_pduTag(asn1::ASTag::tagContextSpecific, 1);

void REInvokePdu::setLinkedId(const ros::LinkedId & link_id) /*throw(std::exception)*/
{
  if (!_linkedId) {
    _linkedId = new (_memLinkId._buf) RELinkedIdType(link_id._invId, getVALRule());
    asn1::ber::EncoderOfPlainSequence_T<4>::setField(1, *_linkedId);
  } else
    _linkedId->setIdLinked(link_id._invId);
}

void REInvokePdu::setArgType(const PDUArgument & use_arg) /*throw(std::exception)*/
{
  if (!_argType) {
    _argType = new (_memArgType._buf) asn1::ber::EncoderOfASType(getTSRule());
    asn1::ber::EncoderOfPlainSequence_T<4>::setField(3, *_argType);
  }

  if (use_arg._kind & PDUArgument::asvTSyntax)
    _argType->setValue(use_arg._tsEnc);
  else {
    asn1::ber::TypeEncoderAC * pEnc = 
      static_cast<asn1::ber::TypeEncoderAC *>(use_arg._asType->getEncoder(getTSRule()));
    _argType->setValue(*pEnc);
  }
}

void REInvokePdu::setValue(const ROSInvokePdu & use_val) /*throw(std::exception)*/
{
  _invId.setValue(use_val.getHeader()._invId);
  _opCode.setValue(use_val.getHeader()._opCode);
  if (use_val.getParam()._linked._present)
    setLinkedId(use_val.getParam()._linked);
  if (use_val.hasArgument())
    setArgType(use_val.getArg());
}

}}}}

