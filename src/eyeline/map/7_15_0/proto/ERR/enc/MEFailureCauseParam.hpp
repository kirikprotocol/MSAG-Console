#ifndef __EYELINE_MAP_7F0_PROTO_ERR_ENC_MEFAILURECAUSEPARAM_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_ERR_ENC_MEFAILURECAUSEPARAM_HPP__

# include "eyeline/asn1/BER/rtenc/EncodeENUM.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace enc {

/*
 FailureCauseParam ::= ENUMERATED {
        limitReachedOnNumberOfConcurrentLocationRequests (0),
        ... }
*/
class MEFailureCauseParam : public asn1::ber::EncoderOfENUM {
public:
  explicit MEFailureCauseParam(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
  : asn1::ber::EncoderOfENUM(use_rule)
  {}

  explicit MEFailureCauseParam(const asn1::ASTagging & eff_tags,
                               asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
  : asn1::ber::EncoderOfENUM(eff_tags, use_rule)
  {}

  MEFailureCauseParam(const asn1::ASTag& outer_tag,
                      const asn1::ASTagging::Environment_e tag_env,
                      asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfENUM(outer_tag, tag_env, use_rule)
  {}
};

}}}}

#endif
