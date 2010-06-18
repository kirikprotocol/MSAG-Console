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
public:
  static const ASTag _tagSingleASN1Type;
  static const ASTag _tagOctetAligned;
  static const ASTag _tagBitAligned;

private:
  union {
    void *  _aligner;
    uint8_t _buf[eyeline::util::MaxSizeOf3_T<DecoderOfASType, DecoderOfOCTSTR, DecoderOfBITSTR>::VALUE];
  } _memAlt;

protected:
  asn1::EmbeddedEncoding * _dVal;
  /* -- */
  union {
    TypeDecoderAC *   _ptr;
    DecoderOfASType * _astype;
    DecoderOfOCTSTR * _octstr;
    DecoderOfBITSTR * _bitstr;
  } _altDec;


  void cleanUp(void)
  {
    if (_altDec._ptr) {
      _altDec._ptr->~TypeDecoderAC();
      _altDec._ptr = NULL;
    }
  }
  //Initializes ElementDecoder of this type;
  void construct(void);

  // ----------------------------------------
  // -- DecoderOfChoiceAC interface methods
  // ----------------------------------------
  //If necessary, allocates alternative and initializes associated TypeDecoderAC
  virtual TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception)*/;

public:
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
    _memAlt._aligner = _altDec._ptr = 0;
    construct();
  }
  DecoderOfEmbdEncoding(asn1::EmbeddedEncoding & use_val,
                        TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : DecoderOfChoice_T<3>(use_rule), _dVal(&use_val)
  {
    _memAlt._aligner = _altDec._ptr = 0;
    construct();
  }
  //
  ~DecoderOfEmbdEncoding()
  {
    cleanUp();
  }

  void setValue(asn1::EmbeddedEncoding & use_val) { _dVal = &use_val; }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_EMBEDDED_ENCODING */

