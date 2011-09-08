#ifndef __EYELINE_ASN1_BER_TESTS_SEQTYPE4_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
# define __EYELINE_ASN1_BER_TESTS_SEQTYPE4_HPP__

//# include <inttypes.h>
# include "util/OptionalObjT.hpp"
# include "eyeline/asn1/UnknownExtensions.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {

struct SeqType4 {
  SeqType4() : a(0)
  { }

  int                             a;
  smsc::util::OptionalObj_T<int>  b;
  smsc::util::OptionalObj_T<int>  c;
  asn1::UnknownExtensions   _unkExt;
};

}}}}

#endif /* __EYELINE_ASN1_BER_TESTS_SEQTYPE4_HPP__ */

