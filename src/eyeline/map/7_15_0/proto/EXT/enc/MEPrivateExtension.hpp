#ifndef __EYELINE_MAP_7F0_PROTO_EXT_ENC_MEPRIVATEEXTENSION_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_EXT_ENC_MEPRIVATEEXTENSION_HPP__

# include "eyeline/asn1/BER/rtenc/EncodeASType.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeEOID.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"
# include "eyeline/asn1/BER/rtenc/EncoderProducer.hpp"
# include "eyeline/map/7_15_0/proto/EXT/PrivateExtension.hpp"

namespace eyeline {
namespace map {
namespace ext {
namespace enc {

/* Type is defined in IMPLICIT tagging environment as follow:
  PrivateExtension ::= SEQUENCE {
    extId       MAP-EXTENSION.&extensionId ({ExtensionSet}),
    extType     MAP-EXTENSION.&ExtensionType ({ExtensionSet}{@extId}) OPTIONAL
  }
*/

class MEPrivateExtension : public asn1::ber::EncoderOfPlainSequence_T<2> {
public:
  explicit MEPrivateExtension(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfPlainSequence_T<2>(use_rule)
  {
    construct();
  }

  MEPrivateExtension(const PrivateExtension& private_ext, asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfPlainSequence_T<2>(use_rule)
  {
    construct();
    setValue(private_ext);
  }

  MEPrivateExtension(const asn1::ASTag & use_tag, asn1::ASTagging::Environment_e tag_env,
                     asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfPlainSequence_T<2>(use_tag, tag_env, use_rule)
  {
    construct();
  }

  void setValue(const PrivateExtension& private_ext);

private:
  void construct() {
    setField(0, _eExtId);
  }

  asn1::ber::EncoderOfEOID _eExtId;
  asn1::ber::EncoderProducer_T<asn1::ber::EncoderOfASType> _eExtType;
};

}}}}

#endif
