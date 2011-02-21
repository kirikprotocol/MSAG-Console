#ifndef __EYELINE_ASN1_BER_TESTS_DEC_MDSEQTYPE5_HPP__
# define __EYELINE_ASN1_BER_TESTS_DEC_MDSEQTYPE5_HPP__

# include "eyeline/asn1/BER/rtdec/DecodeSeq.hpp"
# include "eyeline/asn1/BER/tests/SeqType5.hpp"
# include "eyeline/asn1/BER/tests/dec/MDChoiceType1.hpp"
# include "eyeline/asn1/BER/tests/dec/MDChoiceType2.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {
namespace dec {

class MDSeqType5 : public DecoderOfSequence_T<4> {
public:
  explicit MDSeqType5(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
  : asn1::ber::DecoderOfSequence_T<4>(use_rule),
    _value(NULL)
  {
    construct();
  }

  void setValue(SeqType5& value)
  {
    _value = &value;
  }

protected:
  void construct(void);
  virtual asn1::ber::TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */;

private:
  SeqType5* _value;

  MDChoiceType1 _dA;
  MDChoiceType2 _dB;
  MDChoiceType1 _dC;
  MDChoiceType2 _dD;

  class FieldA_TaggingOptions : public TaggingOptions {
  public:
    FieldA_TaggingOptions()
    {
      addTagging(_tagINTEGER, ASTagging::tagsIMPLICIT);
      addTagging(_tagBOOL, ASTagging::tagsIMPLICIT);
    }
  };

  static const FieldA_TaggingOptions _tagOptions;

  static const ASTag _choiceType2Tag;
  static const ASTag _tag_C;
  static const ASTag _tag_D;
};

}}}}}

#endif
