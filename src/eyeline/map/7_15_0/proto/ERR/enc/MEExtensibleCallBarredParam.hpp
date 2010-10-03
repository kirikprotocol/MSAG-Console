#ifndef __EYELINE_MAP_7F0_PROTO_ERR_ENC_MEEXTENSIBLECALLBARREDPARAM_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_ERR_ENC_MEEXTENSIBLECALLBARREDPARAM_HPP__

# include "util/Exception.hpp"

# include "eyeline/asn1/BER/rtenc/EncodeNULL.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeUExt.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"
# include "eyeline/map/7_15_0/proto/ERR/CallBarringCause.hpp"
# include "eyeline/map/7_15_0/proto/ERR/ExtensibleCallBarredParam.hpp"
# include "eyeline/map/7_15_0/proto/ERR/enc/MECallBarringCause.hpp"
# include "eyeline/map/7_15_0/proto/EXT/ExtensionContainer.hpp"
# include "eyeline/map/7_15_0/proto/EXT/enc/MEExtensionContainer.hpp"

# include "eyeline/util/MaxSizeof.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace enc {

/*
 ExtensibleCallBarredParam ::= SEQUENCE {
        callBarringCause        CallBarringCause        OPTIONAL,
        extensionContainer      ExtensionContainer      OPTIONAL,
        ... ,
        unauthorisedMessageOriginator   [1] NULL                OPTIONAL }
*/
class MEExtensibleCallBarredParam : public asn1::ber::EncoderOfSequence_T<4> {
public:
  explicit MEExtensibleCallBarredParam(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfSequence_T<4>(use_rule)
  {}

  explicit MEExtensibleCallBarredParam(const ExtensibleCallBarredParam& value,
                                       asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfSequence_T<4>(use_rule)
  {
    setValue(value);
  }

  MEExtensibleCallBarredParam(const asn1::ASTag& outer_tag,
                              const asn1::ASTagging::Environment_e tag_env,
                              asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfSequence_T<4>(outer_tag, tag_env, use_rule)
  {}

  void setValue(const ExtensibleCallBarredParam& value);

private:
  static const asn1::ASTag _tag_unauthMsgOrig;/*= asn1::ASTag(asn1::ASTag::tagContextSpecific, 1);*/

  asn1::ber::EncoderProducer_T<MECallBarringCause> _eCallBarringCause;
  asn1::ber::EncoderProducer_T<ext::enc::MEExtensionContainer> _eExtensionContainer;
  typedef asn1::ber::EncoderOfUExtension_T<1> MEArgUExt;
  util::OptionalObj_T<MEArgUExt> _eUnkExt;
  asn1::ber::EncoderProducer_T<asn1::ber::EncoderOfNULL> _eUnauthMsgOrig;
};

}}}}

#endif
