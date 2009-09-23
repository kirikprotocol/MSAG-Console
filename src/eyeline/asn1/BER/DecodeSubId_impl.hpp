/* ************************************************************************* *
 * BER Decoder: tag, subIdentifier, length determinant decoding.
 * ************************************************************************* */
#ifndef __ASN1_BER_DECODER_SUBID_DEFS
#ident "@(#)$Id$"
#define __ASN1_BER_DECODER_SUBID_DEFS

#include "eyeline/asn1/TransferSyntax.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Decodes by BER the unsigned value that is on of:
 * - Tag value grater than 30 (X.690 cl. 8.1.2.4)
 * - Length determinant value grater than 127 (X.690 cl. 8.1.3.5)
 * - OID subId value (X.690 cl. 8.19.2)
* Returns number of bytes processed from encoding or zero in case of failure.
 * ************************************************************************* */
template <class _TArg /* unsigned integer type */>
uint16_t decode_identifier(_TArg & use_val, const uint8_t * use_enc, TSLength enc_len)
{
  if (!enc_len)
    return 0;

  use_val = (_TArg)(*use_enc & 0x7F);
  if (*use_enc < 0x80)
    return 1;

  uint8_t l7b = *use_enc;
  unsigned i = 1;
  while ((i < enc_len) && ((i < ((sizeof(_TArg) << 3) + 6)/7))
         && ((l7b = use_enc[i++]) & 0x80) ) {
    use_val <<= 7;
    use_val |= (_TArg)(l7b & 0x7F);
  }
  return  (l7b < 0x80) ? i : 0;
}

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_SUBID_DEFS */

