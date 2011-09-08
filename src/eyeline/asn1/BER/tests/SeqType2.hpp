#ifndef __EYELINE_ASN1_BER_TESTS_SEQTYPE2_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
# define __EYELINE_ASN1_BER_TESTS_SEQTYPE2_HPP__

//# include <inttypes.h>
# include "util/OptionalObjT.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {

struct SeqType2 {
  SeqType2() : a(0)
  { }

  int                             a;
  smsc::util::OptionalObj_T<int>  b;
  smsc::util::OptionalObj_T<int>  c;
};

}}}}

#endif /* __EYELINE_ASN1_BER_TESTS_SEQTYPE2_HPP__ */

