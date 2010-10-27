#ifndef __EYELINE_MAP_7F0_PROTO_ERR_ENC_MEABSENTSUBSCRIBERPARAM_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_ERR_ENC_MEABSENTSUBSCRIBERPARAM_HPP__

# include "eyeline/map/7_15_0/proto/ERR/AbsentSubscriberParam.hpp"

# include "eyeline/map/7_15_0/proto/ERR/enc/MEAbsentSubscriberReason.hpp"
# include "eyeline/map/7_15_0/proto/EXT/enc/MEExtensionContainer.hpp"

# include "eyeline/asn1/BER/rtenc/EncodeNULL.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeSequenceExt.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace enc {

/*
AbsentSubscriberParam ::= SEQUENCE {
        extensionContainer      ExtensionContainer      OPTIONAL,
        ...,
        absentSubscriberReason  [0] AbsentSubscriberReason      OPTIONAL
 }
*/
class MEAbsentSubscriberParam : public asn1::ber::EncoderOfExtensibleSequence_T<2> {
public:
  explicit MEAbsentSubscriberParam(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfExtensibleSequence_T<2>(use_rule)
  {}

  explicit MEAbsentSubscriberParam(const AbsentSubscriberParam& value,
                                   asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfExtensibleSequence_T<2>(use_rule)
  {
    setValue(value);
  }

  MEAbsentSubscriberParam(const asn1::ASTag& outer_tag,
                          const asn1::ASTagging::Environment_e tag_env,
                          asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfExtensibleSequence_T<2>(outer_tag, tag_env, use_rule)
  {}

  void setValue(const AbsentSubscriberParam& value) /*throw(std::exception)*/;

private:
  static const asn1::ASTag _tag_absentSubscriberReason;

  asn1::ber::EncoderProducer_T<ext::enc::MEExtensionContainer>  _eExtensionContainer;
  asn1::ber::EncoderProducer_T<MEAbsentSubscriberReason>        _eAbsentSubscirberReason;
};

}}}}

#endif
