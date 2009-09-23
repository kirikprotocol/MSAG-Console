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
ENCResult TCMsgAbort::encode(OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return ENCResult(ENCResult::encBadArg);
}

//REQ: presentation == valNone
//OUT: presentation (include all subcomponents) = valDecoded,
//NOTE: in case of decMoreInput, stores decoding context 
DECResult TCMsgAbort::decode(const OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return DECResult(DECResult::decBadArg);
}

//REQ: presentation == valNone
//OUT: presentation (include all subcomponents) = valMixed | valDecoded
//NOTE: in case of valMixed keeps references to BITBuffer !!!
//NOTE: in case of decMoreInput, stores decoding context 
DECResult TCMsgAbort::deferredDecode(const OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return DECResult(DECResult::decBadArg);
}


} //proto
} //tcap
} //eyeline

