#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/proto/TCUniDialogue.hpp"

namespace eyeline {
namespace tcap {
namespace proto {

// {itu-t(0) recommendation(0) q(17) 773 as(1) unidialogue-as(2) version1(1)}
static uint8_t _octs_uniDialogue_as_id[] =
  { 7, 
    0x00,       //itu-t(0) recommendation(0)
    0x11,       //q(17)
    0x86, 0x05, //773
    0x01,       //as(1)
    0x02,       //unidialogue-as(2)
    0x01        //version1(1)
  };

EncodedOID _ac_tcap_uniDialogue_as(_octs_uniDialogue_as_id, "_ac_tcap_uniDialogue_as");

/* ************************************************************************* *
 * class TCAudtPDU implementation
 * ************************************************************************* */

//REQ: if use_rule == valRule, presentation > valNone, otherwise presentation == valDecoded
ASTypeAC::ENCResult
  TCAudtPDU::Encode(BITBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return ASTypeAC::ENCResult();
}

//REQ: presentation == valNone
//OUT: presentation (include all subcomponents) = valDecoded,
//NOTE: in case of decMoreInput, stores decoding context 
ASTypeAC::DECResult
  TCAudtPDU::Decode(const BITBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
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
  TCAudtPDU::DeferredDecode(const BITBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return ASTypeAC::DECResult();
}


/* ************************************************************************* *
 * class TCUniDialogueAS implementation
 * ************************************************************************* */

//REQ: if use_rule == valRule, presentation > valNone, otherwise presentation == valDecoded
ASTypeAC::ENCResult
  TCUniDialogueAS::Encode(BITBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return ASTypeAC::ENCResult();
}

//REQ: presentation == valNone
//OUT: presentation (include all subcomponents) = valDecoded,
//NOTE: in case of decMoreInput, stores decoding context 
ASTypeAC::DECResult
  TCUniDialogueAS::Decode(const BITBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
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
  TCUniDialogueAS::DeferredDecode(const BITBuffer & use_buf, EncodingRule use_rule/* = ruleDER*/)
  /*throw ASN1CodecError*/
{
  //TODO:
  return ASTypeAC::DECResult();
}


} //proto
} //tcap
} //eyeline

