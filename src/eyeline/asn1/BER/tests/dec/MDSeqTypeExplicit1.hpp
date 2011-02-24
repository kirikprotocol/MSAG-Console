#ifndef __EYELINE_ASN1_BER_TESTS_DEC_MDSEQTYPEEXPLICIT1_HPP__
# define __EYELINE_ASN1_BER_TESTS_DEC_MDSEQTYPEEXPLICIT1_HPP__

# include "eyeline/asn1/BER/rtdec/DecodeINT.hpp"
# include "eyeline/asn1/BER/rtdec/DecodeNULL.hpp"
# include "eyeline/asn1/BER/rtdec/DecodeSeq.hpp"
# include "eyeline/asn1/BER/tests/SeqType1.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {
namespace dec {

class MDSeqTypeExplicit1 : public DecoderOfSequence_T<3> {
public:
  explicit MDSeqTypeExplicit1(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
  : asn1::ber::DecoderOfSequence_T<3>(use_rule),
    _value(NULL)
  {
    construct();
  }

  void setValue(SeqType1& value)
  {
    _value = &value;
    _seqDec.reset();
  }

protected:
  void construct(void);
  virtual asn1::ber::TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */;

private:
  SeqType1* _value;

  DecoderOfINTEGER _dA;
  DecoderOfINTEGER _dB;
  DecoderOfNULL _dC;

  static const asn1::ASTag _tag_B;
  static const asn1::ASTag _tag_C;
};

}}}}}

#endif
