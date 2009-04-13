#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/proto/TCAPAbort.hpp"

namespace eyeline {
namespace tcap {
namespace proto {

/* ************************************************************************* *
 * class TCMsgAbort implementation
 * ************************************************************************* */

//REQ: if use_rule == valRule, presentation > valNone, otherwise presentation == valDecoded
ASTypeAC::ENCResult
  TCMsgAbort::Encode(BITBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return ASTypeAC::ENCResult();
}

//REQ: presentation == valNone
//OUT: presentation (include all subcomponents) = valDecoded,
//NOTE: in case of decMoreInput, stores decoding context 
ASTypeAC::DECResult
  TCMsgAbort::Decode(const BITBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return ASTypeAC::DECResult();
}

//REQ: presentation == valNone
//OUT: presentation (include all subcomponents) = valMixed | valDecoded
//NOTE: in case of valMixed keeps references to BITBuffer !!!
//NOTE: in case of decMoreInput, stores decoding context 
ASTypeAC::DECResult
  TCMsgAbort::DeferredDecode(const BITBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return ASTypeAC::DECResult();
}


} //proto
} //tcap
} //eyeline

