#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/ros/proto/enc/RERejectPdu.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace enc {

const asn1::ASTag RERejectPdu::_pduTag(asn1::ASTag::tagContextSpecific, 4);

void RERejectPdu::setValue(const ROSRejectPdu & use_val) /*throw(std::exception)*/
{
  _invId.setValue(use_val.getHeader()._invId);
  _problem.setValue(use_val.getParam());
}

}}}}
