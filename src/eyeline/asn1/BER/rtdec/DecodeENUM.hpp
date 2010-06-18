/* ************************************************************************* *
 * BER Decoder: ENUMERATED type encoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_DECODER_ENUM
#ident "@(#)$Id: "
#define __ASN1_BER_DECODER_ENUM

#include "eyeline/asn1/BER/rtdec/DecodeIINT.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Encodes by BER/DER/CER the ENUMERATED value according to X.690
 * clause 8.4 (with appropriate DER/CER restrctions).
 * ************************************************************************* */
class DecoderOfENUM : public DecoderOfINTEGER {
public:
  DecoderOfENUM(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : DecoderOfINTEGER(asn1::_tagsENUM, use_rule)
  { }
  //Tagged ENUMERATED type encoder constructor
  DecoderOfENUM(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : DecoderOfINTEGER(ASTagging(use_tag, tag_env, asn1::_tagsENUM), use_rule)
  { }
  ~DecoderOfENUM()
  { }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_ENUM */
