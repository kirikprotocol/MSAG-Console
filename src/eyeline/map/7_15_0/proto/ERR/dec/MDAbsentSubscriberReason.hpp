#ifndef __EYELINE_MAP_7F0_PROTO_ERR_DEC_MDABSENTSUBSCRIBERREASON_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_ERR_DEC_MDABSENTSUBSCRIBERREASON_HPP__

# include "eyeline/map/7_15_0/proto/ERR/AbsentSubscriberReason.hpp"
# include "eyeline/asn1/BER/rtdec/DecodeENUM.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace dec {

/*
 AbsentSubscriberReason ::= ENUMERATED {
        imsiDetach (0),
        restrictedArea (1),
        noPageResponse (2),
        ... ,
        purgedMS (3)}
*/
class MDAbsentSubscriberReason : public asn1::ber::DecoderOfENUM {
public:
  explicit MDAbsentSubscriberReason(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::DecoderOfENUM(use_rule)
  {}

  explicit MDAbsentSubscriberReason(AbsentSubscriberReason & use_val,
                                    asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::DecoderOfENUM(use_rule)
  {
    setValue(use_val);
  }

  MDAbsentSubscriberReason(const asn1::ASTag& outer_tag,
                           const asn1::ASTagging::Environment_e tag_env,
                           asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::DecoderOfENUM(outer_tag, tag_env, use_rule)
  {}

  void setValue(AbsentSubscriberReason & value) /*throw(std::exception)*/;
};

}}}}

#endif
