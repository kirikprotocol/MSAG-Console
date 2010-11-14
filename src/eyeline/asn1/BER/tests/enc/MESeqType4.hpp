#ifndef __EYELINE_ASN1_BER_TESTS_ENC_MESEQTYPE4_HPP__
# define __EYELINE_ASN1_BER_TESTS_ENC_MESEQTYPE4_HPP__

# include "eyeline/asn1/BER/rtenc/EncodeINT.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeNULL.hpp"
# include "eyeline/asn1/BER/rtenc/EncoderProducer.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeSequenceExt.hpp"
# include "eyeline/asn1/BER/tests/SeqType4.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {
namespace enc {

class MESeqType4 : public EncoderOfExtensibleSequence_T<4> {
public:
  explicit MESeqType4(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
  : EncoderOfExtensibleSequence_T<4>(use_rule), _eA(use_rule)
  {
    construct();
  }

  MESeqType4(const ASTag & use_tag, ASTagging::Environment_e tag_env,
             TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
  : EncoderOfExtensibleSequence_T<4>(use_tag, tag_env, use_rule)
  {
    construct();
  }

  void setValue(const SeqType4& value);

private:
  void construct();

  static const ASTag _tag_b;
  static const ASTag _tag_c;

  EncoderOfINTEGER _eA;
  asn1::ber::EncoderProducer_T<EncoderOfINTEGER> _eB;
  asn1::ber::EncoderProducer_T<EncoderOfINTEGER> _eC;
};

}}}}}

#endif
