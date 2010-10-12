#ifndef __EYELINE_MAP_7F0_PROTO_EXT_DEC_MDPRIVATEEXTENSIONLIST_HPP__
# define __EYELINE_MAP_7F0_PROTO_EXT_DEC_MDPRIVATEEXTENSIONLIST_HPP__

# include "eyeline/asn1/BER/rtdec/DecodeSeqOfLinked.hpp"
# include "eyeline/map/7_15_0/proto/EXT/PrivateExtension.hpp"
# include "eyeline/map/7_15_0/proto/EXT/dec/MDPrivateExtension.hpp"

namespace eyeline {
namespace map {
namespace ext {
namespace dec {

/*
 PrivateExtensionList ::= SEQUENCE SIZE (1..maxNumOfPrivateExtensions) OF
                                PrivateExtension
*/
class MDPrivateExtensionList : public asn1::ber::DecoderOfSeqOfLinked_T<PrivateExtension, MDPrivateExtension> {
public:
  explicit MDPrivateExtensionList(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
  : asn1::ber::DecoderOfSeqOfLinked_T<PrivateExtension, MDPrivateExtension>(use_rule)
  {}

  MDPrivateExtensionList(const asn1::ASTag & use_tag, asn1::ASTagging::Environment_e tag_env,
                         asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfSeqOfLinked_T<PrivateExtension, MDPrivateExtension>(use_tag, tag_env, use_rule)
  {}
};

}}}}

#endif
