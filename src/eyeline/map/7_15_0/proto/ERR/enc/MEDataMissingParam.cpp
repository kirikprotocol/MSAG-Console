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
    setField(idx++, *_eMsIsdn.get());
  }
//    else
//      asn1::ber::EncoderOfPlainSequence_T<5>::clearField(3);  <===== Na huja vot eto?!

  if (!value.unkExt._tsList.empty()) {
    _unkExt.init().setValue(use_val._unkExt, *this, idx);
  } //else
    //asn1::ber::EncoderOfPlainSequence_T<5>::clearField(4); ???
}

}}}}
