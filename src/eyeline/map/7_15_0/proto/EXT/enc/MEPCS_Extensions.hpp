#ifndef __EYELINE_MAP_7F0_PROTO_EXT_ENC_MEPCS_EXTENSIONS_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_EXT_ENC_MEPCS_EXTENSIONS_HPP__

# include "eyeline/map/7_15_0/proto/EXT/PCS_Extensions.hpp"

# include "eyeline/asn1/BER/rtenc/EncodeSequenceExt.hpp"

namespace eyeline {
namespace map {
namespace ext {
namespace enc {

/* Type is defined in IMPLICIT tagging environment as follow:
 PCS-Extensions ::= SEQUENCE { ... }
 */
class MEPCS_Extensions : public asn1::ber::EncoderOfExtensibleSequence_T<0> {
public:
  explicit MEPCS_Extensions(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfExtensibleSequence_T<0>(use_rule)
  {}

  MEPCS_Extensions(const PCS_Extensions& pcs_exts, asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfExtensibleSequence_T<0>(use_rule)
  {
    setValue(pcs_exts);
  }

  MEPCS_Extensions(const asn1::ASTag& outer_tag,
                   const asn1::ASTagging::Environment_e tag_env,
                   asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfExtensibleSequence_T<0>(outer_tag, tag_env, use_rule)
  {}

  void setValue(const PCS_Extensions& value);
};

}}}}

#endif /* MEPCS_EXTENSIONS_HPP_ */
