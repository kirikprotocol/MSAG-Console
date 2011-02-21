#include "MDSeqType4.hpp"
#include "util/Exception.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {
namespace dec {

const ASTag MDSeqType4::_tag_B= ASTag(ASTag::tagContextSpecific, 3);
const ASTag MDSeqType4::_tag_C= ASTag(ASTag::tagContextSpecific, 4);

void
MDSeqType4::construct(void)
{
  setField(0, asn1::_tagINTEGER, EDAlternative::altMANDATORY);
  setField(1, _tag_B, ASTagging::tagsIMPLICIT, EDAlternative::altOPTIONAL);
  setField(2, _tag_C, ASTagging::tagsIMPLICIT, EDAlternative::altOPTIONAL);
  setUnkExtension(3);
}

asn1::ber::TypeDecoderAC*
MDSeqType4::prepareAlternative(uint16_t unique_idx)
{
  if (!_value)
    throw smsc::util::Exception("MDSeqType2::prepareAlternative: value isn't set!");
  if (unique_idx > 3)
    throw smsc::util::Exception("MDSeqType2::prepareAlternative: undefined UId=%d", unique_idx);

  if (unique_idx == 0) {
    _dA.setValue(_value->a);
    return &_dA;
  }

  if (unique_idx == 1) {
    _dB.init(getTSRule()).setValue(_value->b.init());
    return _dB.get();
  }

  if (unique_idx == 2) {
    _dC.init(getTSRule()).setValue(_value->c.init());
    return _dC.get();
  }

  //NOTE: unknown extension is a repeatition type
  if (!_uext.get())
    _uext.init(getTSRule()).setValue(_value->_unkExt);
  else
    _uext->setValue(_value->_unkExt);

  return _uext.get();
}

}}}}}
