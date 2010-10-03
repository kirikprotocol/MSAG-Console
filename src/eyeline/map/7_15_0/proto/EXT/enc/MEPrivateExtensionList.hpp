#ifndef __EYELINE_MAP_7F0_PROTO_EXT_ENC_MEPRIVATEEXTENSIONLIST_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_EXT_ENC_MEPRIVATEEXTENSIONLIST_HPP__

# include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"
# include "eyeline/map/7_15_0/proto/EXT/PrivateExtensionList.hpp"
# include "eyeline/map/7_15_0/proto/EXT/enc/MEPrivateExtension.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeSeqOf.hpp"

namespace eyeline {
namespace map {
namespace ext {
namespace enc {

/*
 PrivateExtensionList ::= SEQUENCE SIZE (1..maxNumOfPrivateExtensions) OF
                                PrivateExtension
*/
class MEPrivateExtensionList : public asn1::ber::EncoderOfSequenceOf_T<PrivateExtension, MEPrivateExtension, 2> {
public:
  explicit MEPrivateExtensionList(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfSequenceOf_T<PrivateExtension, MEPrivateExtension, 2>(use_rule)
  {}

  MEPrivateExtensionList(const PrivateExtensionList& private_ext_list,
                         asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfSequenceOf_T<PrivateExtension, MEPrivateExtension, 2>(use_rule)
  {
    setValue(private_ext_list);
  }

  MEPrivateExtensionList(const asn1::ASTag& outer_tag,
                         const asn1::ASTagging::Environment_e tag_env,
                         asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfSequenceOf_T<PrivateExtension, MEPrivateExtension, 2>(outer_tag, tag_env, use_rule)
  {}

  void setValue(const PrivateExtensionList& private_ext_list);

private:
  static const unsigned maxNumOfPrivateExtensions= 10;
};

}}}}

#endif
