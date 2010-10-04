#ifndef __EYELINE_MAP_7F0_PROTO_ERR_ENC_MEDATAMISSINGPARAM_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_ERR_ENC_MEDATAMISSINGPARAM_HPP__

# include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"
# include "eyeline/asn1/BER/rtenc/EncoderProducer.hpp"
# include "eyeline/map/7_15_0/proto/ERR/DataMissingParam.hpp"
# include "eyeline/map/7_15_0/proto/EXT/enc/MEExtensionContainer.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace enc {

/*
DataMissingParam ::= SEQUENCE {
  extensionContainer      ExtensionContainer      OPTIONAL,
  ...
}
*/
class MEDataMissingParam : public asn1::ber::EncoderOfPlainSequence_T<2> {
public:
  explicit MEDataMissingParam(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfPlainSequence_T<2>(use_rule)
  {}

  explicit MEDataMissingParam(const DataMissingParam& value,
                              asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfPlainSequence_T<2>(use_rule)
  {
    setValue(value);
  }

  MEDataMissingParam(const asn1::ASTag& outer_tag,
                     const asn1::ASTagging::Environment_e tag_env,
                     asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfPlainSequence_T<2>(outer_tag, tag_env, use_rule)
  {}

  void setValue(const DataMissingParam& value) /*throw(std::exception)*/;

private:
  typedef asn1::ber::EncoderOfUExtension_T<1> MEArgUExt;
  asn1::ber::EncoderProducer_T<ext::enc::MEExtensionContainer> _extensionContainer;
  util::OptionalObj_T<MEArgUExt> _unkExt;
};

}}}}

#endif
