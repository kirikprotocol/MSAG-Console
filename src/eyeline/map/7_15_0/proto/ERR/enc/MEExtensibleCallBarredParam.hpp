#ifndef __EYELINE_MAP_7F0_PROTO_ERR_ENC_MEEXTENSIBLECALLBARREDPARAM_HPP__
#ifndef __GNUC__
# ident "@(#)$Id$"
#endif
# define __EYELINE_MAP_7F0_PROTO_ERR_ENC_MEEXTENSIBLECALLBARREDPARAM_HPP__

# include "eyeline/map/7_15_0/proto/ERR/CallBarringCause.hpp"
# include "eyeline/map/7_15_0/proto/ERR/ExtensibleCallBarredParam.hpp"
# include "eyeline/map/7_15_0/proto/EXT/ExtensionContainer.hpp"

# include "eyeline/map/7_15_0/proto/ERR/enc/MECallBarringCause.hpp"
# include "eyeline/map/7_15_0/proto/EXT/enc/MEExtensionContainer.hpp"

# include "eyeline/asn1/BER/rtenc/EncodeNULL.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeSequenceExt.hpp"

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
class MEExtensibleCallBarredParam : public asn1::ber::EncoderOfExtensibleSequence_T<3> {
public:
  explicit MEExtensibleCallBarredParam(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfExtensibleSequence_T<3>(use_rule)
  {}

  explicit MEExtensibleCallBarredParam(const ExtensibleCallBarredParam& value,
                                       asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfExtensibleSequence_T<3>(use_rule)
  {
    setValue(value);
  }

  MEExtensibleCallBarredParam(const asn1::ASTag& outer_tag,
                              const asn1::ASTagging::Environment_e tag_env,
                              asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfExtensibleSequence_T<3>(outer_tag, tag_env, use_rule)
  {}

  void setValue(const ExtensibleCallBarredParam& value) /*throw(std::exception)*/;

private:
  static const asn1::ASTag _tag_unauthMsgOrig;

  asn1::ber::EncoderProducer_T<MECallBarringCause>              _eCallBarringCause;
  asn1::ber::EncoderProducer_T<ext::enc::MEExtensionContainer>  _eExtensionContainer;
  asn1::ber::EncoderProducer_T<asn1::ber::EncoderOfNULL>        _eUnauthMsgOrig;
};

}}}}

#endif
