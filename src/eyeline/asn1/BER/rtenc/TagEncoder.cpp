#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/util/IntConv.hpp"
#include "eyeline/asn1/BER/rtenc/TagEncoder.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeIdents_impl.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Estimates length of BER encoding of ASN.1 Tag:
 * Returns  number of bytes of resulted encoding or zero.
 * ************************************************************************* */
uint8_t estimate_tag(const ASTag & use_tag)
{
  if (use_tag._tagValue <= 30)  //short tag form
    return 1;
  return estimate_identifier(use_tag._tagValue);
}

/* ************************************************************************* *
 * Composes the 'Tag-octets' according to X.690 clause 8.1.2.
 * Returns zero in case of insuffient buffer is provided.
 * NOTE: 'use_enc' & 'max_len' argument shouldn't be zero.
 * ************************************************************************* */
uint8_t compose_toc(const ASTag & use_tag, bool is_constructed,
                                   uint8_t * use_enc, uint8_t max_len)
{
  uint8_t rval = 0;
  if (use_tag._tagValue <= 30) {
    //short tag form, compose 1st byte
    *use_enc = (uint8_t)use_tag._tagValue;
  } else {
    //long tag form (tag > 30)
    if (!(rval = encode_identifier(use_tag._tagValue, use_enc + 1, max_len - 1)))
      return 0;
    //compose 1st byte
    *use_enc = 0x1F;
  }
  //complete 1st byte: class & constructedness
  *use_enc |= (uint8_t)use_tag._tagClass << 6;
  if (is_constructed)
    *use_enc |= 0x20;
  ++rval;
  return rval;
}

/* ************************************************************************* *
 * Encodes by BER the ASN.1 Tag according to X.690 clause 8.1.2.
 * ************************************************************************* */
ENCResult encode_tag(const ASTag & use_tag, bool is_constructed, 
                     uint8_t * use_enc, TSLength max_len)
{
  ENCResult rval(ENCResult::encOk);

  if (!max_len || !use_enc) {
    rval.status = ENCResult::encBadArg;
  } else if (!(rval.nbytes = compose_toc(use_tag, is_constructed, use_enc,
                                                 DOWNCAST_UNSIGNED(max_len, uint8_t)))) {
    rval.status = ENCResult::encMoreMem;
  }
  return rval;
}

} //ber
} //asn1
} //eyeline

