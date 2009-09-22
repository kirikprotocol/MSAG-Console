/* ************************************************************************* *
 * BER Encoder: tag, subIdentifier, length determinant encoding.
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_SUBID_DEFS
#ident "@(#)$Id$"
#define __ASN1_BER_ENCODER_SUBID_DEFS

#include "eyeline/asn1/TransferSyntax.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Estimates length of BER encoding of unsigned value that is on of:
 * - Tag value grater than 30 (X.690 cl. 8.1.2.4)
 * - Length determinant value grater than 127 (X.690 cl. 8.1.3.5)
 * - OID subId value (X.690 cl. 8.19.2)
 * Returns  number of bytes of resulted encoding or zero.
 * ************************************************************************* */
template <class _TArg /* unsigned integer type */>
uint16_t estimate_identifier(const _TArg & use_val)
{
  if (use_val < 0x80)
    return 1;

  uint16_t msBit = sizeof(_TArg)*8;
  while (--msBit && !(use_val & (1 << msBit)));

  return (msBit + 1 + 6)/7;
}

/* ************************************************************************* *
 * Encodes by BER the unsigned value that is on of:
 * - Tag value grater than 30 (X.690 cl. 8.1.2.4)
 * - Length determinant value grater than 127 (X.690 cl. 8.1.3.5)
 * - OID subId value (X.690 cl. 8.19.2)
 * Returns  number of bytes of resulted encoding or zero in case of
 * insufficient length of buffer provided.
 * ************************************************************************* */
template <class _TArg /* unsigned integer type */>
uint16_t encode_identifier(const _TArg & use_val, uint8_t * use_enc, TSLength max_len)
{
  if (use_val < 0x80) {
    if (max_len) {
      *use_enc = (uint8_t)use_val;
      return 1;
    }
    return 0;
  }

  uint16_t rlen = estimate_identifier(use_val);
  if (max_len < rlen)
    return 0;

  //encode value - split it to groups of 7 bits
  _TArg uValue = use_val;
  uint16_t i = rlen;

  use_enc[--i] = ((uint8_t)uValue & 0x7F);
  while ((--i) >= 1) {
    uValue >>= 7;
    use_enc[i] = ((uint8_t)uValue & 0x7F) | 0x80;
  }
  return rlen;
}

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_SUBID_DEFS */

