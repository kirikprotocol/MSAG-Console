#include "MDChoiceType1.hpp"

#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {
namespace dec {

void
MDChoiceType1::construct()
{
  setAlternative(0, _tagINTEGER, asn1::ASTagging::tagsIMPLICIT);
  setAlternative(1, _tagBOOL, asn1::ASTagging::tagsIMPLICIT);
}

asn1::ber::TypeDecoderAC *
MDChoiceType1::prepareAlternative(uint16_t unique_idx) /*throw(std::exception)*/
{
  if (!_value) //assertion!!!
    throw smsc::util::Exception("MDChoiceType1::prepareAlternative: value isn't set!");
  if (unique_idx > 1) //assertion!!!
    throw smsc::util::Exception("MDChoiceType1::prepareAlternative: undefined UId");

  if (unique_idx == 0)
    _altDec.case1().init(getTSRule()).setValue(_value->case1().init());
  else
    _altDec.case2().init(getTSRule()).setValue(_value->case2().init());

  return _altDec.get();
}

}}}}}
