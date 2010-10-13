#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/ros/proto/enc/RELinkedIdType.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace enc {

const asn1::ASTag RELinkedIdType::_tagPresent(asn1::ASTag::tagContextSpecific, 0);
const asn1::ASTag RELinkedIdType::_tagAbsent(asn1::ASTag::tagContextSpecific, 1);

/* LinkedIdType is defined in IMPLICIT tagging environment as follow:
  LinkedIdType ::=  CHOICE {
      present  [0] IMPLICIT InvokeIdType,
      absent   [1] IMPLICIT NULL
  }
*/

void RELinkedIdType::setIdLinked(ros::InvokeId inv_id)
{
  _alt.present().init(_tagPresent, asn1::ASTagging::tagsIMPLICIT, getTSRule()).setValue(inv_id);
  setSelection(*_alt.get());
}

void RELinkedIdType::setIdAbsent(void)
{
  _alt.absent().init(_tagAbsent, asn1::ASTagging::tagsIMPLICIT, getTSRule());
  setSelection(*_alt.get());
}

}}}}

