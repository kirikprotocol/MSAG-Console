/* ************************************************************************* *
 * BER Encoder: integer types encoding.
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_INT_DEFS
#ident "@(#)$Id$"
#define __ASN1_BER_ENCODER_INT_DEFS

#include "eyeline/asn1/TransferSyntax.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

using eyeline::asn1::TSLength;

/* ************************************************************************* *
 * Estimates length of BER encoding of integer value according to X.690 cl. 8.3
 * ************************************************************************* */
template < 
  class _TArg /* unsigned integer type, sizeof(_TArg) < 32 bytes */
>
uint8_t estimate_INTEGER(const _TArg & use_val)
{
  if (use_val < 0x80)
    return 1;

  //find MSB
  uint8_t msBit = sizeof(_TArg)*8;
  while (--msBit && !(use_val & (1 << msBit)));

  //check that 9 consequitive bits from MSB are not all set
  if (msBit >= 8) {
    uint8_t lmsBit = msBit;
    while (--lmsBit && (use_val & (1 << lmsBit)));
    while ((msBit - lmsBit) > 9) 
      msBit -= 9;
  }
  return (msBit + 1 + 6)/7;
}

/* ************************************************************************* *
 * Encodes by BER the integer value according to X.690 cl. 8.3
 * Returns  number of bytes of resulted encoding or zero in case of
 * insufficient length of buffer provided.
 * ************************************************************************* */
template <class _TArg /* unsigned integer type */>
uint8_t encode_INTEGER(const _TArg & use_val, uint8_t * use_enc, TSLength max_len)
{
  uint8_t rlen = estimate_INTEGER(use_val);
  if (rlen > max_len)
    return 0;

  if (use_val < 0x80) {
    *use_enc = (uint8_t)use_val;
    return 1;
  }

  //encode value - split it to groups of 7 bits
  _TArg uValue = use_val;
  uint16_t i = rlen; //here rlen >= 2

  use_enc[--i] = ((uint8_t)uValue & 0x7F);
  while ((--i) >= 0) {
    uValue >>= 7;
    use_enc[i] = ((uint8_t)uValue & 0x7F) | 0x80;
  }
  return rlen;
}

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_INT_DEFS */

