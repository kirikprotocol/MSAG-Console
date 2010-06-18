#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/proto/enc/TEAPduAARE.hpp"


namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

const asn1::ASTag
  TEAPduAARE::_typeTag(asn1::ASTag::tagApplication, 1);

const asn1::ASTag
  TEAPduAARE::TEResultField::_typeTag(asn1::ASTag::tagContextSpecific, 2);

const asn1::ASTag
  TEAPduAARE::TEResultDiagnosticField::_typeTag(asn1::ASTag::tagContextSpecific, 3);
const asn1::ASTag
  TEAPduAARE::TEResultDiagnosticField::_tagUser(asn1::ASTag::tagContextSpecific, 1);
const asn1::ASTag
  TEAPduAARE::TEResultDiagnosticField::_tagPrvd(asn1::ASTag::tagContextSpecific, 2);

}}}}

