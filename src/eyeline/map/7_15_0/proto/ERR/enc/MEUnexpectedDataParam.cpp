#include "MEUnexpectedDataParam.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace enc {

void
MEUnexpectedDataParam::setValue(const DataMissingParam& value)
{
  if (value.extensionContainer.get()) {
    _eExtensionContainer.init(getTSRule).setValue(*value.extensionContainer.get());
    setField(0, *_eExtensionContainer.get());
  } else
    clearField(0);

  clearFields(1);
  if (!value._unkExt._tsList.empty())
    _eUnkExt.init().setValue(value._unkExt, *this, 1);
}

}}}}
