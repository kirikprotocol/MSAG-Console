/* ************************************************************************* *
 * BER Runtime: integer value content octets decomposition algorithm.
 * ************************************************************************* */
#ifndef __ASN1_BER_TLV_DECODER_INT_ALGORITHM
#ident "@(#)$Id$"
#define __ASN1_BER_TLV_DECODER_INT_ALGORITHM

#include "eyeline/asn1/TransferSyntax.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

using eyeline::asn1::TSLength;
using eyeline::asn1::DECResult;

/* ************************************************************************* *
 * Decomposes content octets of the integer value according to X.690 cl. 8.3
 * ************************************************************************* */
template <class _TArg /* unsigned integer type */>
DECResult decodeCOC_UINTEGER(_TArg & use_val, const uint8_t * use_enc, TSLength max_len)
{
  DECResult rval(DECResult::decOk, 
                 (sizeof(_TArg) < max_len) ? (TSLength)sizeof(_TArg) : max_len);

  if (rval.nbytes) {
    use_val = use_enc[0];
    if (rval.nbytes > 1) {
      for (TSLength i = 1; i < rval.nbytes; ++i)
        use_val = (use_val << 8) + use_enc[i];
      if (!use_enc[0]) {
        if (!(use_enc[1] & 0x80)) //9 higher bits are all 0s !!!
          rval.status = DECResult::decOkRelaxed;
      } else {
        //check for two's complement encoding
        if (use_enc[0] & 0x80) { //signed number, coorect signedness of value
          for (unsigned i = (unsigned)rval.nbytes; i < (unsigned)sizeof(_TArg); ++i)
            use_val |= (_TArg)0xFF << (i<<3);
        }
        if ((use_enc[0] == 0xFF) && (use_enc[1] & 0x80)) //9 higher bits are all 1s !!!
          rval.status = DECResult::decOkRelaxed;
      }
    }
  } else
    rval.status = DECResult::decMoreInput;
  return rval;
}


} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_TLV_DECODER_INT_ALGORITHM */

