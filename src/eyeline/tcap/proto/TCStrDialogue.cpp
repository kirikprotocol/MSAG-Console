#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/proto/TCStrDialogue.hpp"

namespace eyeline {
namespace tcap {
namespace proto {

// {itu-t(0) recommendation(0) q(17) 773 as(1) dialogue-as(1) version1(1)}
static uint8_t _octs_strDialogue_as_id[] =
  { 7, 
    0x00,       //itu-t(0) recommendation(0)
    0x11,       //q(17)
    0x86, 0x05, //773
    0x01,       //as(1)
    0x01,       //dialogue-as(1)
    0x01        //version1(1)
  };

EncodedOID _ac_tcap_strDialogue_as(_octs_strDialogue_as_id, "_ac_tcap_strDialogue_as");

/* ************************************************************************* *
 * class TCReqPDU implementation
 * ************************************************************************* */

//REQ: if use_rule == valRule, presentation > valNone, otherwise presentation == valDecoded
ENCResult TCReqPDU::encode(OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return ENCResult(ENCResult::encBadArg);
}

//REQ: presentation == valNone
//OUT: presentation (include all subcomponents) = valDecoded,
//NOTE: in case of decMoreInput, stores decoding context 
DECResult TCReqPDU::decode(const OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return DECResult(DECResult::decBadArg);
}

//REQ: presentation == valNone
//OUT: presentation (include all subcomponents) = valMixed | valDecoded
//NOTE: in case of valMixed keeps references to BITBuffer !!!
//NOTE: in case of decMoreInput, stores decoding context 
DECResult TCReqPDU::deferredDecode(const OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return DECResult(DECResult::decBadArg);
}

/* ************************************************************************* *
 * class TCRespPDU implementation
 * ************************************************************************* */

//REQ: if use_rule == valRule, presentation > valNone, otherwise presentation == valDecoded
ENCResult TCRespPDU::encode(OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return ENCResult(ENCResult::encBadArg);
}

//REQ: presentation == valNone
//OUT: presentation (include all subcomponents) = valDecoded,
//NOTE: in case of decMoreInput, stores decoding context 
DECResult TCRespPDU::decode(const OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return DECResult(DECResult::decBadArg);
}

//REQ: presentation == valNone
//OUT: presentation (include all subcomponents) = valMixed | valDecoded
//NOTE: in case of valMixed keeps references to BITBuffer !!!
//NOTE: in case of decMoreInput, stores decoding context 
DECResult TCRespPDU::deferredDecode(const OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return DECResult(DECResult::decBadArg);
}

/* ************************************************************************* *
 * class TCAbrtPDU implementation
 * ************************************************************************* */

//REQ: if use_rule == valRule, presentation > valNone, otherwise presentation == valDecoded
ENCResult TCAbrtPDU::encode(OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return ENCResult(ENCResult::encBadArg);
}

//REQ: presentation == valNone
//OUT: presentation (include all subcomponents) = valDecoded,
//NOTE: in case of decMoreInput, stores decoding context 
DECResult
TCAbrtPDU::decode(const OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return DECResult(DECResult::decBadArg);
}

//REQ: presentation == valNone
//OUT: presentation (include all subcomponents) = valMixed | valDecoded
//NOTE: in case of valMixed keeps references to BITBuffer !!!
//NOTE: in case of decMoreInput, stores decoding context 
DECResult TCAbrtPDU::deferredDecode(const OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return DECResult(DECResult::decBadArg);
}


/* ************************************************************************* *
 * class TCStrDialogueAS implementation
 * ************************************************************************* */

//REQ: if use_rule == valRule, presentation > valNone, otherwise presentation == valDecoded
ENCResult TCStrDialogueAS::encode(OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return ENCResult(ENCResult::encBadArg);
}

//REQ: presentation == valNone
//OUT: presentation (include all subcomponents) = valDecoded,
//NOTE: in case of decMoreInput, stores decoding context 
DECResult TCStrDialogueAS::decode(const OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return DECResult(DECResult::decBadArg);
}

//REQ: presentation == valNone
//OUT: presentation (include all subcomponents) = valMixed | valDecoded
//NOTE: in case of valMixed keeps references to BITBuffer !!!
//NOTE: in case of decMoreInput, stores decoding context 
DECResult TCStrDialogueAS::deferredDecode(const OCTBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return DECResult(DECResult::decBadArg);
}


} //proto
} //tcap
} //eyeline

