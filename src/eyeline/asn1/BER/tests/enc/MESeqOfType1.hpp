#ifndef __EYELINE_ASN1_BER_TESTS_ENC_MESEQOFTYPE1_HPP__
# define __EYELINE_ASN1_BER_TESTS_ENC_MESEQOFTYPE1_HPP__

# include "eyeline/asn1/BER/rtenc/EncodeSeqOfLinked.hpp"
# include "eyeline/asn1/BER/tests/SeqType1.hpp"
# include "eyeline/asn1/BER/tests/enc/MESeqType1.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {
namespace enc {

class MESeqOfType1 : public EncoderOfSeqOfLinked_T<SeqType1, MESeqType1, 2> {
public:
  static const unsigned _maxCount = 5;

  explicit MESeqOfType1(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : EncoderOfSeqOfLinked_T<SeqType1, MESeqType1, 2>(use_rule)
  {
    setMaxElements(_maxCount);
  }

  MESeqOfType1(const asn1::ASTag& outer_tag,
               const asn1::ASTagging::Environment_e tag_env,
               asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : EncoderOfSeqOfLinked_T<SeqType1, MESeqType1, 2>(outer_tag, tag_env, use_rule)
  {
    setMaxElements(_maxCount);
  }
};

}}}}}

#endif
