#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/asn1/BER/tests/dec/MDSeqType5.hpp"
#include "util/Exception.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {
namespace dec {

const ASTag MDSeqType5::_tag_C(ASTag::tagContextSpecific, 10);
const ASTag MDSeqType5::_tag_D(ASTag::tagContextSpecific, 11);

void
MDSeqType5::construct(void)
{
  setField(0, MDChoiceType1::_tagOptions, EDAlternative::altMANDATORY);
  setField(1, MDChoiceType2::_typeTag, EDAlternative::altMANDATORY);
  setField(2, _tag_C, ASTagging::tagsIMPLICIT, EDAlternative::altMANDATORY);
  setField(3, _tag_D, ASTagging::tagsIMPLICIT, EDAlternative::altMANDATORY);
}

asn1::ber::TypeDecoderAC*
MDSeqType5::prepareAlternative(uint16_t unique_idx)
{
  if (!_value)
    throw smsc::util::Exception("MDSeqType5::prepareAlternative: value isn't set!");
  if (unique_idx > 3)
    throw smsc::util::Exception("MDSeqType5::prepareAlternative: undefined UId=%d", unique_idx);

  if (unique_idx == 0) {
    _dA.setValue(_value->a);
    return &_dA;
  }

  if (unique_idx == 1) {
    _dB.setValue(_value->b);
    return &_dB;
  }

  if (unique_idx == 2) {
    _dC.setValue(_value->c);
    return &_dC;
  }

  _dD.setValue(_value->d);
  return &_dD;
}

}}}}}
