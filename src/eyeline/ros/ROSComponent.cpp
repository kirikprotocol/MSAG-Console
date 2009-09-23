#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/ros/ROSComponent.hpp"

namespace eyeline {
namespace ros {

/* ************************************************************************* *
 * class ROSInvoke implementation
 * ************************************************************************* */

//REQ: if use_rule == valRule, presentation > valNone, otherwise presentation == valDecoded
ENCResult ROSInvoke::encode(OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return ENCResult(ENCResult::encBadArg);
}

//REQ: presentation == valNone
//OUT: presentation (include all subcomponents) = valDecoded,
//NOTE: in case of decMoreInput, stores decoding context 
DECResult ROSInvoke::decode(const OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return DECResult(DECResult::decBadArg);
}

//REQ: presentation == valNone
//OUT: presentation (include all subcomponents) = valMixed | valDecoded
//NOTE: in case of valMixed keeps references to BITBuffer !!!
//NOTE: in case of decMoreInput, stores decoding context 
DECResult ROSInvoke::deferredDecode(const OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return DECResult(DECResult::decBadArg);
}


/* ************************************************************************* *
 * class ROSResult implementation
 * ************************************************************************* */

//REQ: if use_rule == valRule, presentation > valNone, otherwise presentation == valDecoded
ENCResult ROSResult::encode(OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return ENCResult(ENCResult::encBadArg);
}

//REQ: presentation == valNone
//OUT: presentation (include all subcomponents) = valDecoded,
//NOTE: in case of decMoreInput, stores decoding context 
DECResult ROSResult::decode(const OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return DECResult(DECResult::decBadArg);
}

//REQ: presentation == valNone
//OUT: presentation (include all subcomponents) = valMixed | valDecoded
//NOTE: in case of valMixed keeps references to BITBuffer !!!
//NOTE: in case of decMoreInput, stores decoding context 
DECResult ROSResult::deferredDecode(const OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return DECResult(DECResult::decBadArg);
}


/* ************************************************************************* *
 * class ROSResultNL implementation
 * ************************************************************************* */

//REQ: if use_rule == valRule, presentation > valNone, otherwise presentation == valDecoded
ENCResult ROSResultNL::encode(OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return ENCResult(ENCResult::encBadArg);
}

//REQ: presentation == valNone
//OUT: presentation (include all subcomponents) = valDecoded,
//NOTE: in case of decMoreInput, stores decoding context 
DECResult ROSResultNL::decode(const OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return DECResult(DECResult::decBadArg);
}

//REQ: presentation == valNone
//OUT: presentation (include all subcomponents) = valMixed | valDecoded
//NOTE: in case of valMixed keeps references to BITBuffer !!!
//NOTE: in case of decMoreInput, stores decoding context 
DECResult ROSResultNL::deferredDecode(const OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return DECResult(DECResult::decBadArg);
}


/* ************************************************************************* *
 * class ROSError implementation
 * ************************************************************************* */

//REQ: if use_rule == valRule, presentation > valNone, otherwise presentation == valDecoded
ENCResult ROSError::encode(OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return ENCResult(ENCResult::encBadArg);
}

//REQ: presentation == valNone
//OUT: presentation (include all subcomponents) = valDecoded,
//NOTE: in case of decMoreInput, stores decoding context 
DECResult ROSError::decode(const OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return DECResult(DECResult::decBadArg);
}

//REQ: presentation == valNone
//OUT: presentation (include all subcomponents) = valMixed | valDecoded
//NOTE: in case of valMixed keeps references to BITBuffer !!!
//NOTE: in case of decMoreInput, stores decoding context 
DECResult ROSError::deferredDecode(const OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return DECResult(DECResult::decBadArg);
}


/* ************************************************************************* *
 * class ROSReject implementation
 * ************************************************************************* */

//REQ: if use_rule == valRule, presentation > valNone, otherwise presentation == valDecoded
ENCResult ROSReject::encode(OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return ENCResult(ENCResult::encBadArg);
}

//REQ: presentation == valNone
//OUT: presentation (include all subcomponents) = valDecoded,
//NOTE: in case of decMoreInput, stores decoding context 
DECResult ROSReject::decode(const OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return DECResult(DECResult::decBadArg);
}

//REQ: presentation == valNone
//OUT: presentation (include all subcomponents) = valMixed | valDecoded
//NOTE: in case of valMixed keeps references to BITBuffer !!!
//NOTE: in case of decMoreInput, stores decoding context 
DECResult ROSReject::deferredDecode(const OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return DECResult(DECResult::decBadArg);
}


} //ros
} //eyeline

