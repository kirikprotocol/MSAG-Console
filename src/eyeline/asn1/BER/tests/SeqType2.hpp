#ifndef __EYELINE_ASN1_BER_TESTS_SEQTYPE2_HPP__
# define __EYELINE_ASN1_BER_TESTS_SEQTYPE2_HPP__

# include <inttypes.h>
# include "eyeline/util/OptionalObjT.hpp"

using eyeline::util::OptionalObj_T;

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {

struct SeqType2 {
  SeqType2()
  : a(0)
  {}

  int a;
  util::OptionalObj_T<int> b;
  util::OptionalObj_T<int> c;
};

}}}}

#endif
