#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/ros/proto/enc/RERejectPdu.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace enc {

/* ROS::Reject PDU is defined in IMPLICIT tagging environment as following:

Reject ::= SEQUENCE {
    invokeId	InvokeId,
    problem     ProblemType
} */
void RERejectPdu::construct(void)
{
  asn1::ber::EncoderOfPlainSequence_T<2>::setField(0, _invId);
  asn1::ber::EncoderOfPlainSequence_T<2>::setField(1, _problem);
}

void RERejectPdu::setValue(const ROSRejectPdu & use_val) /*throw(std::exception)*/
{
  _invId.setValue(use_val.getHeader()._invId);
  _problem.setValue(use_val.getParam());
}

}}}}
