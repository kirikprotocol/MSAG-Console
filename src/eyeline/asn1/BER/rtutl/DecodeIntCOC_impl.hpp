/* ************************************************************************* *
 * BER Runtime: integer value content octets decomposition algorithm.
 * ************************************************************************* */
#ifndef __ASN1_BER_TLV_DECODER_INT_ALGORITHM
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_TLV_DECODER_INT_ALGORITHM

#include "eyeline/asn1/TransferSyntax.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

using eyeline::asn1::TSLength;
using eyeline::asn1::DECResult;

typedef bool (*IsCondition_fp)(uint8_t u_val);

inline bool isFalse(uint8_t u_val)
{
  return (u_val == 0x00);
}

inline bool isTrue(uint8_t u_val)
{
  return (u_val != 0x00);
}

inline uint16_t countMSBits(const uint8_t * use_enc, uint16_t max_len,
                            IsCondition_fp fp_condition = isTrue)
{
  uint16_t rval = 0;

  for (uint16_t i = 0; i < max_len; ++i) {
    for (uint8_t k = 0; k < 8; ++k) {
      if (!fp_condition(use_enc[i] & (0x80 >> k)))
        return rval;
      ++rval;
    }
  }
  return rval;
}


//Accumulates content octets of INTEGER encoding.
//'max_len' cann't be aero upon entry
template <class _TArg /* some integer type */>
inline
DECResult accumulateIntCOC(_TArg & use_val, IsCondition_fp skip_condition,
                           const uint8_t * use_enc, TSLength max_len)
{
  DECResult rval(DECResult::decOk, 1);

  use_val = *use_enc;
  if (max_len == 1)
    return rval;

  uint16_t leadingBits = countMSBits(use_enc, (uint16_t)max_len, skip_condition);
  if (leadingBits >= 8)
    rval.nbytes = leadingBits/8;
  if ((max_len - leadingBits/8) > sizeof(_TArg))
    return DECResult(DECResult::decBadVal); //too long integer

  for (TSLength i = rval.nbytes; i < max_len; ++i) {
    use_val = (use_val << 8) | use_enc[i];
    ++rval.nbytes;
  }
  rval.status = (leadingBits > 8) ? DECResult::decOkRelaxed : DECResult::decOk;
  return rval;
}



/* ************************************************************************* *
 * Decomposes content octets of the positive integer value according to 
 * X.690 cl. 8.3.
 * 
 * NOTE: valid positive integer encoding takes [1 .. sizeof(_TArg)+1] bytes
 * ************************************************************************* */
// 'max_len' cann't be aero upon entry
template <class _TArg /* unsigned integer type */>
inline
DECResult decodeIntCOC_unsigned(_TArg & use_val, const uint8_t * use_enc, TSLength max_len)
{
  return accumulateIntCOC(use_val, isFalse, use_enc, max_len);
}

/* ************************************************************************* *
 * Decomposes content octets of the signed integer value according to 
 * X.690 cl. 8.3.
 * 
 * NOTE: valid signed integer encoding takes [1 .. sizeof(_TArg)] bytes
 * ************************************************************************* */
// 'max_len' cann't be aero upon entry

template <class _TArg /* signed integer type */>
inline
DECResult decodeIntCOC_signed(_TArg & use_val, const uint8_t * use_enc, TSLength max_len)
{
  bool      encSigned = (*use_enc & 0x80) != 0;
  DECResult rval = accumulateIntCOC(use_val, encSigned ? isTrue : isFalse, use_enc, max_len);

  if (rval.isOkRelaxed()) { //check signedness bit
    bool msBit = (use_val & ((_TArg)1 << ((sizeof(_TArg) << 3) - 1))) != 0;
    if (!encSigned && msBit) //encoded positive value is out of signed int range
      return DECResult(DECResult::decBadVal);

    if (encSigned && !msBit) { //need to expand signedness bit
      uint8_t i = sizeof(_TArg) << 3;
      while (--i && !(use_val & ((_TArg)1 << i))) 
        use_val |= ((_TArg)1 << i);
    }
  }
  return rval;
}

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_TLV_DECODER_INT_ALGORITHM */

