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
  class _TArg /* signed integer type, sizeof(_TArg) < 32 bytes */
>
inline uint8_t estimate_signed_INTEGER(const _TArg & use_val)
{
  uint8_t highBit = (uint8_t)((sizeof(_TArg)<<3) - 1);
  uint8_t msBit = highBit;
  if (use_val & (1 << highBit)) { //'two's complement' number
    //check that 9 consequitive bits starting from MSB are not
    //all set, in order to get fewest meaningfull octets
    if (msBit > 7) {
      while (--msBit && (use_val & (1 << msBit)))
        ;
    }
  } else {                      //positive number
    //check that 9 consequitive bits starting from MSB are not
    //all unSet, in order to get fewest meaningfull octets
    while (--msBit && !(use_val & (1 << msBit)))
      ;
    //check that MSB isn't higher bit of octet
    if (!((msBit+1)%8) && (msBit < highBit))
      msBit += 8;
  }
  return ((msBit + 1) + 7) >> 3;
}


template < 
  class _TArg /* unsigned integer type, sizeof(_TArg) < 32 bytes */
>
inline uint8_t estimate_unsigned_INTEGER(const _TArg & use_val)
{
  if (use_val < 0x80)
    return 1;

  uint8_t msBit = (uint8_t)((sizeof(_TArg)<<3) - 1);
  if (use_val & (1 << msBit)) {
    //unsigned value conflicts with 'two's complement' number of same sizeof()
    //add leading zero byte
    return 1 + (((msBit + 1) + 7) >> 3);
  }
  //check that 9 consequitive bits starting from MSB are not
  //all unSet, in order to get fewest meaningfull octets
  while (--msBit && !(use_val & (1 << msBit)))
    ;
  return ((msBit + 1) + 7) >> 3;
}

// -----------------------------------------------
// Specialization for uint8_t
// -----------------------------------------------
template <> inline
uint8_t estimate_signed_INTEGER(const int8_t & use_val)
{
  return 1;
}

template <> inline
uint8_t estimate_unsigned_INTEGER(const uint8_t & use_val)
{
  return (use_val < 0x80) ? 1 : 2;
}


/* ************************************************************************* *
 * Encodes by BER the integer value according to X.690 cl. 8.3
 * Returns  number of bytes of resulted encoding or zero in case of
 * insufficient length of buffer provided.
 * ************************************************************************* */
template <
  class _TArg /* signed integer type */
>
inline uint8_t 
  encode_signed_INTEGER(const _TArg & use_val, uint8_t * use_enc, TSLength max_len)
{
  uint8_t rlen = estimate_signed_INTEGER(use_val);
  if (rlen > max_len)
    return 0;

  _TArg     uValue = use_val;
  uint16_t  i = rlen; // always >= 1

  do { //encode value - from MSB to LSB
    use_enc[--i] = (uint8_t)(uValue & 0xFF);
    uValue >>= 8;
  } while (i);
  return rlen;
}

template <
  class _TArg /* unsigned integer type */
>
inline uint8_t
  encode_unsigned_INTEGER(const _TArg & use_val, uint8_t * use_enc, TSLength max_len)
{
  if (!max_len)
    return 0;

  if (use_val < 0x80) {
    *use_enc = (uint8_t)use_val;
    return 1;
  }

  uint8_t rlen = 0, i = 0;
  uint8_t msBit = (uint8_t)(sizeof(_TArg)<<3) - 1;

  if (use_val & (1 << msBit)) {
    //unsigned value conflicts with 'two's complement' number of same sizeof()
    //add leading zero byte
    *use_enc++ = 0;
    i = (uint8_t)sizeof(_TArg);
    rlen = i + 1;
  } else {
    //check that 9 consequitive bits starting from MSB are not
    //all unSet, in order to get fewest meaningfull octets
    while (--msBit && !(use_val & (1 << msBit)))
      ;
    i = rlen = ((msBit + 1) + 7) >> 3;
  }
  if (rlen > max_len)
    return 0;

  _TArg uValue = use_val;
  do { //encode value - from MSB to LSB
    use_enc[--i] = (uint8_t)uValue;
    uValue >>= 8;
  } while (i);

  return rlen;
}

// -----------------------------------------------
// Specialization for uint8_t
// -----------------------------------------------
template <> inline
uint8_t encode_signed_INTEGER(const int8_t & use_val, uint8_t * use_enc, TSLength max_len)
{
  *use_enc = use_val;
  return 1;
}

template <> inline
uint8_t encode_unsigned_INTEGER(const uint8_t & use_val, uint8_t * use_enc, TSLength max_len)
{
  if (max_len < estimate_unsigned_INTEGER(use_val))
    return 0;
  if (use_val & 0x80) {
    use_enc[0] = 0;
    use_enc[1] = use_val;
    return 2;
  }
  *use_enc = use_val;
  return 1;
}


} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_INT_DEFS */

