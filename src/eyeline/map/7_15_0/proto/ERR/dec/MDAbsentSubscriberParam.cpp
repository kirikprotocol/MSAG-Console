#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "MDAbsentSubscriberParam.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace dec {

const asn1::ASTag
MDAbsentSubscriberParam::_tag_absentSubscriberReason= asn1::ASTag(asn1::ASTag::tagContextSpecific, 0);

void
MDAbsentSubscriberParam::construct()
{
  setField(0, asn1::_tagSEQOF, asn1::ASTagging::tagsIMPLICIT, asn1::ber::EDAlternative::altOPTIONAL);
  setField(1, _tag_absentSubscriberReason, asn1::ASTagging::tagsIMPLICIT, asn1::ber::EDAlternative::altOPTIONAL);
  setUnkExtension(2);
}

/*
AbsentSubscriberParam ::= SEQUENCE {
        extensionContainer      ExtensionContainer      OPTIONAL,
        ...,
        absentSubscriberReason  [0] AbsentSubscriberReason      OPTIONAL
}
*/
asn1::ber::TypeDecoderAC*
MDAbsentSubscriberParam::prepareAlternative(uint16_t unique_idx)
{
  if (!_value)
    throw smsc::util::Exception("MDAbsentSubscriberParam::prepareAlternative: value isn't set!");
  if (unique_idx > 2)
    throw smsc::util::Exception("MDAbsentSubscriberParam::prepareAlternative: undefined UId");

  if (unique_idx == 0) {
    _dExtContainer.init(getTSRule()).setValue(_value->extensionContainer.init());
    return _dExtContainer.get();
  }
  if (unique_idx == 1) {
    _dAbsentSubscriberReason.init(getTSRule()).setValue(_value->absentSubscriberReason.init());
    return _dAbsentSubscriberReason.get();
  }
  _uext.init(getTSRule()).setValue(_value->_unkExt);
  return _uext.get();

}

}}}}
