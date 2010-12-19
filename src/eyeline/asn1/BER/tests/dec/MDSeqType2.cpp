#include "MDSeqType2.hpp"
#include "util/Exception.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {
namespace dec {

const ASTag MDSeqType2::_tag_B= ASTag(ASTag::tagContextSpecific, 3);
const ASTag MDSeqType2::_tag_C= ASTag(ASTag::tagContextSpecific, 7);

void
MDSeqType2::construct(void)
{
  setField(0, asn1::_tagINTEGER, EDAlternative::altMANDATORY);
  setField(1, _tag_B, ASTagging::tagsIMPLICIT, EDAlternative::altOPTIONAL);
  setField(2, _tag_C, ASTagging::tagsIMPLICIT, EDAlternative::altOPTIONAL);
}

asn1::ber::TypeDecoderAC*
MDSeqType2::prepareAlternative(uint16_t unique_idx)
{
  if (!_value)
    throw smsc::util::Exception("MDSeqType2::prepareAlternative: value isn't set!");
  if (unique_idx > 2)
    throw smsc::util::Exception("MDSeqType2::prepareAlternative: undefined UId=%d", unique_idx);

  if (unique_idx == 0) {
    _dA.setValue(_value->a);
    return &_dA;
  }

  if (unique_idx == 1) {
    _dB.init(getTSRule()).setValue(_value->b.init());
    return _dB.get();
  }

  _dC.init(getTSRule()).setValue(_value->c.init());
  return _dC.get();
}

}}}}}
