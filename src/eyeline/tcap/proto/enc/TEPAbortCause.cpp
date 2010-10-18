#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/tcap/proto/enc/TEPAbortCause.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

const asn1::ASTagging
  TEPAbortCause::_typeTags(asn1::ASTag::tagApplication,
                         10, asn1::ASTagging::tagsIMPLICIT);

}}}}

