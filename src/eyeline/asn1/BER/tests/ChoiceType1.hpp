#ifndef __EYELINE_ASN1_BER_TESTS_CHOICETYPE1_HPP__
# define __EYELINE_ASN1_BER_TESTS_CHOICETYPE1_HPP__

# include <inttypes.h>
# include "eyeline/util/ChoiceOfT.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {

class ChoiceType1 :
  public util::ChoiceOf2_T<int32_t, bool> {

public:
  ChoiceType1()
    : util::ChoiceOf2_T<int32_t, bool>()
  {}

  Alternative_T<int32_t>  case1() { return alternative0(); }
  Alternative_T<bool>  case2() { return alternative1(); }

  ConstAlternative_T<int32_t>   case1() const { return alternative0(); }
  ConstAlternative_T<bool>   case2() const { return alternative1(); }
};

}}}}

#endif
