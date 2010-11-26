#ifndef __EYELINE_MAP_7F0_PROTO_ERR_ENC_MEABSENTSUBSCRIBERREASON_HPP__
#ifndef __GNUC__
# ident "@(#)$Id$"
#endif
# define __EYELINE_MAP_7F0_PROTO_ERR_ENC_MEABSENTSUBSCRIBERREASON_HPP__

# include "eyeline/map/7_15_0/proto/ERR/AbsentSubscriberReason.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeENUM.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace enc {

/*
AbsentSubscriberReason ::= ENUMERATED {
        imsiDetach (0),
        restrictedArea (1),
        noPageResponse (2),
        ... ,
        purgedMS (3)}
*/
class MEAbsentSubscriberReason : public asn1::ber::EncoderOfENUM {
public:
  explicit MEAbsentSubscriberReason(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfENUM(use_rule)
  {}

  explicit MEAbsentSubscriberReason(const AbsentSubscriberReason & use_val,
                                    asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfENUM(use_rule)
  {
    setValue(use_val);
  }

  MEAbsentSubscriberReason(const asn1::ASTag& outer_tag,
                           const asn1::ASTagging::Environment_e tag_env,
                           asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfENUM(outer_tag, tag_env, use_rule)
  {}

  void setValue(const AbsentSubscriberReason & use_val) /*throw(std::exception)*/;
};

}}}}

#endif
