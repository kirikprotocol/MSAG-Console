#include "MESeqType2.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {
namespace enc {

const ASTag MESeqType2::_tag_b= ASTag(ASTag::tagContextSpecific, 3);

void
MESeqType2::setValue(const SeqType2& value)
{
  _eA.setValue(value.a);
  //clear optionals and unknown extensions first
  clearFields(1);

  if (value.b.get()) {
    _eB.init(_tag_b, ASTagging::tagsIMPLICIT, getTSRule()).setValue(*value.b.get());
    setField(1, *_eB.get());
  }
}

void
MESeqType2::construct()
{
  setField(0, _eA);
}

}}}}}
