#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/map/7_15_0/proto/EXT/enc/MEExtensionContainer.hpp"

namespace eyeline {
namespace map {
namespace ext {
namespace enc {

const asn1::ASTag MEExtensionContainer::_tag_PrivateExtensionList(asn1::ASTag::tagContextSpecific, 0);
const asn1::ASTag MEExtensionContainer::_tag_PcsExtensions(asn1::ASTag::tagContextSpecific, 1);

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
  //clear optionals and extensions
  clearFields();

  if (value.privateExtensionList.get()) {
    _ePrivateExtensionList.init(_tag_PrivateExtensionList, asn1::ASTagging::tagsIMPLICIT,
                                getTSRule()).setValue(*value.privateExtensionList.get());
    setField(0, *_ePrivateExtensionList.get());
  }
  if (value.pcs_Extensions.get()) {
    _ePcsExtensions.init(_tag_PcsExtensions, asn1::ASTagging::tagsIMPLICIT,
                         getTSRule()).setValue(*value.pcs_Extensions.get());
    setField(1, *_ePcsExtensions.get());
  }
  if (!value._unkExt._tsList.empty())
    setExtensions(value._unkExt, 2);
}

}}}}

