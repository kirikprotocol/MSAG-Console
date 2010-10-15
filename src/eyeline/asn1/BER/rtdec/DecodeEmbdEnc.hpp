/* ************************************************************************* *
 * BER Decoder: EmbeddedEncoding helper type decoder (used
 *              by EXTERNAL/EMBEDDED PDV decoders)
 * ************************************************************************* */
#ifndef __ASN1_BER_DECODER_EMBEDDED_ENCODING
#ident "@(#)$Id$"
#define __ASN1_BER_DECODER_EMBEDDED_ENCODING

#include "eyeline/asn1/EmbdEncoding.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeBITSTR.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeOCTSTR.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeASType.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeChoice.hpp"
#include "eyeline/asn1/BER/rtdec/DecodersChoiceT.hpp"


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
class DecoderOfEmbdEncoding : public DecoderOfChoice_T<3> {
protected:
  class AltDecoder : public asn1::ber::ChoiceOfDecoders3_T<
                DecoderOfASType, DecoderOfOCTSTR, DecoderOfBITSTR> {
  public:
    Alternative_T<DecoderOfASType>  astype() { return alternative0(); }
    Alternative_T<DecoderOfOCTSTR>  octstr() { return alternative1(); }
    Alternative_T<DecoderOfBITSTR>  bitstr() { return alternative2(); }

    ConstAlternative_T<DecoderOfASType> astype()  const { return alternative0(); }
    ConstAlternative_T<DecoderOfOCTSTR> octstr()  const { return alternative1(); }
    ConstAlternative_T<DecoderOfBITSTR> bitstr()  const { return alternative2(); }
  };

  AltDecoder  _altDec;
  /* -- */
  asn1::EmbeddedEncoding * _dVal;
  //Initializes ElementDecoder of this type;
  void construct(void);

  // ----------------------------------------
  // -- DecoderOfChoiceAC interface methods
  // ----------------------------------------
  //If necessary, allocates alternative and initializes associated TypeDecoderAC
  virtual TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception)*/;

public:
  static const ASTag _tagSingleASN1Type;
  static const ASTag _tagOctetAligned;
  static const ASTag _tagBitAligned;

  class TaggingOptions : public asn1::ber::TaggingOptions {
  public:
    TaggingOptions() : asn1::ber::TaggingOptions()
    {
      addTagging(_tagSingleASN1Type, asn1::ASTagging::tagsEXPLICIT);
      addTagging(_tagOctetAligned, asn1::ASTagging::tagsIMPLICIT);
      addTagging(_tagBitAligned, asn1::ASTagging::tagsIMPLICIT);
    }
  };

  static const TaggingOptions _tagOptions;

  explicit DecoderOfEmbdEncoding(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : DecoderOfChoice_T<3>(use_rule), _dVal(0)
  {
    construct();
  }
  explicit DecoderOfEmbdEncoding(asn1::EmbeddedEncoding & use_val,
                        TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : DecoderOfChoice_T<3>(use_rule), _dVal(&use_val)
  {
    construct();
  }
  //
  ~DecoderOfEmbdEncoding()
  { }

  void setValue(asn1::EmbeddedEncoding & use_val)
  {
    _dVal = &use_val; 
    _chcDec.reset();
  }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_EMBEDDED_ENCODING */

