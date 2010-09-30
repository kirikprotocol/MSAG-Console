#ifndef __EYELINE_MAP_7F0_PROTO_COMMON_MENETWORKRESOURCE_HPP__
# define __EYELINE_MAP_7F0_PROTO_COMMON_MENETWORKRESOURCE_HPP__

# include "eyeline/asn1/BER/rtenc/EncodeENUM.hpp"

namespace eyeline {
namespace map {
namespace common {
namespace enc {

/*
NetworkResource ::= ENUMERATED {
        plmn  (0),
        hlr  (1),
        vlr  (2),
        pvlr  (3),
        controllingMSC  (4),
        vmsc  (5),
        eir  (6),
        rss  (7)}
*/

class MENetworkResource : public asn1::ber::EncoderOfENUM {
public:
  explicit MENetworkResource(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
  : asn1::ber::EncoderOfENUM(use_rule)
  {}

  explicit MENetworkResource(const asn1::ASTagging & eff_tags,
                             asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
  : asn1::ber::EncoderOfENUM(eff_tags, use_rule)
  {}

  MENetworkResource(const asn1::ASTag& outer_tag,
                    const asn1::ASTagging::Environment_e tag_env,
                    asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfENUM(outer_tag, tag_env, use_rule)
  {}
};

}}}}

#endif
