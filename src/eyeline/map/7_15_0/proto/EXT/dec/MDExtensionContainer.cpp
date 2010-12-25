#include "MDExtensionContainer.hpp"

#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif

namespace eyeline {
namespace map {
namespace ext {
namespace dec {

const asn1::ASTag MDExtensionContainer::_tag_PrvtExtLst= asn1::ASTag(asn1::ASTag::tagContextSpecific, 0);
const asn1::ASTag MDExtensionContainer::_tag_PCS_Ext= asn1::ASTag(asn1::ASTag::tagContextSpecific, 1);

void
MDExtensionContainer::construct()
{
  setField(0, _tag_PrvtExtLst, asn1::ASTagging::tagsIMPLICIT, asn1::ber::EDAlternative::altOPTIONAL);
  setField(1, _tag_PCS_Ext, asn1::ASTagging::tagsIMPLICIT, asn1::ber::EDAlternative::altOPTIONAL);
  setUnkExtension(2);
}

/*
 ExtensionContainer ::= SEQUENCE {
    privateExtensionList    [0] PrivateExtensionList OPTIONAL,
    pcs-Extensions          [1] PCS-Extensions       OPTIONAL,
    ...
 }
*/
asn1::ber::TypeDecoderAC*
MDExtensionContainer::prepareAlternative(uint16_t unique_idx)
{
  if (!_value)
    throw smsc::util::Exception("MDExtensionContainer::prepareAlternative: value isn't set!");
  if (unique_idx > 2)
    throw smsc::util::Exception("MDExtensionContainer::prepareAlternative: undefined UId");

  if (unique_idx == 0) {
    _dPrvtExtLst.init(getTSRule()).setValue(_value->privateExtensionList.init());
    return _dPrvtExtLst.get();
  }
  if (unique_idx == 1) {
    _dPcsExts.init(getTSRule()).setValue(_value->pcs_Extensions.init());
    return _dPcsExts.get();
  }

  if (!_uext.get())
    _uext.init(getTSRule()).setValue(_value->_unkExt);
  else
    _uext->setValue(_value->_unkExt);

  return _uext.get();
}

}}}}
