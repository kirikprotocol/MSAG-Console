#ifndef __EYELINE_ASN1_BER_TESTS_DEC_MDCHOICETYPE1_HPP__
# define __EYELINE_ASN1_BER_TESTS_DEC_MDCHOICETYPE1_HPP__

# include <inttypes.h>

# include "eyeline/asn1/BER/tests/ChoiceType1.hpp"
# include "eyeline/asn1/BER/rtdec/DecodeBOOL.hpp"
# include "eyeline/asn1/BER/rtdec/DecodeINT.hpp"
# include "eyeline/asn1/BER/rtdec/DecodeChoice.hpp"
# include "eyeline/asn1/BER/rtdec/DecodersChoiceT.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {
namespace dec {

class MDChoiceType1 : public DecoderOfChoice_T<2> {
public:
  explicit MDChoiceType1(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : DecoderOfChoice_T<2>(use_rule), _value(NULL)
  {
    construct();
  }

  void setValue(ChoiceType1& value) {
    _value= &value;
    _chcDec.reset();
  }

private:
  void construct();
  virtual asn1::ber::TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */;

  class AltDecoder : public ChoiceOfDecoders2_T<DecoderOfINTEGER,
                                                DecoderOfBOOL> {
  public:
    Alternative_T<DecoderOfINTEGER>  case1() { return alternative0(); }
    Alternative_T<DecoderOfBOOL>     case2() { return alternative1(); }

    ConstAlternative_T<DecoderOfINTEGER> case1() const { return alternative0(); }
    ConstAlternative_T<DecoderOfBOOL>    case2()  const { return alternative1(); }
  };

  AltDecoder  _altDec;
  ChoiceType1* _value;
};

}}}}}

#endif
