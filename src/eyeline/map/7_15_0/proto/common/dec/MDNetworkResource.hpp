#ifndef __EYELINE_MAP_7F0_PROTO_COMMON_DEC_MDNETWORKRESOURCE_HPP__
#ifndef __GNUC__
# ident "@(#)$Id$"
#endif
# define __EYELINE_MAP_7F0_PROTO_COMMON_DEC_MDNETWORKRESOURCE_HPP__

# include "eyeline/map/7_15_0/proto/common/NetworkResource.hpp"
# include "eyeline/asn1/BER/rtdec/DecodeENUM.hpp"

namespace eyeline {
namespace map {
namespace common {
namespace dec {

/* Type is defined in IMPLICIT tagging environment as follow:
NetworkResource ::= ENUMERATED {
        plmn  (0),
        hlr  (1),
        vlr  (2),
        pvlr  (3),
        controllingMSC  (4),
        vmsc  (5),
        eir  (6),
        rss  (7)
}
*/
class MDNetworkResource : public asn1::ber::DecoderOfENUM {
public:
  explicit MDNetworkResource(asn1::TransferSyntax::Rule_e use_rule
                             = asn1::TransferSyntax::ruleDER)
    : asn1::ber::DecoderOfENUM(use_rule)
  { }
  MDNetworkResource(const asn1::ASTag& outer_tag,
                    const asn1::ASTagging::Environment_e tag_env,
                    asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::DecoderOfENUM(outer_tag, tag_env, use_rule)
  { }

  void setValue(NetworkResource & use_val) /*throw(std::exception)*/;
};

}}}}

#endif
