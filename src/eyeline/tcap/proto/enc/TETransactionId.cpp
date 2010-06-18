#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/proto/enc/TETransactionId.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

const asn1::ASTagging
  TEOrigTransactionId::_typeTags(asn1::ASTag::tagApplication,
                               8, asn1::ASTagging::tagsIMPLICIT);

const asn1::ASTagging
  TEDestTransactionId::_typeTags(asn1::ASTag::tagApplication,
                               9, asn1::ASTagging::tagsIMPLICIT);

}}}}
