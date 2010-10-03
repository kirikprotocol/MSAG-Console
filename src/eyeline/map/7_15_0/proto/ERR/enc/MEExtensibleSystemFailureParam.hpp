#ifndef __EYELINE_MAP_7F0_PROTO_ERR_ENC_MEEXTENSIBLESYSTEMFAILUREPARAM_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_ERR_ENC_MEEXTENSIBLESYSTEMFAILUREPARAM_HPP__

# include "util/Exception.hpp"

# include "eyeline/asn1/BER/rtenc/EncodeUExt.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"
# include "eyeline/map/7_15_0/proto/common/AdditionalNetworkResource.hpp"
# include "eyeline/map/7_15_0/proto/common/NetworkResource.hpp"
# include "eyeline/map/7_15_0/proto/EXT/ExtensionContainer.hpp"
# include "eyeline/map/7_15_0/proto/ERR/FailureCauseParam.hpp"

# include "eyeline/map/7_15_0/proto/common/enc/MEAdditionalNetworkResource.hpp"
# include "eyeline/map/7_15_0/proto/common/enc/MENetworkResource.hpp"
# include "eyeline/map/7_15_0/proto/ERR/enc/MEFailureCauseParam.hpp"
# include "eyeline/map/7_15_0/proto/EXT/enc/MEExtensionContainer.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace enc {

/*
ExtensibleSystemFailureParam ::= SEQUENCE {
        networkResource NetworkResource OPTIONAL,
        extensionContainer      ExtensionContainer      OPTIONAL,
        ...,
        additionalNetworkResource       [0] AdditionalNetworkResource   OPTIONAL,
        failureCauseParam       [1] FailureCauseParam   OPTIONAL }
*/

class MEExtensibleSystemFailureParam : public asn1::ber::EncoderOfSequence_T<5> {
public:
  explicit MEExtensibleSystemFailureParam(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfSequence_T<5>(use_rule)
  {}

  explicit MEExtensibleSystemFailureParam(const ExtensibleSystemFailureParam& value,
                                          asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfSequence_T<5>(use_rule)
  {
    setValue(value);
  }

  MEExtensibleSystemFailureParam(const asn1::ASTag& outer_tag,
                                 const asn1::ASTagging::Environment_e tag_env,
                                 asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfSequence_T<5>(outer_tag, tag_env, use_rule)
  {}

  void setValue(const ExtensibleSystemFailureParam& value);

private:
  static const asn1::ASTag _tag_additionalNetworkResource;
  static const asn1::ASTag _tag_failureCauseParam;

  asn1::ber::EncoderProducer_T<common::enc::MENetworkResource> _eNetworkResource;
  asn1::ber::EncoderProducer_T<ext::enc::MEExtensionContainer> _eExtensionContainer;
  typedef asn1::ber::EncoderOfUExtension_T<1> MEArgUExt;
  util::OptionalObj_T<MEArgUExt> _eUnkExt;
  asn1::ber::EncoderProducer_T<common::enc::MEAdditionalNetworkResource> _eAdditionalNetworkResource;
  asn1::ber::EncoderProducer_T<MEFailureCauseParam> _eFailureCauseParam;
};

}}}}

#endif
