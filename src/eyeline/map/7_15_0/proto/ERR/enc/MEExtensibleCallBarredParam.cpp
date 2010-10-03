#include "MEExtensibleCallBarredParam.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace enc {

const asn1::ASTag MEExtensibleCallBarredParam::_tag_unauthMsgOrig= asn1::ASTag(asn1::ASTag::tagContextSpecific, 1);

void
MEExtensibleCallBarredParam::setValue(const ExtensibleCallBarredParam& value)
{
  uint16_t idx= 0;
  if (value.callBarringCause.get()) {
    _eCallBarringCause.init(getTSRule()).setValue(value.callBarringCause.get()->value);
    setField(idx++, *_eCallBarringCause.get());
  }
  if (value.extensionContainer.get()) {
    _eExtensionContainer.init(getTSRule()).setValue(*value.extensionContainer.get());
    setField(idx++, *_eExtensionContainer.get());
  }
  if ( !value._unkExt._tsList.empty() )
    idx= _eUnkExt.init().setValue(value._unkExt, *this, idx);

  if (value.unauthorisedMessageOriginator.get()) {
    _eUnauthMsgOrig.init(_tag_unauthMsgOrig, asn1::ASTagging::tagsIMPLICIT, getTSRule());
    setField(idx, *_eUnauthMsgOrig.get());
  }
}

}}}}
