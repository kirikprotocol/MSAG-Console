#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "util/Exception.hpp"
#include "eyeline/map/7_15_0/proto/ERR/enc/MECallBarredParam.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace enc {

void
MECallBarredParam::setValue(const CallBarredParam& value)
{
  switch (value.getChoiceIdx()) {
  case 0: setCallBarringCause(*value.callBarringCause().get());
    break;
  case 1: setExtensibleCallBarredParam(*value.extensibleCallBarredParam().get());
    break;
  default:
    throw smsc::util::Exception("map::7_15_0::proto::ERR::enc::MECallBarredParam::setValue() : invalid value");
  }
}

void
MECallBarredParam::setCallBarringCause(const CallBarringCause& use_val)
{
  _altEnc.callBarringCause().init(getTSRule()).setValue(use_val);
  setSelection(*_altEnc.callBarringCause().get());
}

void
MECallBarredParam::setExtensibleCallBarredParam(const ExtensibleCallBarredParam& use_val)
{
  _altEnc.extensibleCallBarredParam().init(getTSRule()).setValue(use_val);
  setSelection(*_altEnc.extensibleCallBarredParam().get());
}

}}}}
