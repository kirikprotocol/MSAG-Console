#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/asn1/BER/rtutl/TLVUtils.hpp"
#include "eyeline/asn1/BER/rtutl/DecodeIdents_impl.hpp"
#include "eyeline/asn1/BER/rtutl/DecodeIntCOC_impl.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Class TLParser implementation:
 * ************************************************************************* */
DECResult TLParser::decode_tag(ASTag & use_tag, bool & is_constructed, 
                     const uint8_t * use_enc, TSLength max_len)
{
  DECResult rval(DECResult::decOk);

  if (!max_len) {
    rval.status = DECResult::decMoreInput;
    return rval;
  }

  //decompose tag value
  rval.nbytes = 1;
  if ((use_enc[0] & 0x1F) < 0x1F) { //short tag
    use_tag._tagValue = use_enc[0] & 0x1F;
  } else {                          //long tag
    rval += decode_identifier(use_tag._tagValue, use_enc + 1, max_len - 1);
  }
  //decompose class & constructedness
  is_constructed = ((use_enc[0] & 0x20) != 0);
  use_tag._tagClass = static_cast<ASTag::TagClass_e>(use_enc[0] & 0xC0);

  return rval;
}

//Decodes 'Length determinant'
DECResult TLParser::decode_ld(LDeterminant & use_ld,
                             const uint8_t * use_enc, TSLength max_len)
{
  if (!max_len)
    return DECResult(DECResult::decMoreInput);

  DECResult rval(DECResult::decOk, 1);

  if (use_enc[0] < 0x80) { //definite, short form
    use_ld._ldForm = LDeterminant::frmDefinite;
    use_ld._valLen = use_enc[0];
    return rval;
  }
  if (use_enc[0] == 0x80) { //indefinite
    use_ld._ldForm = LDeterminant::frmIndefinite;
    use_ld._valLen = 0;
    return rval;
  }
  //if (enc[0] > 0x80) //definite, long form
  TSLength ldLen = use_enc[0] & 0x7F;
  if (ldLen > (max_len - 1)) {
    rval.status = DECResult::decMoreInput;
    return rval;
  }
  return rval += decodeIntCOC_unsigned(use_ld._valLen, use_enc + 1, ldLen);
}

//Decodes 'Tag' octets of TLV encoding
DECResult TLParser::decodeTOC(const uint8_t * use_enc, TSLength max_len) /*throw()*/
{
  DECResult rval = decode_tag(_tag, _isConstructed, use_enc, max_len);
  if (rval.isOk())
    _szoTag = rval.nbytes;
  return rval;
}
//Decodes 'Length' octets of TLV encoding
DECResult TLParser::decodeLOC(const uint8_t * use_enc, TSLength max_len) /*throw()*/
{
  DECResult rval = decode_ld(*this, use_enc, max_len);
  if (rval.isOk())
    _szoLOC = rval.nbytes;
  return rval;
}


//Decodes 'begin-of-content' octets ('T'+'L') of TLV encoding
//NOTE: result may be DECResult::decOkRelaxed!
DECResult TLParser::decodeBOC(const uint8_t * use_enc, TSLength max_len)
{
  DECResult rval = decodeTOC(use_enc, max_len);
  if (rval.isOk())
    rval += decodeLOC(use_enc + rval.nbytes, max_len - rval.nbytes);
  return rval;
}

//Decodes 'end-of-content' octets of TLV encoding
DECResult TLParser::decodeEOC(const uint8_t * use_enc, TSLength max_len) const
{
  DECResult rval(DECResult::decOk);
  if (isDefinite())
    return rval;

  if (max_len < 2) {
    rval.status = (!max_len || use_enc[0]) ?
                  DECResult::decBadEncoding : DECResult::decMoreInput;
  } else {
    if ((use_enc[0] || use_enc[1]))
      rval.status = DECResult::decBadEncoding;
    else
      rval.nbytes += 2;
  }
  return rval;
}

} //ber
} //asn1
} //eyeline

