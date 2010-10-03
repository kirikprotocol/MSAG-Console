#ifndef __EYELINE_MAP_7F0_PROTO_EXT_ENC_MEEXTENSIONCONTAINER_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_EXT_ENC_MEEXTENSIONCONTAINER_HPP__

# include "eyeline/util/OptionalObjT.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeUExt.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"
# include "eyeline/map/7_15_0/proto/EXT/ExtensionContainer.hpp"
# include "eyeline/map/7_15_0/proto/EXT/enc/MEPrivateExtensionList.hpp"
# include "eyeline/map/7_15_0/proto/EXT/enc/MEPCS_Extensions.hpp"

namespace eyeline {
namespace map {
namespace ext {
namespace enc {

/*
 ExtensionContainer ::= SEQUENCE {
    privateExtensionList    [0] PrivateExtensionList OPTIONAL,
    pcs-Extensions          [1] PCS-Extensions       OPTIONAL,
    ...
 }
*/
class MEExtensionContainer : public asn1::ber::EncoderOfSequence_T<3> {
public:
  explicit MEExtensionContainer(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfSequence_T<3>(use_rule)
  {}

  MEExtensionContainer(const ExtensionContainer& ext_container,
                       asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfSequence_T<3>(use_rule)
  {
    setValue(ext_container);
  }

  MEExtensionContainer(const asn1::ASTag& outer_tag,
                       const asn1::ASTagging::Environment_e tag_env,
                       asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfSequence_T<3>(outer_tag, tag_env, use_rule)
  {}

  void setValue(const ExtensionContainer& ext_container);

private:
  const static asn1::ASTag _tag_PrivateExtensionList;
  const static asn1::ASTag _tag_PcsExtensions;

  asn1::ber::EncoderProducer_T<MEPrivateExtensionList> _ePrivateExtensionList;
  asn1::ber::EncoderProducer_T<MEPCS_Extensions>       _ePcsExtensions;

  typedef asn1::ber::EncoderOfUExtension_T<1> MEArgUExt;
  util::OptionalObj_T<MEArgUExt>                       _eUnkExt;
};

}}}}

#endif
