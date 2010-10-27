#ifndef __EYELINE_MAP_7F0_PROTO_ERR_DEC_MDEXTENSIBLESYSTEMFAILUREPARAM_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_ERR_DEC_MDEXTENSIBLESYSTEMFAILUREPARAM_HPP__

# include "eyeline/asn1/BER/rtdec/DecodeSeq.hpp"
# include "eyeline/asn1/BER/rtdec/DecodeUExt.hpp"
# include "eyeline/asn1/BER/rtdec/DecoderProducer.hpp"

# include "eyeline/map/7_15_0/proto/ERR/ExtensibleSystemFailureParam.hpp"

# include "eyeline/map/7_15_0/proto/common/dec/MDAdditionalNetworkResource.hpp"
# include "eyeline/map/7_15_0/proto/common/dec/MDNetworkResource.hpp"
# include "eyeline/map/7_15_0/proto/ERR/dec/MDFailureCauseParam.hpp"
# include "eyeline/map/7_15_0/proto/EXT/dec/MDExtensionContainer.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace dec {

/*
ExtensibleSystemFailureParam ::= SEQUENCE {
        networkResource NetworkResource OPTIONAL,
        extensionContainer      ExtensionContainer      OPTIONAL,
        ...,
        additionalNetworkResource       [0] AdditionalNetworkResource   OPTIONAL,
        failureCauseParam       [1] FailureCauseParam   OPTIONAL }
*/
class MDExtensibleSystemFailureParam : public asn1::ber::DecoderOfSequence_T<5,1> {
public:
  explicit MDExtensibleSystemFailureParam(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
  : asn1::ber::DecoderOfSequence_T<5,1>(use_rule),
    _value(NULL)
  {
    construct();
  }

  explicit MDExtensibleSystemFailureParam(ExtensibleSystemFailureParam& value,
                                          asn1::ber::TSGroupBER::Rule_e use_rule = asn1::ber::TSGroupBER::ruleBER)
  : asn1::ber::DecoderOfSequence_T<5,1>(asn1::ber::TSGroupBER::getTSRule(use_rule)),
    _value(&value)
  {
    construct();
  }

  void setValue(ExtensibleSystemFailureParam& value)
  {
    _value = &value;
  }

protected:
  void construct();
  virtual asn1::ber::TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */;

private:
  static const asn1::ASTag _tag_additionalNetworkResource;
  static const asn1::ASTag _tag_failureCauseParam;

  ExtensibleSystemFailureParam* _value;
  asn1::ber::DecoderProducer_T<common::dec::MDNetworkResource> _dNetworkResource;
  asn1::ber::DecoderProducer_T<ext::dec::MDExtensionContainer> _dExtContainer;
  asn1::ber::DecoderProducer_T<common::dec::MDAdditionalNetworkResource> _dAdditionalNetworkResource;
  asn1::ber::DecoderProducer_T<MDFailureCauseParam> _dFailureCauseParam;
  asn1::ber::DecoderProducer_T<asn1::ber::DecoderOfUExtension>   _uext;
};

}}}}

#endif
