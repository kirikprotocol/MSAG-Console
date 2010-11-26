#ifndef __EYELINE_MAP_7F0_PROTO_ERR_ENC_MEILLEGALSUBSCRIBERPARAM_HPP__
#ifndef __GNUC__
# ident "@(#)$Id$"
#endif
# define __EYELINE_MAP_7F0_PROTO_ERR_ENC_MEILLEGALSUBSCRIBERPARAM_HPP__

# include "eyeline/map/7_15_0/proto/ERR/IllegalSubscriberParam.hpp"
# include "eyeline/map/7_15_0/proto/EXT/enc/MEExtensionContainer.hpp"

# include "eyeline/asn1/BER/rtenc/EncodeSequenceExt.hpp"
# include "eyeline/asn1/BER/rtenc/EncoderProducer.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace enc {

/*
IllegalSubscriberParam ::= SEQUENCE {
        extensionContainer      ExtensionContainer      OPTIONAL,
        ...}
*/
class MEIllegalSubscriberParam : public asn1::ber::EncoderOfExtensibleSequence_T<1> {
public:
  explicit MEIllegalSubscriberParam(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfExtensibleSequence_T<1>(use_rule)
  {}

  explicit MEIllegalSubscriberParam(const IllegalSubscriberParam& value,
                                    asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfExtensibleSequence_T<1>(use_rule)
  {
    setValue(value);
  }

  MEIllegalSubscriberParam(const asn1::ASTag& outer_tag,
                           const asn1::ASTagging::Environment_e tag_env,
                           asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfExtensibleSequence_T<1>(outer_tag, tag_env, use_rule)
  {}

  void setValue(const IllegalSubscriberParam& value);

private:
  asn1::ber::EncoderProducer_T<ext::enc::MEExtensionContainer> _eExtensionContainer;
};

}}}}

#endif
