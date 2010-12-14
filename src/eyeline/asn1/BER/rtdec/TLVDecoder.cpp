#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/asn1/BER/rtdec/TLVDecoder.hpp"
#include "eyeline/asn1/BER/rtdec/TaggingDecoder.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

bool checkEOC(const uint8_t * use_enc, TSLength max_len) /*throw()*/
{
  return (max_len > 1)  ? (!use_enc[0] && !use_enc[1]) : false;
}

//Searches 'content octets' of primitive encoding for EOC octets (two zeroes)
//Returns number of bytes preceeding EOC if it's found
//
// T,L, COC, EOC
//    ^ --> ^
DECResult searchEOC(const uint8_t * use_enc, TSLength max_len)
{
  for (TSLength i = 0; i < max_len; ++i) {
    if (!use_enc[i] && ((i + 1) < max_len) && !use_enc[i+1])
      return DECResult(DECResult::decOk, i);
  }
  return DECResult(DECResult::decMoreInput);
}

//Searches 'content octets' of constructed encoding for outermost EOC octets (two zeroes)
//Returns number of bytes preceeding outermost EOC if it's found
//
// T,L
// --> T,L, COC, EOC
//  |  T,L, COC, EOC
// --> T,L, COC, EOC
// EOC 
//     
DECResult searchEOCconstructed(const uint8_t * use_enc, TSLength max_len, bool relaxed_rule/* = true */)
{
  DECResult rval(DECResult::decOk);
  while (rval.isOk(relaxed_rule)) {
    if ((max_len - rval.nbytes) < 2)
      return DECResult(DECResult::decMoreInput);
    if (checkEOC(use_enc + rval.nbytes, max_len - rval.nbytes))
      break;
    rval += skipTLV(use_enc + rval.nbytes, max_len - rval.nbytes, relaxed_rule);
  }
  return rval;
}

//Returns number of bytes TLV occupies.
//
// T,L, COC, EOC
// ^    -->    ^
DECResult skipTLV(const uint8_t * use_enc, TSLength max_len, bool relaxed_rule/* = true */)
{
  TLParser  tlProp;
  DECResult rval(DECResult::decOk);

  tlProp.decodeBOC(use_enc, max_len);
  if (!rval.isOk(relaxed_rule))
    return rval;

  if (tlProp.isDefinite()) {
    if ((rval.nbytes + tlProp._valLen) < max_len)
      rval.status = DECResult::decMoreInput;
    else
      rval.nbytes += tlProp._valLen;
    return rval;
  }
  //indefinite LDForm_e, search for EOCs
  if (tlProp._isConstructed)
    rval += searchEOCconstructed(use_enc + rval.nbytes, max_len - rval.nbytes, relaxed_rule);
  else
    rval += searchEOC(use_enc + rval.nbytes, max_len - rval.nbytes);

  if (rval.isOk(relaxed_rule))
    rval.nbytes += 2;   //count EOCs
  return rval;
}

/* ************************************************************************* *
 * Class TypeDecoderAC implementation:
 * ************************************************************************* */
DECResult TypeDecoderAC::decode(const uint8_t * use_enc, TSLength max_len)
  /*throw(BERDecoderException)*/
{
  DECResult       rval(DECResult::decOk);
  TaggingDecoder  tagDec(refreshTagging(), _outerTL);

  rval += tagDec.decodeBOC(use_enc, max_len, _relaxedRule); //throws
  if (rval.isOk(_relaxedRule)) {
    rval += _valDec->decodeVAL(tagDec.getVProperties(), use_enc + rval.nbytes, max_len - rval.nbytes,
                             getVALRule(), _relaxedRule); //throws
    if (rval.isOk(_relaxedRule))
      rval += tagDec.decodeEOC(use_enc + rval.nbytes, max_len - rval.nbytes);
  }
  return rval;
}

} //ber
} //asn1
} //eyeline

