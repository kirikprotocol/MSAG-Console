#ifndef __EYELINE_ASN1_BER_TESTS_DEC_MDSEQTYPE2_HPP__
# define __EYELINE_ASN1_BER_TESTS_DEC_MDSEQTYPE2_HPP__

# include "eyeline/asn1/BER/rtdec/DecodeINT.hpp"
# include "eyeline/asn1/BER/rtdec/DecodeNULL.hpp"
# include "eyeline/asn1/BER/rtdec/DecodeSeq.hpp"
# include "eyeline/asn1/BER/rtdec/DecoderProducer.hpp"
# include "eyeline/asn1/BER/tests/SeqType2.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {
namespace dec {

class MDSeqType2 : public DecoderOfSequence_T<2,1> {
public:
  explicit MDSeqType2(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
  : asn1::ber::DecoderOfSequence_T<2,1>(use_rule),
    _value(NULL)
  {
    construct();
  }

  void setValue(SeqType2& value)
  {
    _value = &value;
  }

protected:
  void construct(void);
  virtual asn1::ber::TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */;

private:
  SeqType2* _value;

  DecoderOfINTEGER _dA;
  asn1::ber::DecoderProducer_T<DecoderOfINTEGER> _dB;

  static const asn1::ASTag _tag_B;
};

}}}}}

#endif
