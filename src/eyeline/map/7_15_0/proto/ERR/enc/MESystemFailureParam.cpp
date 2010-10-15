#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "util/Exception.hpp"
#include "eyeline/map/7_15_0/proto/ERR/enc/MESystemFailureParam.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace enc {

void
MESystemFailureParam::setValue(const SystemFailureParam& value)
{
  switch (value.getChoiceIdx()) {
  case 0: setNetworkResource(*value.networkResource().get());
    break;
  case 1: setExtensibleSystemFailureParam(*value.extensibleSystemFailureParam().get());
    break;
  default:
    throw smsc::util::Exception("enc::MESystemFailureParam::setValue() : invalid value");
  }
}

void
MESystemFailureParam::setNetworkResource(const common::NetworkResource& use_val)
{
  _altEnc.networkResource().init(getTSRule()).setValue(use_val);
  setSelection(*_altEnc.networkResource().get());
}

void
MESystemFailureParam::setExtensibleSystemFailureParam(const ExtensibleSystemFailureParam& use_val)
{
  _altEnc.extensibleSystemFailureParam().init(getTSRule()).setValue(use_val);
  setSelection(*_altEnc.extensibleSystemFailureParam().get());
}

}}}}
