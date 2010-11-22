#include "MESeqTypeExplicit1.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {
namespace enc {

const ASTag MESeqTypeExplicit1::_tag_b= ASTag(ASTag::tagContextSpecific, 5);
const ASTag MESeqTypeExplicit1::_tag_c= ASTag(ASTag::tagContextSpecific, 1);

void
MESeqTypeExplicit1::setValue(const SeqType1& value)
{
  _eA.setValue(value.a);
  _eB.setValue(value.b);
}

void
MESeqTypeExplicit1::construct()
{
  setField(0, _eA);
  setField(1, _eB);
  setField(2, _eC);
}

}}}}}
