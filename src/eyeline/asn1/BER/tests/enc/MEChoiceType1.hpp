#ifndef __EYELINE_ASN1_BER_TESTS_ENC_MECHOICETYPE1_HPP__
# define __EYELINE_ASN1_BER_TESTS_ENC_MECHOICETYPE1_HPP__

# include <inttypes.h>
# include "eyeline/asn1/BER/rtenc/EncodeINT.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeBOOL.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeChoice.hpp"
# include "eyeline/asn1/BER/tests/ChoiceType1.hpp"
# include "eyeline/asn1/BER/rtenc/EncodersChoiceT.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {
namespace enc {

class MEChoiceType1 : public asn1::ber::EncoderOfChoice {
private:
  asn1::ber::EncoderOfChoice::setSelection;

protected:
  class AltEncoder : public ChoiceOfEncoders2_T<EncoderOfINTEGER, EncoderOfBOOL> {
  public:
    Alternative_T<EncoderOfINTEGER> case1()  { return alternative0(); }
    Alternative_T<EncoderOfBOOL>    case2()  { return alternative1(); }

    ConstAlternative_T<EncoderOfINTEGER> case1()  const { return alternative0(); }
    ConstAlternative_T<EncoderOfBOOL>    case2()  const { return alternative1(); }
  };

  AltEncoder  _altEnc;

public:
  explicit MEChoiceType1(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfChoice(use_rule)
  {
    addCanonicalAlternative(asn1::_tagBOOL, asn1::ASTagging::tagsIMPLICIT);
  }
  //
  MEChoiceType1(const asn1::ASTag& outer_tag,
                const asn1::ASTagging::Environment_e tag_env,
                asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfChoice(outer_tag, tag_env, use_rule)
  {
    addCanonicalAlternative(asn1::_tagBOOL, asn1::ASTagging::tagsIMPLICIT);
  }

  void setValue(const ChoiceType1& value);
private:
  void setCase1(int32_t value);
  void setCase2(bool value);
};

}}}}}

#endif
