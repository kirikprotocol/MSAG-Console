/* ************************************************************************* *
 * ASN.1 BIT STRING type definitions.
 * ************************************************************************* */
#ifndef __ABSTRACT_SYNTAX_BITSTR_DEFS__
#ident "@(#)$Id$"
#define __ABSTRACT_SYNTAX_BITSTR_DEFS__

#include "eyeline/util/BITArray.hpp"

namespace eyeline {
namespace asn1 {

using eyeline::util::BITArray_T;

static const uint16_t _BITSTR_DFLT_SZ = 256; //in bits

//Template for BIT STRING with known maximum number of bits.
//Whole BITSTR is allocated on stack at once
template <uint16_t _MaxBitsTArg>
class BITSTR_T : public BITArray_T<uint16_t, _MaxBitsTArg> {
public:
  BITSTR_T(uint16_t num_bits = 0)
    : BITArray_T<uint16_t, _MaxBitsTArg>(num_bits)
  { }
  BITSTR_T(const BITSTR_T & use_arr)
    : BITArray_T<uint16_t, _MaxBitsTArg>(use_arr)
  { }
};

//Generic BIT STRING, if number of bits excesses _BITSTR_DFLT_SZ
//bit array is allocated on heap
typedef BITSTR_T<_BITSTR_DFLT_SZ> BITSTR;


} //asn1
} //eyeline

#endif /* __ABSTRACT_SYNTAX_BITSTR_DEFS__ */

