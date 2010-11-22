#ifndef __EYELINE_ASN1_BER_TESTS_ENC_MESEQTYPEEXPLICIT1_HPP__
# define __EYELINE_ASN1_BER_TESTS_ENC_MESEQTYPEEXPLICIT1_HPP__

# include "eyeline/asn1/BER/rtenc/EncodeINT.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeNULL.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"
# include "eyeline/asn1/BER/tests/SeqType1.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {
namespace enc {

class MESeqTypeExplicit1 : public EncoderOfSequence_T<3> {
public:
  explicit MESeqTypeExplicit1(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
  : EncoderOfSequence_T<3>(use_rule), _eA(use_rule),
    _eB(_tag_b, ASTagging::tagsEXPLICIT, use_rule),
    _eC(_tag_c, ASTagging::tagsEXPLICIT, use_rule)
  {
    construct();
  }

  explicit MESeqTypeExplicit1(const SeqType1& value,
             TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
  : EncoderOfSequence_T<3>(use_rule), _eA(use_rule),
    _eB(_tag_b, ASTagging::tagsEXPLICIT, use_rule),
    _eC(_tag_c, ASTagging::tagsEXPLICIT, use_rule)
  {
    construct();
    setValue(value);
  }

  MESeqTypeExplicit1(const ASTag & use_tag, ASTagging::Environment_e tag_env,
             TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
  : EncoderOfSequence_T<3>(use_tag, tag_env, use_rule), _eA(use_rule),
    _eB(_tag_b, ASTagging::tagsEXPLICIT, use_rule),
    _eC(_tag_c, ASTagging::tagsEXPLICIT, use_rule)
  {
    construct();
  }

  void setValue(const SeqType1& value);

private:
  void construct();

  static const ASTag _tag_b;
  static const ASTag _tag_c;

  EncoderOfINTEGER _eA;
  EncoderOfINTEGER _eB;
  EncoderOfNULL    _eC;
};

}}}}}

#endif
