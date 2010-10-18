#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/tcap/proto/enc/TEAPduAARQ.hpp"


namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

const asn1::ASTagging
  TEAPduAARQ::_typeTags(asn1::ASTag::tagApplication,
                      0, asn1::ASTagging::tagsIMPLICIT);

/* AARQ APdu is defined in EXPLICIT tagging environment as following:

AARQ-apdu ::= [APPLICATION 0] IMPLICIT SEQUENCE {
  protocol-version          ProtocolVersion DEFAULT {version1},
  application-context-name  ApplicationContext,
  user-information          UserInformation OPTIONAL
} */

void TEAPduAARQ::construct(void)
{
  setField(0, _protoVer);
  setField(1, _appCtx);
}

TEUserInformation * TEAPduAARQ::getUI(void)
{
  if (!_pUI.get()) {
    _pUI.init(getTSRule());
    setField(2, *_pUI.get());
  }
  return _pUI.get();
}

}}}}

