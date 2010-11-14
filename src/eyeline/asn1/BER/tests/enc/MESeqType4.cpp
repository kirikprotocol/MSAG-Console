#include "MESeqType4.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {
namespace enc {

const ASTag MESeqType4::_tag_b= ASTag(ASTag::tagContextSpecific, 3);
const ASTag MESeqType4::_tag_c= ASTag(ASTag::tagContextSpecific, 4);

void
MESeqType4::setValue(const SeqType4& value)
{
  _eA.setValue(value.a);
  //clear optionals and unknown extensions first
  clearFields(1);

  if (value.b.get()) {
    _eB.init(_tag_b, ASTagging::tagsIMPLICIT, getTSRule()).setValue(*value.b.get());
    setField(1, *_eB.get());
  }

  if (value.c.get()) {
    _eC.init(_tag_c, ASTagging::tagsIMPLICIT, getTSRule()).setValue(*value.c.get());
    setField(2, *_eC.get());
  }
}

void
MESeqType4::construct()
{
  setField(0, _eA);
}

}}}}}
