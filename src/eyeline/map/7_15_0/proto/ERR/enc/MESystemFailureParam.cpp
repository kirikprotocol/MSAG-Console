#include "MESystemFailureParam.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace enc {

void
MESystemFailureParam::setValue(const SystemFailureParam& value)
{
  switch (value.getKind()) {
  case SystemFailureParam::KindNetworkResource:
    setNetworkResource(*value.getNetworkResource());
    break;
  case SystemFailureParam::KindExtensibleSystemFailureParam:
    setExtensibleSystemFailureParam(*value.getExtensibleSystemFailureParam());
    break;
  default:
    throw smsc::util::Exception("enc::MESystemFailureParam::setValue() : invalid value");
  }
}

void
MESystemFailureParam::setNetworkResource(const common::NetworkResource& val)
{
  cleanup();
  _value.networkResource= new (_memAlloc.buf) common::enc::MENetworkResource(getTSRule());
  _value.networkResource->setValue(val);
  asn1::ber::EncoderOfChoice::setSelection(*_value.networkResource);
}

void
MESystemFailureParam::setExtensibleSystemFailureParam(const ExtensibleSystemFailureParam& value)
{
  cleanup();
  _value.any = new (_memAlloc.buf) MEExtensibleSystemFailureParam(value, getTSRule());
  asn1::ber::EncoderOfChoice::setSelection(*_value.extensibleSystemFailureParam);
}

}}}}
