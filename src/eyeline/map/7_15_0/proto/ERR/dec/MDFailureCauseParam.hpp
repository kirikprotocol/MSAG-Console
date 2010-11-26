#ifndef __EYELINE_MAP_7F0_PROTO_ERR_DEC_MDFAILURECAUSEPARAM_HPP__
#ifndef __GNUC__
# ident "@(#)$Id$"
#endif
# define __EYELINE_MAP_7F0_PROTO_ERR_DEC_MDFAILURECAUSEPARAM_HPP__

# include "eyeline/asn1/BER/rtdec/DecodeENUM.hpp"
# include "eyeline/map/7_15_0/proto/ERR/FailureCauseParam.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace dec {

/*
 FailureCauseParam ::= ENUMERATED {
        limitReachedOnNumberOfConcurrentLocationRequests (0),
        ... }
*/
class MDFailureCauseParam : public asn1::ber::DecoderOfENUM {
public:
  explicit MDFailureCauseParam(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::DecoderOfENUM(use_rule)
  {}

  MDFailureCauseParam(const asn1::ASTag& outer_tag,
                      const asn1::ASTagging::Environment_e tag_env,
                      asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::DecoderOfENUM(outer_tag, tag_env, use_rule)
  {}

  void setValue(FailureCauseParam & value) /*throw(std::exception)*/;
};

}}}}

#endif
