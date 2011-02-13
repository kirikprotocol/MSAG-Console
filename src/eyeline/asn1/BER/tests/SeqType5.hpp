#ifndef __EYELINE_ASN1_BER_TESTS_SEQTYPE5_HPP__
# define __EYELINE_ASN1_BER_TESTS_SEQTYPE5_HPP__

# include <inttypes.h>
# include "eyeline/asn1/BER/tests/ChoiceType1.hpp"
# include "eyeline/asn1/BER/tests/ChoiceType2.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {

struct SeqType5 {
  ChoiceType1 a;
  ChoiceType2 b;
  ChoiceType1 c;
  ChoiceType2 d;
};

}}}}

#endif
