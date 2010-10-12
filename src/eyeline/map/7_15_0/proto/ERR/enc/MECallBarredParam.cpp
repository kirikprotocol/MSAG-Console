#include "MECallBarredParam.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace enc {

void
MECallBarredParam::setValue(const CallBarredParam& value) {
  switch (value.getKind()) {
  case CallBarredParam::KindCallBarringCause:
    setCallBarringCause(*value.getCallBarringCause());
    break;
  case CallBarredParam::KindExtensibleCallBarredParam:
    setExtensibleCallBarredParam(*value.getExtensibleCallBarredParam());
    break;
  default:
    throw smsc::util::Exception("map::7_15_0::proto::ERR::enc::MECallBarredParam::setValue() : invalid value");
  }
}

void
MECallBarredParam::setCallBarringCause(const CallBarringCause& value) {
  cleanup();
  _value.callBarringCause= new (_memAlloc.buf) MECallBarringCause(getTSRule());
  _value.callBarringCause->setValue(value);
  asn1::ber::EncoderOfChoice::setSelection(*_value.callBarringCause);
}

void
MECallBarredParam::setExtensibleCallBarredParam(const ExtensibleCallBarredParam& value) {
  cleanup();
  _value.any = new (_memAlloc.buf) MEExtensibleCallBarredParam(value, getTSRule());
  asn1::ber::EncoderOfChoice::setSelection(*_value.callBarringCause);
}

}}}}
