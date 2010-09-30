#ifndef __EYELINE_MAP_7F0_PROTO_EXT_ENC_MEPCS_EXTENSIONS_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_EXT_ENC_MEPCS_EXTENSIONS_HPP__

# include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"

namespace eyeline {
namespace map {
namespace ext {
namespace enc {

class MEPCS_Extensions : public asn1::ber::EncoderOfSequence_T<1> {
public:
  explicit MEPCS_Extensions(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfSequence_T<1>(use_rule)
  {}

  MEPCS_Extensions(const PCS_Extensions& pcs_exts, asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfSequence_T<1>(use_rule)
  {}

  MEPCS_Extensions(const asn1::ASTag& outer_tag,
                   const asn1::ASTagging::Environment_e tag_env,
                   asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfSequence_T<1>(outer_tag, tag_env, use_rule)
  {}
};

}}}}

#endif /* MEPCS_EXTENSIONS_HPP_ */
