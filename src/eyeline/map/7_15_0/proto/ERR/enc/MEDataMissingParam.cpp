#include "MEDataMissingParam.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace enc {

void
MEDataMissingParam::setValue(const DataMissingParam& value) {
  uint16_t idx=0;
  if (value.extensionContainer.get()) {
    _extensionContainer.init(getTSRule()).setValue(*value.extensionContainer.get());
    setField(0, *_eMsIsdn.get());
  } else
    clearField(0);

  clearFields(1);
  if (!value.unkExt._tsList.empty())
    _unkExt.init().setValue(value.unkExt, *this, 1);
}

}}}}
