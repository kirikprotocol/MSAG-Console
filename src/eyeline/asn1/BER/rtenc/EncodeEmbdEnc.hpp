/* ************************************************************************* *
 * BER Encoder: EmbeddedEncoding helper type encoder (used
 *              by EXTERNAL/EMBEDDED PDV decoders)
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_EMBEDDED_ENCODING
#ident "@(#)$Id$"
#define __ASN1_BER_ENCODER_EMBEDDED_ENCODING

#include "eyeline/asn1/EmbdEncoding.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeBITSTR.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeOCTSTR.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeASType.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeChoice.hpp"
#include "eyeline/asn1/BER/rtenc/EncodersChoiceT.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

//According to X.690 cl.8.18.1,
//EmbeddedEncoding type is defined in EXPLICIT tagging environment as follow:
//
//  EmbeddedEncoding ::= CHOICE {
//    single-ASN1-type  [0] ABSTRACT-SYNTAX.&Type,
//    octet-aligned     [1] IMPLICIT OCTET STRING,
//    arbitrary         [2] IMPLICIT BIT STRING
//  }
class EncoderOfEmbdEncoding : public EncoderOfChoice {
private:
  using asn1::ber::EncoderOfChoice::setSelection;

protected:
  class AltEncoder : public asn1::ber::ChoiceOfEncoders3_T<
                EncoderOfASType, EncoderOfOCTSTR, EncoderOfBITSTR> {
  public:
    Alternative_T<EncoderOfASType>  astype() { return alternative0(); }
    Alternative_T<EncoderOfOCTSTR>  octstr() { return alternative1(); }
    Alternative_T<EncoderOfBITSTR>  bitstr() { return alternative2(); }

    ConstAlternative_T<EncoderOfASType> astype()  const { return alternative0(); }
    ConstAlternative_T<EncoderOfOCTSTR> octstr()  const { return alternative1(); }
    ConstAlternative_T<EncoderOfBITSTR> bitstr()  const { return alternative2(); }
  };

  AltEncoder  _altEnc;

public:
  static const ASTag _tagSingleASN1Type;
  static const ASTag _tagOctetAligned;
  static const ASTag _tagBitAligned;

  EncoderOfEmbdEncoding(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfChoice(use_rule)
  {
    //add canonical alternative tagging (see EncodeChoice.hpp:NOTE.1)
    addCanonicalAlternative(ASTagging(_tagSingleASN1Type, ASTagging::tagsEXPLICIT));
  }
  ~EncoderOfEmbdEncoding()
  { }

  void setValue(TypeEncoderAC & use_val) /*throw(std::exception)*/;
  void setValue(const asn1::TransferSyntax & use_val) /*throw(std::exception)*/;
  void setValue(const asn1::OCTSTR::ArrayType & use_val) /*throw(std::exception)*/;
  void setValue(const asn1::BITSTR::ArrayType & use_val) /*throw(std::exception)*/;

  void setValue(const asn1::EmbeddedEncoding & use_val) /*throw(std::exception)*/;
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_EMBEDDED_ENCODING */

