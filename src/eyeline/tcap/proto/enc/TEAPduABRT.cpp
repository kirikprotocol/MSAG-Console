#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/proto/enc/TEAPduABRT.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

const asn1::ASTagging
  TEAPduABRT::_typeTags(asn1::ASTag::tagApplication,
                       4, asn1::ASTagging::tagsIMPLICIT);

const asn1::ASTagging 
  TEAPduABRT::AbortSource::_typeTags(asn1::ASTag::tagContextSpecific,
                                    0, asn1::ASTagging::tagsIMPLICIT);

}}}}

