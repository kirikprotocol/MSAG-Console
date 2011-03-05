#include "MDIllegalSubscriberParam.hpp"

#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif

namespace eyeline {
namespace map {
namespace err {
namespace dec {

void
MDIllegalSubscriberParam::construct()
{
  setField(0, asn1::_uniTag().SEQOF, asn1::ber::EDAlternative::altOPTIONAL);
  setUnkExtension(1);
}

asn1::ber::TypeDecoderAC*
MDIllegalSubscriberParam::prepareAlternative(uint16_t unique_idx)
{
  if (!_value) //assertion!!!
    throw smsc::util::Exception("MDIllegalSubscriberParam::prepareAlternative : value isn't set!");
  if (unique_idx > 1)
    throw smsc::util::Exception("MDIllegalSubscriberParam::prepareAlternative() : undefined UId");

  if (unique_idx == 0) {
    _dExtContainer.init(getTSRule()).setValue(_value->extensionContainer.init());
    return _dExtContainer.get();
  }

  if (!_uext.get())
    _uext.init(getTSRule()).setValue(_value->_unkExt);
  else
    _uext->setValue(_value->_unkExt);

  return _uext.get();
}

}}}}
