#ifndef __EYELINE_MAP_7F0_PROTO_ERR_DEC_MDEXTENSIBLECALLBARREDPARAM_HPP__
#ifndef __GNUC__
# ident "@(#)$Id$"
#endif
# define __EYELINE_MAP_7F0_PROTO_ERR_DEC_MDEXTENSIBLECALLBARREDPARAM_HPP__

# include "eyeline/asn1/BER/rtdec/DecodeNULL.hpp"
# include "eyeline/asn1/BER/rtdec/DecodeSeq.hpp"
# include "eyeline/asn1/BER/rtdec/DecodeUExt.hpp"
# include "eyeline/map/7_15_0/proto/ERR/ExtensibleCallBarredParam.hpp"
# include "eyeline/map/7_15_0/proto/ERR/dec/MDCallBarringCause.hpp"
# include "eyeline/map/7_15_0/proto/EXT/dec/MDPCS_Extensions.hpp"
# include "eyeline/map/7_15_0/proto/EXT/dec/MDExtensionContainer.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace dec {

/*
 ExtensibleCallBarredParam ::= SEQUENCE {
        callBarringCause        CallBarringCause        OPTIONAL,
        extensionContainer      ExtensionContainer      OPTIONAL,
        ... ,
        unauthorisedMessageOriginator   [1] NULL                OPTIONAL }
*/
class MDExtensibleCallBarredParam : public asn1::ber::DecoderOfSequence_T<4,1> {
public:
  explicit MDExtensibleCallBarredParam(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
  : asn1::ber::DecoderOfSequence_T<4,1>(use_rule),
    _value(NULL)
  {
    construct();
  }

  explicit MDExtensibleCallBarredParam(ExtensibleCallBarredParam& value,
                                       asn1::ber::TSGroupBER::Rule_e use_rule = asn1::ber::TSGroupBER::ruleBER)
  : asn1::ber::DecoderOfSequence_T<4,1>(asn1::ber::TSGroupBER::getTSRule(use_rule)),
    _value(&value)
  {
    construct();
  }

  void setValue(ExtensibleCallBarredParam& value)
  {
    _value = &value;
    _seqDec.reset();
  }

protected:
  void construct();
  virtual asn1::ber::TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */;

private:
  ExtensibleCallBarredParam* _value;

  asn1::ber::DecoderProducer_T<MDCallBarringCause> _dCallBarringCause;
  asn1::ber::DecoderProducer_T<ext::dec::MDExtensionContainer> _dExtCont;
  asn1::ber::DecoderProducer_T<asn1::ber::DecoderOfNULL> _dUnauthMsgOrig;
  asn1::ber::DecoderProducer_T<asn1::ber::DecoderOfUExtension> _uext;

  static const asn1::ASTag _tag_UnauthMsgOrig;
};

}}}}

#endif
