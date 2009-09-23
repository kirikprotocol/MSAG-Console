#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/proto/TCUserInfo.hpp"

namespace eyeline {
namespace tcap {
namespace proto {

/* ************************************************************************* *
 * class TCExternal implementation
 * ************************************************************************* */

//REQ: if use_rule == valRule, presentation > valNone, otherwise presentation == valDecoded
ENCResult TCExternal::EncodeASEValue(const ASExternalValue & use_val,
                             BITBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return ENCResult(ENCResult::encBadArg);
}

//REQ: presentation == valNone
//OUT: presentation (include all subcomponents) = valDecoded,
//NOTE: in case of decMoreInput, stores decoding context 
DECResult TCExternal::decode(const OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return DECResult(DECResult::decBadArg);
}

//REQ: presentation == valNone
//OUT: presentation (include all subcomponents) = valMixed | valDecoded
//NOTE: in case of valMixed keeps references to BITBuffer !!!
//NOTE: in case of decMoreInput, stores decoding context 
DECResult TCExternal::deferredDecode(const OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return DECResult(DECResult::decBadArg);
}


/* ************************************************************************* *
 * class TCUserInformation implementation
 * ************************************************************************* */

//REQ: if use_rule == valRule, presentation > valNone, otherwise presentation == valDecoded
ENCResult TCUserInformation::encode(OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return ENCResult(ENCResult::encBadArg);
}

//REQ: presentation == valNone
//OUT: presentation (include all subcomponents) = valDecoded,
//NOTE: in case of decMoreInput, stores decoding context 
DECResult TCUserInformation::decode(const OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return DECResult(DECResult::decBadArg);
}

//REQ: presentation == valNone
//OUT: presentation (include all subcomponents) = valMixed | valDecoded
//NOTE: in case of valMixed keeps references to BITBuffer !!!
//NOTE: in case of decMoreInput, stores decoding context 
DECResult TCUserInformation::deferredDecode(const OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return DECResult(DECResult::decBadArg);
}


} //proto
} //tcap
} //eyeline

