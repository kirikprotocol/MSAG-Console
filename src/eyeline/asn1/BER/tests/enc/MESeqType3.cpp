#include "MESeqType3.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {
namespace enc {

const ASTag MESeqType3::_tag_b= ASTag(ASTag::tagContextSpecific, 3);

void
MESeqType3::setValue(const SeqType3& value)
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
MESeqType3::construct()
{
  setField(0, _eA);
}

}}}}}
