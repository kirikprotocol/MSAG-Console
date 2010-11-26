#ifndef __EYELINE_MAP_7F0_PROTO_ERR_DEC_MDCALLBARRINGCAUSE_HPP__
#ifndef __GNUC__
# ident "@(#)$Id$"
#endif
# define __EYELINE_MAP_7F0_PROTO_ERR_DEC_MDCALLBARRINGCAUSE_HPP__

# include "eyeline/map/7_15_0/proto/ERR/CallBarringCause.hpp"
# include "eyeline/asn1/BER/rtdec/DecodeENUM.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace dec {

/*
 CallBarringCause ::= ENUMERATED {
        barringServiceActive  (0),
        operatorBarring  (1)}
*/
class MDCallBarringCause : public asn1::ber::DecoderOfENUM {
public:
  explicit MDCallBarringCause(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::DecoderOfENUM(use_rule)
  {}

  explicit MDCallBarringCause(CallBarringCause & use_val,
                              asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::DecoderOfENUM(use_rule)
  {
    setValue(use_val);
  }

  MDCallBarringCause(const asn1::ASTag& outer_tag,
                      const asn1::ASTagging::Environment_e tag_env,
                      asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::DecoderOfENUM(outer_tag, tag_env, use_rule)
  {}

  void setValue(CallBarringCause & value) /*throw(std::exception)*/;
};

}}}}

#endif
