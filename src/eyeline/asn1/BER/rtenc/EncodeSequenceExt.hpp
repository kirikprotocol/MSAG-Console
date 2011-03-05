/* ************************************************************************* *
 * BER Encoder methods: extensible SEQUENCE type encoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_SEQUENCE_EXT
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_ENCODER_SEQUENCE_EXT

#include "eyeline/asn1/BER/rtenc/EncodeStructExt.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Encodes by BER/DER/CER the SEQUENCE type value according to X.690
 * clause 8.9, 8.10 (with appropriate DER/CER restrctions).
 * NOTE: if ASTagging is not set the standard [UNIVERSAL 16] tag goes to
 * resulting TLV encoding.
 * ************************************************************************* */

//Template for Encoder of extensible SEQUENCE type,
//All fields/elements housekeeping structures are allocated as class members at once
template <
  uint16_t _NumFieldsTArg    /* overall number of known fields */
>
class EncoderOfExtensibleSequence_T : public EncoderOfExtensibleStructure_T<_NumFieldsTArg> {
public:
  //Constructor for SEQUENCE/SEQUENCE OF type
  explicit EncoderOfExtensibleSequence_T(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfExtensibleStructure_T<_NumFieldsTArg>(asn1::_uniTagging().SEQOF, use_rule)
  { }
  //Constructor for tagged SEQUENCE/SEQUENCE OF type
  EncoderOfExtensibleSequence_T(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                    TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfExtensibleStructure_T<_NumFieldsTArg>(use_tag, tag_env, asn1::_uniTagging().SEQOF, use_rule)
  { }
  virtual ~EncoderOfExtensibleSequence_T()
  { }
};


} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_SEQUENCE_EXT */

