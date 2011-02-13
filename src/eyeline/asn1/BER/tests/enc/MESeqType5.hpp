#ifndef __EYELINE_ASN1_BER_TESTS_ENC_MESEQTYPE5_HPP__
# define __EYELINE_ASN1_BER_TESTS_ENC_MESEQTYPE5_HPP__

# include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"
# include "eyeline/asn1/BER/tests/SeqType5.hpp"
# include "eyeline/asn1/BER/tests/enc/MEChoiceType1.hpp"
# include "eyeline/asn1/BER/tests/enc/MEChoiceType2.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {
namespace enc {

class MESeqType5 : public EncoderOfSequence_T<4> {
public:
  explicit MESeqType5(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
  : EncoderOfSequence_T<4>(use_rule), _eA(use_rule), _eB(use_rule),
    _eC(_tag_c, ASTagging::tagsIMPLICIT, use_rule),
    _eD(_tag_d, ASTagging::tagsIMPLICIT, use_rule)
  {
    construct();
  }

  void setValue(const SeqType5& value);

private:
  void construct();

  static const ASTag _tag_c;
  static const ASTag _tag_d;

  MEChoiceType1 _eA;
  MEChoiceType2 _eB;
  MEChoiceType1 _eC;
  MEChoiceType2 _eD;
};

}}}}}

#endif
