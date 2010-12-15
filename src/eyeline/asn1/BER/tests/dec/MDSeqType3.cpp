#include "MDSeqType3.hpp"
#include "util/Exception.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {
namespace dec {

const ASTag MDSeqType3::_tag_B= ASTag(ASTag::tagContextSpecific, 3);

void
MDSeqType3::construct(void)
{
  setField(0, asn1::_tagINTEGER, EDAlternative::altMANDATORY);
  setField(1, _tag_B, ASTagging::tagsIMPLICIT, EDAlternative::altOPTIONAL);
  setUnkExtension(2);
}

asn1::ber::TypeDecoderAC*
MDSeqType3::prepareAlternative(uint16_t unique_idx)
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

  //NOTE: unknown extension is a repeatition type
  if (!_uext.get())
    _uext.init(getTSRule()).setValue(_value->_unkExt);
  else
    _uext->setValue(_value->_unkExt);

  return _uext.get();
}

}}}}}
