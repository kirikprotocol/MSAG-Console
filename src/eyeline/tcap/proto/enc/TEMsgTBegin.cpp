#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/proto/enc/TEMsgTBegin.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

const asn1::ASTagging
  TETBegin::_typeTags(asn1::ASTag::tagApplication,
                    2, asn1::ASTagging::tagsIMPLICIT);

}}}}

