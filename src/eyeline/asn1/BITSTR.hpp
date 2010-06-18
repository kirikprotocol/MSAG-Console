/* ************************************************************************* *
 * ASN.1 BIT STRING type definitions.
 * ************************************************************************* */
#ifndef __ABSTRACT_SYNTAX_BITSTR_DEFS__
#ident "@(#)$Id$"
#define __ABSTRACT_SYNTAX_BITSTR_DEFS__

#include "eyeline/util/BITArray.hpp"

namespace eyeline {
namespace asn1 {

/* ---------------------------------------------------------- *
 * Template for BIT STRING with maximum number of bits = 255.
 * ---------------------------------------------------------- */
static const uint8_t _BITSTR_TINY_DFLT_SZ = 16*8; //in bits

typedef eyeline::util::BITArrayExtension_T<uint8_t> BITArrayTiny;

template <uint8_t _StackBitsTArg = _BITSTR_TINY_DFLT_SZ>
class BITSTRTiny_T : public eyeline::util::BITArray_T<uint8_t, _StackBitsTArg> {
public:
  typedef eyeline::util::BITArrayExtension_T<uint8_t> ArrayType;
  typedef eyeline::util::BITArrayExtension_T<uint8_t>::size_type size_type;

  explicit BITSTRTiny_T(uint8_t num_bits = 0)
    : eyeline::util::BITArray_T<uint8_t, _StackBitsTArg>(num_bits)
  { }
  explicit BITSTRTiny_T(const uint8_t use_bits, uint8_t num_bits)
    : eyeline::util::BITArray_T<uint8_t, _StackBitsTArg>(use_bits, num_bits)
  { }
  BITSTRTiny_T(const BITSTR_T & use_arr)
    : eyeline::util::BITArray_T<uint8_t, _StackBitsTArg>(use_arr)
  { }
  ~BITSTRTiny_T()
  { }
};
typedef BITSTRTiny_T<_BITSTR_TINY_DFLT_SZ> BITSTRTiny;


/* ---------------------------------------------------------- *
 * Template for BIT STRING with maximum number of bits = 64k.
 * ---------------------------------------------------------- */
static const uint16_t _BITSTR_DFLT_SZ = 256*8; //in bits

typedef eyeline::util::BITArrayExtension_T<uint16_t> BITArray64k;

template <uint16_t _StackBitsTArg = _BITSTR_LRG_DFLT_SZ>
class BITSTR_T : public eyeline::util::BITArray_T<uint16_t, _StackBitsTArg> {
public:
  typedef eyeline::util::BITArrayExtension_T<uint16_t> ArrayType;
  typedef eyeline::util::BITArrayExtension_T<uint16_t>::size_type size_type;

  explicit BITSTR_T(uint16_t num_bits = 0)
    : eyeline::util::BITArray_T<uint16_t, _StackBitsTArg>(num_bits)
  { }
  explicit BITSTR_T(const uint16_t use_bits, uint16_t num_bits)
    : eyeline::util::BITArray_T<uint16_t, _StackBitsTArg>(use_bits, num_bits)
  { }
  BITSTR_T(const BITSTR_T & use_arr)
    : eyeline::util::BITArray_T<uint16_t, _StackBitsTArg>(use_arr)
  { }
  ~BITSTR_T()
  { }
};
typedef BITSTR_T<_BITSTR_DFLT_SZ> BITSTR;

} //asn1
} //eyeline

#endif /* __ABSTRACT_SYNTAX_BITSTR_DEFS__ */

