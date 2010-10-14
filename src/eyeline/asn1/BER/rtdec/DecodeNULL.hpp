/* ************************************************************************* *
 * BER Decoder: NULL type decoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_DECODER_NULL
#ident "@(#)$Id$"
#define __ASN1_BER_DECODER_NULL

#include "eyeline/asn1/BER/rtdec/TLVDecoder.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Encodes by BER/DER/CER the BOOLEAN value according to X.690
 * clause 8.8 (with appropriate DER/CER restrctions).
 * NOTE: if ASTagging is not set the standard [UNIVERSAL 5] tag goes to
 * resulting TLV encoding.
 * ************************************************************************* */
class DecoderOfNULL : public TypeValueDecoderAC {
protected:
  // -- ************************************* --
  // -- ValueDecoderIface interface methods
  // -- ************************************* --
  virtual DECResult decodeVAL(const TLVProperty * val_prop,
                              const uint8_t * use_enc, TSLength max_len,
                              TSGroupBER::Rule_e use_rule = TSGroupBER::ruleBER,
                              bool relaxed_rule = false)
    /*throw(BERDecoderException)*/;

public:
  //Constructor for NULL type
  DecoderOfNULL(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : TypeValueDecoderAC(asn1::_tagsNULL, use_rule)
  { }
  //Constructor for tagged NULL type
  DecoderOfNULL(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : TypeValueDecoderAC(ASTagging(use_tag, tag_env, asn1::_tagsNULL), use_rule)
  { }
  ~DecoderOfNULL()
  { }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_NULL */

