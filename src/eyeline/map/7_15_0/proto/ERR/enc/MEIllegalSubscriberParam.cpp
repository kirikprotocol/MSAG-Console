#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/map/7_15_0/proto/ERR/enc/MEIllegalSubscriberParam.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace enc {

/*
IllegalSubscriberParam ::= SEQUENCE {
        extensionContainer      ExtensionContainer      OPTIONAL,
        ...}
*/
void
MEIllegalSubscriberParam::setValue(const IllegalSubscriberParam& value)
{
  //clear optionals and extensions
  clearFields();

  if (value.extensionContainer.get()) {
    _eExtensionContainer.init(getTSRule()).setValue(*value.extensionContainer.get());
    setField(0, *_eExtensionContainer.get());
  }
  if (!value._unkExt._tsList.empty())
    setExtensions(value._unkExt, 1);
}

}}}}
