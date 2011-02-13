#include "MEChoiceType1.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {
namespace enc {

void
MEChoiceType1::setValue(const ChoiceType1& value)
{
  switch (value.getChoiceIdx()) {
  case 0:
    setCase1(*value.case1().get());
    break;
  case 1:
    setCase2(*value.case2().get());
    break;
  default:
    throw smsc::util::Exception("MEChoiceType1::setValue() : invalid value");
  }
}

void
MEChoiceType1::setCase1(int32_t value)
{
  _altEnc.case1().init(getTSRule()).setValue(value);
  setSelection(*_altEnc.case1().get());
}

void
MEChoiceType1::setCase2(bool value)
{
  _altEnc.case2().init(getTSRule()).setValue(value);
  setSelection(*_altEnc.case2().get());
}

}}}}}
