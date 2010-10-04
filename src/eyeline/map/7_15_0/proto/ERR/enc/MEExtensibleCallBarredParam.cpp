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
    setField(0, *_eCallBarringCause.get());
  } else
    clearField(0);

  if (value.extensionContainer.get()) {
    _eExtensionContainer.init(getTSRule()).setValue(*value.extensionContainer.get());
    setField(1, *_eExtensionContainer.get());
  } else
    clearField(1);

  if (value.unauthorisedMessageOriginator.get()) {
    _eUnauthMsgOrig.init(_tag_unauthMsgOrig, asn1::ASTagging::tagsIMPLICIT, getTSRule());
    setField(2, *_eUnauthMsgOrig.get());
  } else
    clearField(2);

  clearField3(3);
  if ( !value._unkExt._tsList.empty() )
    _eUnkExt.init().setValue(value._unkExt, *this, 3);
}

}}}}
