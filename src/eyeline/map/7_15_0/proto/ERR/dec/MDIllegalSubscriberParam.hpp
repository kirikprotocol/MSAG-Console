#ifndef __EYELINE_MAP_7F0_PROTO_ERR_DEC_MDILLEGALSUBSCRIBERPARAM_HPP__
#ifndef __GNUC__
# ident "@(#)$Id$"
#endif
# define __EYELINE_MAP_7F0_PROTO_ERR_DEC_MDILLEGALSUBSCRIBERPARAM_HPP__

# include "eyeline/asn1/BER/rtdec/DecodeSeq.hpp"
# include "eyeline/asn1/BER/rtdec/DecoderProducer.hpp"
# include "eyeline/asn1/BER/rtdec/DecodeUExt.hpp"
# include "eyeline/map/7_15_0/proto/ERR/IllegalSubscriberParam.hpp"
# include "eyeline/map/7_15_0/proto/EXT/dec/MDExtensionContainer.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace dec {

/*
IllegalSubscriberParam ::= SEQUENCE {
  extensionContainer      ExtensionContainer      OPTIONAL,
  ...
}
*/
class MDIllegalSubscriberParam : public asn1::ber::DecoderOfSequence_T<2,1> {
public:
  MDIllegalSubscriberParam(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
  : asn1::ber::DecoderOfSequence_T<2,1>(use_rule),
    _value(NULL)
  {
    construct();
  }

  MDIllegalSubscriberParam(const asn1::ASTag & use_tag, asn1::ASTagging::Environment_e tag_env,
                           asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
  : asn1::ber::DecoderOfSequence_T<2,1>(use_tag, tag_env, use_rule),
    _value(NULL)
  {
    construct();
  }

  void setValue(IllegalSubscriberParam& value) {
    _value= &value;
    _seqDec.reset();
  }

protected:
  void construct();
  virtual asn1::ber::TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */;

private:
  IllegalSubscriberParam* _value;
  asn1::ber::DecoderProducer_T<ext::dec::MDExtensionContainer> _dExtContainer;
  asn1::ber::DecoderProducer_T<asn1::ber::DecoderOfUExtension>   _uext;
};

}}}}

#endif
