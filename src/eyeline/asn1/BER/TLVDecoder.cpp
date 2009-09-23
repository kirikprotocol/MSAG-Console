#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/asn1/ASNTags.hpp"
#include "eyeline/asn1/BER/DecodeSubId_impl.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Decodes by BER the ASN.1 Tag according to X.680 clause 8.1.2. 
 * Returns number of bytes processed from encoding or zero in case of failure.
 * ************************************************************************* */
DECResult decode_tag(ASTag & use_tag, const uint8_t * use_enc, TSLength enc_len)
{
  DECResult rval(DECResult::decBadEncoding);

  if (enc_len) {
    use_tag._tagClass = static_cast<ASTag::TagClass_e>(use_enc[0] & 0xC0);
    use_tag._isConstructed = static_cast<ASTag::TagClass_e>(use_enc[0] & 0x60);
    
    if ((use_enc[0] & 0x1F) == 0x1F) {  //long tag grater than 30
      if (!(rval.nbytes = decode_identifier(use_tag._tagValue, use_enc + 1, enc_len - 1)))
        return rval;
    } else {            //short tag less than or equal to 30
      use_tag._tagValue = (ASTag::ValueType)(use_enc[0] & 0x1F);
      rval.nbytes = 1;
    }
  }
  rval.status = DECResult::decOk;
  return rval;
}

} //ber
} //asn1
} //eyeline

