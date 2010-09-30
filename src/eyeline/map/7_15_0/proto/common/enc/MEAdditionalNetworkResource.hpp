#ifndef __EYELINE_MAP_7F0_PROTO_COMMON_MEADDITIONALNETWORKRESOURCE_HPP__
# define __EYELINE_MAP_7F0_PROTO_COMMON_MEADDITIONALNETWORKRESOURCE_HPP__

# include "eyeline/asn1/BER/rtenc/EncodeENUM.hpp"

namespace eyeline {
namespace map {
namespace common {
namespace enc {

/*
AdditionalNetworkResource ::= ENUMERATED {
        sgsn (0),
        ggsn (1),
        gmlc (2),
        gsmSCF (3),
        nplr (4),
        auc (5),
        ... ,
        ue (6)}
*/
class MEAdditionalNetworkResource : public asn1::ber::EncoderOfENUM {
public:
  explicit MEAdditionalNetworkResource(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
  : asn1::ber::EncoderOfENUM(use_rule)
  {}

  explicit MEAdditionalNetworkResource(const asn1::ASTagging & eff_tags,
                                       asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
  : asn1::ber::EncoderOfENUM(eff_tags, use_rule)
  {}

  MEAdditionalNetworkResource(const asn1::ASTag& outer_tag,
                              const asn1::ASTagging::Environment_e tag_env,
                              asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfENUM(outer_tag, tag_env, use_rule)
  {}
};

}}}}

#endif
