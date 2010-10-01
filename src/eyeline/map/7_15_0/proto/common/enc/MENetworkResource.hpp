/* ************************************************************************* *
 * NetworkResource type encoder.
 * ************************************************************************* */
#ifndef __EYELINE_MAP_7F0_PROTO_COMMON_MENETWORKRESOURCE_HPP__
#ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_COMMON_MENETWORKRESOURCE_HPP__

#include "eyeline/map/7_15_0/proto/common/NetworkResource.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeENUM.hpp"


namespace eyeline {
namespace map {
namespace common {
namespace enc {

using eyeline::asn1::ber::TSGroupBER;

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

class MENetworkResource : public asn1::ber::EncoderOfENUM {
public:
  explicit MENetworkResource(asn1::TransferSyntax::Rule_e use_rule
                             = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfENUM(use_rule)
  { }
  MENetworkResource(const asn1::ASTag& outer_tag,
                    const asn1::ASTagging::Environment_e tag_env,
                    asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfENUM(outer_tag, tag_env, use_rule)
  { }

  explicit MENetworkResource(const NetworkResource & use_val,
                             TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfENUM(TSGroupBER::getTSRule(use_rule))
  {
    setValue(use_val);
  }

  ~MENetworkResource()
  { }

  void setValue(const NetworkResource & use_val) /*throw(std::exception)*/;
};

}}}}

#endif /* __EYELINE_MAP_7F0_PROTO_COMMON_MENETWORKRESOURCE_HPP__ */

