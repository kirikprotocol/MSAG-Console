#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/map/7_15_0/proto/ERR/enc/MEExtensibleCallBarredParam.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace enc {

const asn1::ASTag MEExtensibleCallBarredParam::_tag_unauthMsgOrig= asn1::ASTag(asn1::ASTag::tagContextSpecific, 1);

void
MEExtensibleCallBarredParam::setValue(const ExtensibleCallBarredParam& value)
{
  //clear optionals first
  clearFields();

  if (value.callBarringCause.get()) {
    _eCallBarringCause.init(getTSRule()).setValue(*value.callBarringCause.get());
    setField(0, *_eCallBarringCause.get());
  }

  if (value.extensionContainer.get()) {
    _eExtensionContainer.init(getTSRule()).setValue(*value.extensionContainer.get());
    setField(1, *_eExtensionContainer.get());
  }

  if (value.unauthorisedMessageOriginator.get()) {
    _eUnauthMsgOrig.init(_tag_unauthMsgOrig, asn1::ASTagging::tagsIMPLICIT, getTSRule());
    setField(2, *_eUnauthMsgOrig.get());
  }
  
  if ( !value._unkExt._tsList.empty() )
    _eUnkExt.init().setValue(value._unkExt, *this, 3);
}

}}}}
