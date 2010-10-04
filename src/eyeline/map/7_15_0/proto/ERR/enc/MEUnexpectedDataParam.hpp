#ifndef __EYELINE_MAP_7F0_PROTO_ERR_ENC_MEUNEXPECTEDDATAPARAM_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_ERR_ENC_MEUNEXPECTEDDATAPARAM_HPP__

# include "eyeline/asn1/BER/rtenc/EncodeUExt.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"
# include "eyeline/asn1/BER/rtenc/EncoderProducer.hpp"
# include "eyeline/map/7_15_0/proto/ERR/DataMissingParam.hpp"
# include "eyeline/map/7_15_0/proto/EXT/enc/MEExtensionContainer.hpp"
# include "eyeline/util/OptionalObjT.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace enc {

/*
UnexpectedDataParam ::= SEQUENCE {
  extensionContainer      ExtensionContainer      OPTIONAL,
  ...
}
*/
class MEUnexpectedDataParam : public asn1::ber::EncoderOfPlainSequence_T<2> {
public:
  explicit MEUnexpectedDataParam(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfPlainSequence_T<2>(use_rule)
  {}

  explicit MEUnexpectedDataParam(const DataMissingParam& value,
                                 asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfPlainSequence_T<2>(use_rule)
  {
    setValue(value);
  }

  MEUnexpectedDataParam(const asn1::ASTag& outer_tag,
                        const asn1::ASTagging::Environment_e tag_env,
                        asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfPlainSequence_T<2>(outer_tag, tag_env, use_rule)
  {}

  void setValue(const DataMissingParam& value);

private:
  asn1::ber::EncoderProducer_T<ext::enc::MEExtensionContainer> _eExtensionContainer;
  typedef asn1::ber::EncoderOfUExtension_T<1> MEArgUExt;
  util::OptionalObj_T<MEArgUExt> _eUnkExt;
};

}}}}

#endif
