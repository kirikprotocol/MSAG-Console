#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/proto/enc/TEApplicationContext.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

const asn1::ASTagging
  TEApplicationContext::_typeTags(asn1::ASTag::tagContextSpecific,
                             1, asn1::ASTagging::tagsEXPLICIT);

}}}}

