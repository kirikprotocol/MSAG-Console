#ifndef __EYELINE_ASN1_BER_TESTS_DEC_MDSEQTYPE4_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
# define __EYELINE_ASN1_BER_TESTS_DEC_MDSEQTYPE4_HPP__

# include "eyeline/asn1/BER/rtdec/DecodeINT.hpp"
# include "eyeline/asn1/BER/rtdec/DecodeSeq.hpp"
# include "eyeline/asn1/BER/rtdec/DecoderProducer.hpp"
# include "eyeline/asn1/BER/rtdec/DecodeUExt.hpp"
# include "eyeline/asn1/BER/tests/SeqType4.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {
namespace tests {
namespace dec {

class MDSeqType4 : public DecoderOfSequence_T<4,1> {
public:
  explicit MDSeqType4(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
  : asn1::ber::DecoderOfSequence_T<4,1>(use_rule),
    _value(NULL)
  {
    construct();
  }

  void setValue(SeqType4 & value) { _value = &value; }

protected:
  void construct(void);
  // ----------------------------------------
  // -- DecoderOfStructAC interface methods
  // ----------------------------------------
  //If necessary, allocates optional element and initializes associated TypeDecoderAC
  virtual asn1::ber::TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */;

private:
  SeqType4* _value;

  DecoderOfINTEGER _dA;
  asn1::ber::DecoderProducer_T<DecoderOfINTEGER> _dB;
  asn1::ber::DecoderProducer_T<DecoderOfINTEGER> _dC;
  asn1::ber::DecoderProducer_T<asn1::ber::DecoderOfUExtension> _uext;

  static const asn1::ASTag _tag_B;
  static const asn1::ASTag _tag_C;
};

}}}}}

#endif
