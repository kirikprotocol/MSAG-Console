#ifndef __EYELINE_ASN1_BER_TESTS_ENC_MDSEQOFTYPE1_HPP__
# define __EYELINE_ASN1_BER_TESTS_ENC_MDSEQOFTYPE1_HPP__

# include "eyeline/asn1/BER/rtdec/DecodeSeqOfLinked.hpp"
# include "eyeline/asn1/BER/tests/SeqType1.hpp"
# include "eyeline/asn1/BER/tests/dec/MDSeqType1.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {
namespace dec {

class MDSeqOfType1 : public asn1::ber::DecoderOfSeqOfLinked_T<SeqType1, MDSeqType1> {
public:
  explicit MDSeqOfType1(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
  : asn1::ber::DecoderOfSeqOfLinked_T<SeqType1, MDSeqType1>(use_rule)
  {}

  MDSeqOfType1(const asn1::ASTag & use_tag, asn1::ASTagging::Environment_e tag_env,
                         asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
  : asn1::ber::DecoderOfSeqOfLinked_T<SeqType1, MDSeqType1>(use_tag, tag_env, use_rule)
  {}
};

}}}}}

#endif
