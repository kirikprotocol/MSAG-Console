#include "MESeqType5.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {
namespace enc {

const ASTag MESeqType5::_tag_c= ASTag(ASTag::tagContextSpecific, 10);
const ASTag MESeqType5::_tag_d= ASTag(ASTag::tagContextSpecific, 11);

void
MESeqType5::setValue(const SeqType5& value)
{
  _eA.setValue(value.a);
  _eB.setValue(value.b);
  _eC.setValue(value.c);
  _eD.setValue(value.d);
}

void
MESeqType5::construct()
{
  setField(0, _eA);
  setField(1, _eB);
  setField(2, _eC);
  setField(3, _eD);
}

}}}}}
