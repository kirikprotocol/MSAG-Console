#ifndef __EYELINE_MAP_7F0_PROTO_ERR_ENC_MEDATAMISSINGPARAM_HPP__
#ifndef __GNUC__
# ident "@(#)$Id$"
#endif
# define __EYELINE_MAP_7F0_PROTO_ERR_ENC_MEDATAMISSINGPARAM_HPP__

# include "eyeline/map/7_15_0/proto/ERR/DataMissingParam.hpp"

# include "eyeline/map/7_15_0/proto/EXT/enc/MEExtensionContainer.hpp"

# include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"
# include "eyeline/asn1/BER/rtenc/EncoderProducer.hpp"

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
class MEDataMissingParam : public asn1::ber::EncoderOfExtensibleSequence_T<1> {
public:
  explicit MEDataMissingParam(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfExtensibleSequence_T<1>(use_rule)
  {}

  explicit MEDataMissingParam(const DataMissingParam& value,
                              asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfExtensibleSequence_T<1>(use_rule)
  {
    setValue(value);
  }

  MEDataMissingParam(const asn1::ASTag& outer_tag,
                     const asn1::ASTagging::Environment_e tag_env,
                     asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfExtensibleSequence_T<1>(outer_tag, tag_env, use_rule)
  {}

  void setValue(const DataMissingParam& value) /*throw(std::exception)*/;

private:
  asn1::ber::EncoderProducer_T<ext::enc::MEExtensionContainer> _extensionContainer;
};

}}}}

#endif
