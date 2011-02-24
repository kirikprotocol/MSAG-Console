#ifndef __EYELINE_ASN1_BER_TESTS_DEC_MDSEQTYPE3_HPP__
# define __EYELINE_ASN1_BER_TESTS_DEC_MDSEQTYPE3_HPP__

# include "eyeline/asn1/BER/rtdec/DecodeINT.hpp"
# include "eyeline/asn1/BER/rtdec/DecodeSeq.hpp"
# include "eyeline/asn1/BER/rtdec/DecoderProducer.hpp"
# include "eyeline/asn1/BER/rtdec/DecodeUExt.hpp"
# include "eyeline/asn1/BER/tests/SeqType3.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {
namespace dec {

class MDSeqType3 : public DecoderOfSequence_T<3,1> {
public:
  explicit MDSeqType3(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
  : asn1::ber::DecoderOfSequence_T<3,1>(use_rule),
    _value(NULL)
  {
    construct();
  }

  void setValue(SeqType3& value)
  {
    _value = &value;
    _seqDec.reset();
  }

protected:
  void construct(void);
  virtual asn1::ber::TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */;

private:
  SeqType3* _value;

  DecoderOfINTEGER _dA;
  asn1::ber::DecoderProducer_T<DecoderOfINTEGER> _dB;
  asn1::ber::DecoderProducer_T<asn1::ber::DecoderOfUExtension> _uext;

  static const asn1::ASTag _tag_B;
};

}}}}}

#endif
