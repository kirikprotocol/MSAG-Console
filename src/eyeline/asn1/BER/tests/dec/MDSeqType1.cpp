#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/asn1/BER/tests/dec/MDSeqType1.hpp"
#include "util/Exception.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {
namespace dec {

const ASTag MDSeqType1::_tag_B= ASTag(ASTag::tagContextSpecific, 5);
const ASTag MDSeqType1::_tag_C= ASTag(ASTag::tagContextSpecific, 1);

void
MDSeqType1::construct(void)
{
  setField(0, _tagINTEGER, EDAlternative::altMANDATORY);
  setField(1, _tag_B, ASTagging::tagsIMPLICIT, EDAlternative::altMANDATORY);
  setField(2, _tag_C, ASTagging::tagsIMPLICIT, EDAlternative::altMANDATORY);
}

asn1::ber::TypeDecoderAC*
MDSeqType1::prepareAlternative(uint16_t unique_idx)
{
  if (!_value)
    throw smsc::util::Exception("MDSeqType1::prepareAlternative: value isn't set!");
  if (unique_idx > 2)
    throw smsc::util::Exception("MDSeqType1::prepareAlternative: undefined UId=%d", unique_idx);

  if (unique_idx == 0) {
    _dA.setValue(_value->a);
    return &_dA;
  }

  if (unique_idx == 1) {
    _dB.setValue(_value->b);
    return &_dB;
  }

  return &_dC;
}

}}}}}
