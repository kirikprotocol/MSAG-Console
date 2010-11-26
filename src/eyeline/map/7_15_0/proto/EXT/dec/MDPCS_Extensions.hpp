#ifndef __EYELINE_MAP_7F0_PROTO_EXT_DEC_MDPCS_EXTENSIONS_HPP__
#ifndef __GNUC__
# ident "@(#)$Id$"
#endif
# define __EYELINE_MAP_7F0_PROTO_EXT_DEC_MDPCS_EXTENSIONS_HPP__

# include "eyeline/asn1/BER/rtdec/DecodeSeq.hpp"
# include "eyeline/asn1/BER/rtdec/DecodeUExt.hpp"
# include "eyeline/asn1/BER/rtdec/DecoderProducer.hpp"

# include "eyeline/map/7_15_0/proto/EXT/PCS_Extensions.hpp"

namespace eyeline {
namespace map {
namespace ext {
namespace dec {

/*
 Type is defined in IMPLICIT tagging environment as follow:
 PCS-Extensions ::= SEQUENCE { ... }
 */
class MDPCS_Extensions : public asn1::ber::DecoderOfSequence_T<1> {
public:
  explicit MDPCS_Extensions(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
  : asn1::ber::DecoderOfSequence_T<1>(use_rule),
    _value(NULL)
  {
    construct();
  }

  explicit MDPCS_Extensions(PCS_Extensions& value,
                            asn1::ber::TSGroupBER::Rule_e use_rule = asn1::ber::TSGroupBER::ruleBER)
  : asn1::ber::DecoderOfSequence_T<1>(asn1::ber::TSGroupBER::getTSRule(use_rule)),
    _value(&value)
  {
    construct();
  }

  void setValue(PCS_Extensions& value)
  {
    _value = &value;
  }

protected:
  void construct(void);
  virtual asn1::ber::TypeDecoderAC* prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */;

private:
  PCS_Extensions* _value;
  asn1::ber::DecoderProducer_T<asn1::ber::DecoderOfUExtension> _uext;
};

}}}}

#endif
