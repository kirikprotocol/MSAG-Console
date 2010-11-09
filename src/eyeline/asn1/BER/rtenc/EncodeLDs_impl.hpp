/* ************************************************************************* *
 * BER Encoder: Length determinant encoding.
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_LDETERMINANT_DEFS
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_ENCODER_LDETERMINANT_DEFS

#include "eyeline/asn1/BER/rtenc/EncodeInt_impl.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Estimates number of octets of definite form of 'length determinant'
 * encoding according to X.690 cl. 8.1.3.
 * NOTE: the maximum length of encoding is limited to 127 bytes (cl. 8.1.3.5)
 * Returns:  number of bytes of 'length determinant' encoding.
 * ************************************************************************* */
template <
  class _TArg /* unsigned integer type, sizeof(_TArg) <= 126 bytes */
>
inline uint8_t estimate_ldeterminant(const _TArg & use_val)
{
  if (use_val < 0x80)
    return 1;

  return 1 + estimate_INTEGER(use_val);
}

/* ************************************************************************* *
 * Encodes the 'length determinant' in definite form (short or long)
 * according to X.690 cl. 8.1.3.
 * NOTE: the maximum length of encoding is limited to 127 bytes (cl. 8.1.3.5)
 * Returns  number of bytes of resulted encoding or zero in case of
 * insufficient length of buffer provided.
 * ************************************************************************* */
template <
  class _TArg /* unsigned integer type, sizeof(_TArg) <= 126 bytes */
>
inline uint8_t encode_ldeterminant(const _TArg & use_val, uint8_t * use_enc, uint8_t max_len)
{
  if (!max_len)
    return 0;

  if (use_val < 0x80) { //short form encoding
    use_enc[0] = (uint8_t)use_val;
    return 1;
  }
  //long form encoding: prefix octet + unsigned number encding
  uint8_t rlen = encode_INTEGER(use_val, use_enc + 1, max_len - 1);
  if (rlen) {
    //encode prefix octet of long form
    use_enc[0] = rlen | 0x80;
    ++rlen;
  }
  return rlen;
}

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_LDETERMINANT_DEFS */

