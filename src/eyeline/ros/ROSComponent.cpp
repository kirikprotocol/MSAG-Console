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
ASTypeAC::ENCResult
ROSInvoke::encode(OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return ASTypeAC::ENCResult();
}

//REQ: presentation == valNone
//OUT: presentation (include all subcomponents) = valDecoded,
//NOTE: in case of decMoreInput, stores decoding context 
ASTypeAC::DECResult
ROSInvoke::decode(const OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
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
ROSInvoke::deferredDecode(const OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return ASTypeAC::DECResult();
}


/* ************************************************************************* *
 * class ROSResult implementation
 * ************************************************************************* */

//REQ: if use_rule == valRule, presentation > valNone, otherwise presentation == valDecoded
ASTypeAC::ENCResult
ROSResult::encode(OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return ASTypeAC::ENCResult();
}

//REQ: presentation == valNone
//OUT: presentation (include all subcomponents) = valDecoded,
//NOTE: in case of decMoreInput, stores decoding context 
ASTypeAC::DECResult
ROSResult::decode(const OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
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
ROSResult::deferredDecode(const OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return ASTypeAC::DECResult();
}


/* ************************************************************************* *
 * class ROSResultNL implementation
 * ************************************************************************* */

//REQ: if use_rule == valRule, presentation > valNone, otherwise presentation == valDecoded
ASTypeAC::ENCResult
ROSResultNL::encode(OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return ASTypeAC::ENCResult();
}

//REQ: presentation == valNone
//OUT: presentation (include all subcomponents) = valDecoded,
//NOTE: in case of decMoreInput, stores decoding context 
ASTypeAC::DECResult
ROSResultNL::decode(const OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
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
ROSResultNL::deferredDecode(const OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return ASTypeAC::DECResult();
}


/* ************************************************************************* *
 * class ROSError implementation
 * ************************************************************************* */

//REQ: if use_rule == valRule, presentation > valNone, otherwise presentation == valDecoded
ASTypeAC::ENCResult
ROSError::encode(OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return ASTypeAC::ENCResult();
}

//REQ: presentation == valNone
//OUT: presentation (include all subcomponents) = valDecoded,
//NOTE: in case of decMoreInput, stores decoding context 
ASTypeAC::DECResult
ROSError::decode(const OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
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
ROSError::deferredDecode(const OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return ASTypeAC::DECResult();
}


/* ************************************************************************* *
 * class ROSReject implementation
 * ************************************************************************* */

//REQ: if use_rule == valRule, presentation > valNone, otherwise presentation == valDecoded
ASTypeAC::ENCResult
ROSReject::encode(OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return ASTypeAC::ENCResult();
}

//REQ: presentation == valNone
//OUT: presentation (include all subcomponents) = valDecoded,
//NOTE: in case of decMoreInput, stores decoding context 
ASTypeAC::DECResult
ROSReject::decode(const OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
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
ROSReject::deferredDecode(const OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return ASTypeAC::DECResult();
}


} //ros
} //eyeline

