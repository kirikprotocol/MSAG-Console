#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/ros/proto/enc/RELinkedIdType.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace enc {

const asn1::ASTagging RELinkedIdType::_tagsPresent(
  asn1::ASTag::tagContextSpecific, 0, asn1::ASTagging::tagsIMPLICIT);

const asn1::ASTagging RELinkedIdType::_tagsAbsent(
  asn1::ASTag::tagContextSpecific, 1, asn1::ASTagging::tagsIMPLICIT);


void RELinkedIdType::resetAlt(void)
{
  if (_alt._none) {
    _alt._none->~TypeEncoderAC();
    _alt._none = NULL;
  }
}

void RELinkedIdType::setIdLinked(ros::InvokeId inv_id)
{
  resetAlt();
  _alt._present = new (_memAlt._buf) asn1::ber::EncoderOfINTEGER(_tagsPresent, getTSRule());
  _alt._present->setValue(inv_id);
}

void RELinkedIdType::setIdAbsent(void)
{
  resetAlt();
  _alt._absent = new (_memAlt._buf) asn1::ber::EncoderOfNULL(_tagsAbsent, getTSRule());
}

}}}}

