#ifndef __EYELINE_MAP_7F0_PROTO_ERR_DEC_MDABSENTSUBSCRIBERPARAM_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_ERR_DEC_MDABSENTSUBSCRIBERPARAM_HPP__

# include "eyeline/asn1/BER/rtdec/DecodeSeq.hpp"
# include "eyeline/asn1/BER/rtdec/DecodeUExt.hpp"
# include "eyeline/asn1/BER/rtdec/DecoderProducer.hpp"

# include "eyeline/map/7_15_0/proto/ERR/AbsentSubscriberParam.hpp"

# include "eyeline/map/7_15_0/proto/ERR/dec/MDAbsentSubscriberReason.hpp"
# include "eyeline/map/7_15_0/proto/EXT/dec/MDExtensionContainer.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace dec {

/*
AbsentSubscriberParam ::= SEQUENCE {
        extensionContainer      ExtensionContainer      OPTIONAL,
        ...,
        absentSubscriberReason  [0] AbsentSubscriberReason      OPTIONAL
}
*/
class MDAbsentSubscriberParam : public asn1::ber::DecoderOfSequence_T<3,1> {
public:
  explicit MDAbsentSubscriberParam(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
  : asn1::ber::DecoderOfSequence_T<3,1>(use_rule),
    _value(NULL)
  {
    construct();
  }

  explicit MDAbsentSubscriberParam(AbsentSubscriberParam& value,
                                   asn1::ber::TSGroupBER::Rule_e use_rule = asn1::ber::TSGroupBER::ruleBER)
  : asn1::ber::DecoderOfSequence_T<3,1>(asn1::ber::TSGroupBER::getTSRule(use_rule)),
    _value(&value)
  {
    construct();
  }

  void setValue(AbsentSubscriberParam& value)
  {
    _value = &value;
  }

protected:
  void construct();
  virtual asn1::ber::TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */;

private:
  static const asn1::ASTag _tag_absentSubscriberReason;

  AbsentSubscriberParam* _value;
  asn1::ber::DecoderProducer_T<ext::dec::MDExtensionContainer> _dExtContainer;
  asn1::ber::DecoderProducer_T<MDAbsentSubscriberReason> _dAbsentSubscriberReason;
  asn1::ber::DecoderProducer_T<asn1::ber::DecoderOfUExtension>   _uext;
};

}}}}

#endif
