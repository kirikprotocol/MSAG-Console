/* ************************************************************************* *
 * BER Encoder: integer types encoding.
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_INT_DEFS
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_ENCODER_INT_DEFS

#include "eyeline/asn1/TransferSyntax.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

using eyeline::asn1::TSLength;

/* ************************************************************************* *
 * Estimates length of BER encoding of integer value according to X.690 cl. 8.3
 * NOTE: negative INTEGER value is encoded as 'two's complement' number.
 * ************************************************************************* */
template < 
  class _TArg /* unsigned integer type, sizeof(_TArg) < 32 bytes */
>
inline uint8_t estimate_INTEGER(const _TArg & use_val)
{
  if (use_val <= 0xFF)
    return 1;

  uint8_t msBit = (sizeof(_TArg)<<3) - 1;
  if (use_val & (1 << msBit)) { //'two's complement' number
    //check that 9 consequitive bits starting from MSB are not
    //all set, in order to get fewest meaningfull octets
    uint8_t lmsBit = msBit;
    while (--lmsBit && (use_val & (1 << lmsBit)))
      ;
    if ((msBit - lmsBit) >= 9)
      return (lmsBit + 9) >> 3;
  } else {                      //unsigned number
    //find MSB, in order to get fewest meaningfull octets
    while (--msBit && !(use_val & (1 << msBit)))
      ;
  }
  return (msBit + 7) >> 3;
}

// -----------------------------------------------
// Specialization for uint8_t
// -----------------------------------------------
template <> inline
uint8_t estimate_INTEGER(const uint8_t & use_val)
{
  return 1;
}



/* ************************************************************************* *
 * Encodes by BER the integer value according to X.690 cl. 8.3
 * Returns  number of bytes of resulted encoding or zero in case of
 * insufficient length of buffer provided.
 * ************************************************************************* */
template <class _TArg /* unsigned integer type */> inline
uint8_t encode_INTEGER(const _TArg & use_val, uint8_t * use_enc, TSLength max_len)
{
  uint8_t rlen = estimate_INTEGER(use_val);
  if (rlen > max_len)
    return 0;

  _TArg uValue = use_val;
  uint16_t i = rlen;
  //encode value - from MSB to LSB
  while (--i) {
    use_enc[i] = (uint8_t)uValue;
    uValue >>= 8;
  }
  return rlen;
}

// -----------------------------------------------
// Specialization for uint8_t
// -----------------------------------------------
template <> inline
uint8_t encode_INTEGER(const uint8_t & use_val, uint8_t * use_enc, TSLength max_len)
{
  if (!max_len)
    return 0;
  *use_enc = use_val;
  return 1;
}


} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_INT_DEFS */

