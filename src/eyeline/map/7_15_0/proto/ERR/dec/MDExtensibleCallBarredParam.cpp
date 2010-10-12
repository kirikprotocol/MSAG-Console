#include "MDExtensibleCallBarredParam.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace dec {

const asn1::ASTag
MDExtensibleCallBarredParam::_tag_UnauthMsgOrig= asn1::ASTag(asn1::ASTag::tagContextSpecific, 1);

void
MDExtensibleCallBarredParam::construct()
{
  setField(0, asn1::_tagENUM, asn1::ASTagging::tagsIMPLICIT, asn1::ber::EDAlternative::altOPTIONAL);
  setField(1, asn1::_tagSEQOF, asn1::ASTagging::tagsIMPLICIT, asn1::ber::EDAlternative::altOPTIONAL);
  setField(2, _tag_UnauthMsgOrig, asn1::ASTagging::tagsIMPLICIT, asn1::ber::EDAlternative::altOPTIONAL);
  setUnkExtension(3);
}

/*
 ExtensibleCallBarredParam ::= SEQUENCE {
        callBarringCause        CallBarringCause        OPTIONAL,
        extensionContainer      ExtensionContainer      OPTIONAL,
        ... ,
        unauthorisedMessageOriginator   [1] NULL                OPTIONAL }
*/
asn1::ber::TypeDecoderAC*
MDExtensibleCallBarredParam::prepareAlternative(uint16_t unique_idx)
{
  if (!_value)
    throw smsc::util::Exception("MDExtensibleCallBarredParam::prepareAlternative: value isn't set!");
  if (unique_idx > 2)
    throw smsc::util::Exception("MDExtensibleCallBarredParam::prepareAlternative: undefined UId");

  if (unique_idx == 0) {
    _dCallBarringCause.init(getTSRule()).setValue(_value->callBarringCause.init());
    return _dCallBarringCause.get();
  }
  if (unique_idx == 1) {
    _dExtCont.init(getTSRule()).setValue(_value->extensionContainer.init());
    return _dExtCont.get();
  }
  if (unique_idx == 2) {
    _dUnauthMsgOrig.init(getTSRule()); // ????? .setValue(_value->unauthorisedMessageOriginator.init());
    return _dUnauthMsgOrig.get();
  }

  _uext.init(getTSRule()).setValue(_value->_unkExt);
  return _uext.get();
}

}}}}
