/* ************************************************************************* *
 * BER Decoder: ANY and ABSTRACT-SYNTAX.&Type (Opentype) decoders.
 * ************************************************************************* */
#ifndef __ASN1_BER_DECODER_ASTYPE
#ident "@(#)$Id$"
#define __ASN1_BER_DECODER_ASTYPE

#include "eyeline/asn1/BER/rtdec/TLVDecoder.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

class DecoderOfASType : public TypeValueDecoderAC {
protected:
  TransferSyntax * _valDec;

  DECResult decodeVAL(const TLVProperty * val_prop,
                      const uint8_t * use_enc, TSLength max_len,
                      TSGroupBER::Rule_e use_rule = TSGroupBER::ruleBER,
                      bool relaxed_rule = false)
    /*throw(BERDecoderException)*/;

public:
  // constructor for untagged ANY/OpenType
  DecoderOfASType(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : TypeValueDecoderAC(0, use_rule), _valDec(0)
  { }
  // constructor for tagged ANY/OpenType
  DecoderOfASType(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                  TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeValueDecoderAC(ASTagging(use_tag, tag_env), use_rule), _valDec(0)
  { }
  ~DecoderOfASType()
  { }

  void setValue(TransferSyntax & use_val) { _valDec = &use_val; }
};

typedef DecoderOfASType DecoderOfANY;

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_ASTYPE */

