#include "MDSystemFailureParam.hpp"

#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif

namespace eyeline {
namespace map {
namespace err {
namespace dec {

void
MDSystemFailureParam::construct()
{
  setAlternative(0, asn1::_tagENUM, asn1::ASTagging::tagsIMPLICIT);
  setAlternative(1, asn1::_tagSEQOF, asn1::ASTagging::tagsIMPLICIT);
}

asn1::ber::TypeDecoderAC*
MDSystemFailureParam::prepareAlternative(uint16_t unique_idx)
{
  if (!_value) //assertion!!!
    throw smsc::util::Exception("MDSystemFailureParam::prepareAlternative: value isn't set!");
  if (unique_idx > 1) //assertion!!!
    throw smsc::util::Exception("MDSystemFailureParam::prepareAlternative: undefined UId");

  if (unique_idx == 0)
    _altDec.networkResource().init(getTSRule()).setValue(_value->networkResource().init()); // ????? init(asn1::_tagENUM,asn1::ASTagging::tagsIMPLICIT,getTSRule())
  else
    _altDec.extensibleSystemFailureParam().init(getTSRule()).setValue(_value->extensibleSystemFailureParam().init());

  return _altDec.get();

}

}}}}
