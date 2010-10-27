#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */
#include "MEAbsentSubscriberParam.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace enc {

const asn1::ASTag MEAbsentSubscriberParam::_tag_absentSubscriberReason(asn1::ASTag::tagContextSpecific, 0);

/*
AbsentSubscriberParam ::= SEQUENCE {
        extensionContainer      ExtensionContainer      OPTIONAL,
        ...,
        absentSubscriberReason  [0] AbsentSubscriberReason      OPTIONAL
 }
*/
void
MEAbsentSubscriberParam::setValue(const AbsentSubscriberParam& value)
{
  //clear optionals and extensions
  clearFields();

  if (value.extensionContainer.get()) {
    _eExtensionContainer.init(getTSRule()).setValue(*value.extensionContainer.get());
    setField(0, *_eExtensionContainer.get());
  }

  if (value.absentSubscriberReason.get()) {
    _eAbsentSubscirberReason.init(_tag_absentSubscriberReason, asn1::ASTagging::tagsIMPLICIT,
                                     getTSRule()).setValue(*value.absentSubscriberReason.get());
    setField(1, *_eAbsentSubscirberReason.get());
  }

  if ( !value._unkExt._tsList.empty() )
    setExtensions(value._unkExt, 2);
}

}}}}
