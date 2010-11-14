#ifndef __EYELINE_ASN1_BER_TESTS_SEQTYPE3_HPP__
# define __EYELINE_ASN1_BER_TESTS_SEQTYPE3_HPP__

# include <inttypes.h>
# include "eyeline/util/OptionalObjT.hpp"
# include "eyeline/asn1/UnknownExtensions.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {

struct SeqType3 {
  SeqType3()
  : a(0)
  {}

  int a;
  util::OptionalObj_T<int> b;
  asn1::UnknownExtensions  _unkExt;
};

}}}}

#endif
