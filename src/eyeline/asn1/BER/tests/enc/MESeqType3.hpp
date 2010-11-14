#ifndef __EYELINE_ASN1_BER_TESTS_ENC_MESEQTYPE3_HPP__
# define __EYELINE_ASN1_BER_TESTS_ENC_MESEQTYPE3_HPP__

# include "eyeline/asn1/BER/rtenc/EncodeINT.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeNULL.hpp"
# include "eyeline/asn1/BER/rtenc/EncoderProducer.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeSequenceExt.hpp"
# include "eyeline/asn1/BER/tests/SeqType3.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {
namespace enc {

class MESeqType3 : public EncoderOfExtensibleSequence_T<3> {
public:
  explicit MESeqType3(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
  : EncoderOfExtensibleSequence_T<3>(use_rule), _eA(use_rule)
  {
    construct();
  }

  MESeqType3(const ASTag & use_tag, ASTagging::Environment_e tag_env,
             TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
  : EncoderOfExtensibleSequence_T<3>(use_tag, tag_env, use_rule)
  {
    construct();
  }

  void setValue(const SeqType3& value);

private:
  void construct();

  static const ASTag _tag_b;

  EncoderOfINTEGER _eA;
  asn1::ber::EncoderProducer_T<EncoderOfINTEGER> _eB;
};

}}}}}

#endif
