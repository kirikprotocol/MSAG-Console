#include "MDCallBarredParam.hpp"

#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif

namespace eyeline {
namespace map {
namespace err {
namespace dec {

void
MDCallBarredParam::construct()
{
  setAlternative(0, asn1::_uniTag().ENUM, asn1::ASTagging::tagsIMPLICIT);
  setAlternative(1, asn1::_uniTag().SEQOF, asn1::ASTagging::tagsIMPLICIT);
}

asn1::ber::TypeDecoderAC *
MDCallBarredParam::prepareAlternative(uint16_t unique_idx) /*throw(std::exception)*/
{
  if (!_value) //assertion!!!
    throw smsc::util::Exception("MDCallBarredParam::prepareAlternative: value isn't set!");
  if (unique_idx > 1) //assertion!!!
    throw smsc::util::Exception("MDCallBarredParam::prepareAlternative: undefined UId");

  if (unique_idx == 0)
    _altDec.callBarringCause().init(getTSRule()).setValue(_value->callBarringCause().init()); // ????? init(asn1::_tagENUM,asn1::ASTagging::tagsIMPLICIT,getTSRule())
  else
    _altDec.extensibleCallBarredParam().init(getTSRule()).setValue(_value->extensibleCallBarredParam().init());

  return _altDec.get();
}

}}}}
