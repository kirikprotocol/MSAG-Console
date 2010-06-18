/* ************************************************************************* *
 * BER Runtime: decoding of tag or subIdentifier.
 * ************************************************************************* */
#ifndef __ASN1_BER_TLV_DECODER_SUBID_ALGORITHM
#ident "@(#)$Id$"
#define __ASN1_BER_TLV_DECODER_SUBID_ALGORITHM

#include "eyeline/asn1/TransferSyntax.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

using eyeline::asn1::TSLength;
using eyeline::asn1::DECResult;
/* ************************************************************************* *
 * Decodes by BER the unsigned value that is on of:
 * - Tag value grater than 30 (X.690 cl. 8.1.2.4)
 * - OID subId value (X.690 cl. 8.19.2)
* Returns number of bytes processed from encoding or zero in case of failure.
 * ************************************************************************* */
template <
  class _TArg /* unsigned integer type, sizeof(_TArg) < 32 bytes */
>
DECResult decode_identifier(_TArg & use_val, const uint8_t * use_enc, TSLength enc_len)
{
  DECResult rval(DECResult::decOk);
  if (!enc_len) {
    rval.status = DECResult::decMoreInput;
    return rval;
  }

  use_val = (_TArg)(*use_enc & 0x7F);
  if (*use_enc < 0x80) {
    rval.nbytes = 1;
    return rval;
  }

  uint8_t l7b = *use_enc;
  uint8_t i = 1;
  while ((i < enc_len) && ((i < ((sizeof(_TArg) << 3) + 6)/7))
         && ((l7b = use_enc[i++]) & 0x80) ) {
    use_val <<= 7;
    use_val |= (_TArg)(l7b & 0x7F);
  }
  rval.nbytes = i;
  if (l7b & 0x80)
    rval.status = DECResult::decMoreInput;
  return rval;
}

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_TLV_DECODER_SUBID_ALGORITHM */

