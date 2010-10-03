#include "MEUnexpectedDataParam.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace enc {

void
MEUnexpectedDataParam::setValue(const DataMissingParam& value)
{
  uint16_t idx=0;
  if (value.extensionContainer.get()) {
    _eExtensionContainer.init(getTSRule).setValue(*value.extensionContainer.get());
    setField(idx++, *_eExtensionContainer.get());
  }
  if (!value._unkExt._tsList.empty())
    _eUnkExt.init().setValue(value._unkExt, *this, idx);
}

}}}}
