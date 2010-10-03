#include "MEExtensionContainer.hpp"

namespace eyeline {
namespace map {
namespace ext {
namespace enc {

const asn1::ASTag MEExtensionContainer::_tag_PrivateExtensionList= asn1::ASTag(asn1::ASTag::tagContextSpecific, 0);
const asn1::ASTag MEExtensionContainer::_tag_PcsExtensions= asn1::ASTag(asn1::ASTag::tagContextSpecific, 1);

/*
 ExtensionContainer ::= SEQUENCE {
    privateExtensionList    [0] PrivateExtensionList OPTIONAL,
    pcs-Extensions          [1] PCS-Extensions       OPTIONAL,
    ...
 }
*/
void
MEExtensionContainer::setValue(const ExtensionContainer& value)
{
  uint16_t idx=0;
  if (value.privateExtensionList.get()) {
    _ePrivateExtensionList.init(_tag_PrivateExtensionList, asn1::ASTagging::tagsIMPLICIT,
                                getTSRule()).setValue(*value.privateExtensionList.get());
    setField(idx++, *_ePrivateExtensionList.get());
  }

  if (value.pcs_Extensions.get()) {
    _ePcsExtensions.init(_tag_PcsExtensions, asn1::ASTagging::tagsIMPLICIT,
                         getTSRule()).setValue(*value.pcs_Extensions.get());
    setField(idx++, *_ePcsExtensions.get());
  }

  if (!value._unkExt._tsList.empty()) {
    _eUnkExt.init().setValue(value._unkExt, *this, idx);
  }
}

}}}}

