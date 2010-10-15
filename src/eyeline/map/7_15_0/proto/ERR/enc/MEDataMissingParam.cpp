#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/map/7_15_0/proto/ERR/enc/MEDataMissingParam.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace enc {

void
MEDataMissingParam::setValue(const DataMissingParam& value)
{
  //clear optionals and extensions
  clearFields();

  if (value.extensionContainer.get()) {
    _extensionContainer.init(getTSRule()).setValue(*value.extensionContainer.get());
    setField(0, *_extensionContainer.get());
  }
  if (!value._unkExt._tsList.empty())
    setExtensions(value._unkExt, 1);
}

}}}}
