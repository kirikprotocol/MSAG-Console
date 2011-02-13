#include "MEChoiceType2.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {
namespace enc {

const ASTag MEChoiceType2::_typeTag= ASTag(ASTag::tagContextSpecific, 15);

void
MEChoiceType2::setValue(const ChoiceType2& value)
{
  switch (value.getChoiceIdx()) {
  case 0:
    setCase1(*value.case1().get());
    break;
  case 1:
    setCase2(*value.case2().get());
    break;
  default:
    throw smsc::util::Exception("MEChoiceType2::setValue() : invalid value");
  }
}

void
MEChoiceType2::setCase1(int32_t value)
{
  _altEnc.case1().init(getTSRule()).setValue(value);
  setSelection(*_altEnc.case1().get());
}

void
MEChoiceType2::setCase2(bool value)
{
  _altEnc.case2().init(getTSRule()).setValue(value);
  setSelection(*_altEnc.case2().get());
}

}}}}}
