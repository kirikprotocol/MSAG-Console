#ifndef __EYELINE_ASN1_BER_TESTS_SEQTYPE1_HPP__
# define __EYELINE_ASN1_BER_TESTS_SEQTYPE1_HPP__

# include <inttypes.h>

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {

struct SeqType1 {
  SeqType1()
  : a(0), b(0), c(0)
  {}
  SeqType1(int a_arg, int b_arg)
  : a(a_arg), b(b_arg), c(0)
  {}
  int a;
  int b;
  uint8_t c;
};

}}}}

#endif
