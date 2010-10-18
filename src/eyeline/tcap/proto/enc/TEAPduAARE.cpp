#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

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


/* AARE APdu is defined in EXPLICIT tagging environment as following:

AARE-apdu ::= [APPLICATION 1] IMPLICIT SEQUENCE {
  protocol-version          ProtocolVersion DEFAULT {version1},
  application-context-name  ApplicationContext,
  result                    [2]  Associate-result,
  result-source-diagnostic  [3]  Associate-source-diagnostic,
  user-information          UserInformation OPTIONAL
} */
void TEAPduAARE::construct(void)
{
  setField(0, _protoVer);
  setField(1, _appCtx);
  setField(2, _ascResult);
  setField(3, _ascDiagn);
}

TEUserInformation * TEAPduAARE::getUI(void)
{
  if (!_pUI.get()) {
    _pUI.init(getTSRule());
    setField(4, *_pUI.get());
  }
  return _pUI.get();
}


}}}}

