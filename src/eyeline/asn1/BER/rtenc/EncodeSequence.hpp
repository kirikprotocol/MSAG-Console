/* ************************************************************************* *
 * BER Encoder methods: SEQUENCE/SEQUENCE OF types encoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_SEQUENCE
#ident "@(#)$Id$"
#define __ASN1_BER_ENCODER_SEQUENCE

#include "eyeline/asn1/BER/rtenc/EncodeStruct.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Encodes by BER/DER/CER the SEQUENCE/SEQUENCE OF value according to X.690
 * clause 8.9, 8.10 (with appropriate DER/CER restrctions).
 * NOTE: if ASTagging is not set the standard [UNIVERSAL 16] tag goes to
 * resulting TLV encoding.
 * ************************************************************************* */

//Template for Encoder of SEQUENCE/SEQUENCE OF type with fixed number of fields/elements,
//which all are untagged (so-called 'Basic structure').
//All fields/elements housekeeping structures are allocated as class members at once
template <
  uint16_t _NumFieldsTArg                         /* overall number of fields/elements */
>
class EncoderOfPlainSequence_T : public EncoderOfPlainStructure_T<_NumFieldsTArg> {
public:
  //Constructor for SEQUENCE/SEQUENCE OF type
  EncoderOfPlainSequence_T(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfPlainStructure_T<_NumFieldsTArg>(asn1::_tagsSEQOF, use_rule)
  { }
  //Constructor for tagged SEQUENCE/SEQUENCE OF type
  EncoderOfPlainSequence_T(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                    TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfPlainStructure_T<_NumFieldsTArg>(use_tag, tag_env, asn1::_tagsSEQOF, use_rule)
  { }
  virtual ~EncoderOfPlainSequence_T()
  { }
};

//Template for Encoder of SEQUENCE/SEQUENCE OF type with fixed number of fields/elements,
//which may be optionally tagged  (so-called 'Mixed structure').
//All fields/elements housekeeping structures are allocated as class members at once
template <
  uint16_t _NumFieldsTArg                         /* overall number of fields/elements */
, uint16_t _NumTaggedFieldsTArg = _NumFieldsTArg  /* number of tagged fields/elements */
>
class EncoderOfSequence_T : public EncoderOfStructure_T<_NumFieldsTArg, _NumTaggedFieldsTArg> {
public:
  //Constructor for SEQUENCE/SEQUENCE OF type
  EncoderOfSequence_T(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfStructure_T<_NumFieldsTArg, _NumTaggedFieldsTArg>(asn1::_tagsSEQOF, use_rule)
  { }
  //Constructor for tagged SEQUENCE/SEQUENCE OF type
  EncoderOfSequence_T(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                    TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfStructure_T<_NumFieldsTArg, _NumTaggedFieldsTArg>(use_tag, tag_env, asn1::_tagsSEQOF, use_rule)
  { }
  virtual ~EncoderOfSequence_T()
  { }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_SEQUENCE */

