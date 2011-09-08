#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/asn1/BER/rtenc/EncodeSubIds.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeIdents_impl.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
// --------------------------------------------------------------------------
// Calculates length of BER encoding of array of subIds of RELATIVE-OID or
// OBJECT-IDENTIFIER value.
// Returns number of bytes of resulted encoding or zero in case of value
// is too long and cann't be encoded.
// --------------------------------------------------------------------------
uint16_t  estimate_SubIds(const SubIdType * use_vals, RelativeOID::size_type val_num)
{
  uint16_t rlen = 0;
  for (RelativeOID::size_type it = 0; it < val_num;  ++it) {
    uint16_t olen = rlen;
    if ((rlen += estimate_identifier(use_vals[it])) < olen)
      return 0; //value overloads uint16_t
  }
  return rlen;
}

// --------------------------------------------------------------------------
// Encodes by BER the array of subIds of RELATIVE-OID or
// OBJECT-IDENTIFIER value according to X.690 clause 8.19.2
// --------------------------------------------------------------------------
ENCResult encode_SubIds(const SubIdType * use_vals, RelativeOID::size_type val_num,
                        uint8_t * use_enc, TSLength max_len)
{
  ENCResult rval(ENCResult::encOk);

  for (RelativeOID::size_type it = 0; it < val_num;  ++it) {
    uint8_t subLen = encode_identifier(use_vals[it], use_enc + rval.nbytes,
                                       TSBuffer::adoptRange<uint8_t>(max_len - rval.nbytes));
    if (!subLen) {
      rval.status = ENCResult::encMoreMem;
      break;
    }
    rval.nbytes += subLen;
  }
  return rval;
}

} //ber
} //asn1
} //eyeline

