/* ************************************************************************* *
 * NetworkResource type encoder.
 * ************************************************************************* */
#ifndef __EYELINE_MAP_7F0_PROTO_COMMON_MEADDITIONALNETWORKRESOURCE_HPP__
#ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_COMMON_MEADDITIONALNETWORKRESOURCE_HPP__

#include "eyeline/map/7_15_0/proto/common/AdditionalNetworkResource.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeENUM.hpp"

namespace eyeline {
namespace map {
namespace common {
namespace enc {

using eyeline::asn1::ber::TSGroupBER;

/* Type is defined in IMPLICIT tagging environment as follow:
AdditionalNetworkResource ::= ENUMERATED {
        sgsn (0),
        ggsn (1),
        gmlc (2),
        gsmSCF (3),
        nplr (4),
        auc (5),
        ... ,
        ue (6)
} */
class MEAdditionalNetworkResource : public asn1::ber::EncoderOfENUM {
public:
  explicit MEAdditionalNetworkResource(asn1::TransferSyntax::Rule_e use_rule
                                        = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfENUM(use_rule)
  { }
  MEAdditionalNetworkResource(const asn1::ASTag & outer_tag,
                              const asn1::ASTagging::Environment_e tag_env,
                              asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfENUM(outer_tag, tag_env, use_rule)
  {}
  //
  explicit MEAdditionalNetworkResource(const AdditionalNetworkResource & use_val,
                                       TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfENUM(TSGroupBER::getTSRule(use_rule))
  {
    setValue(use_val);
  }
  ~MEAdditionalNetworkResource()
  { }

  void setValue(const AdditionalNetworkResource & use_val) /*throw(std::exception)*/;

};

}}}}

#endif /* __EYELINE_MAP_7F0_PROTO_COMMON_MEADDITIONALNETWORKRESOURCE_HPP__ */

