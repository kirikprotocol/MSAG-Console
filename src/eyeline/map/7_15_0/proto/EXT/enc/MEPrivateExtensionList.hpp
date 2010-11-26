#ifndef __EYELINE_MAP_7F0_PROTO_EXT_ENC_MEPRIVATEEXTENSIONLIST_HPP__
#ifndef __GNUC__
# ident "@(#)$Id$"
#endif
# define __EYELINE_MAP_7F0_PROTO_EXT_ENC_MEPRIVATEEXTENSIONLIST_HPP__

# include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"
# include "eyeline/map/7_15_0/proto/EXT/PrivateExtensionList.hpp"

# include "eyeline/map/7_15_0/proto/EXT/enc/MEPrivateExtension.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeSeqOfLinked.hpp"

namespace eyeline {
namespace map {
namespace ext {
namespace enc {

/*
 PrivateExtensionList ::= SEQUENCE SIZE (1..maxNumOfPrivateExtensions) OF
                                PrivateExtension
*/
class MEPrivateExtensionList : public asn1::ber::EncoderOfSeqOfLinked_T<PrivateExtension, MEPrivateExtension, 2> {
public:
  static const unsigned _maxNumOfPrivateExtensions = 10;

  explicit MEPrivateExtensionList(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfSeqOfLinked_T<PrivateExtension, MEPrivateExtension, 2>(use_rule)
  {
    setMaxElements(_maxNumOfPrivateExtensions);
  }

  MEPrivateExtensionList(const PrivateExtensionList& private_ext_list,
                         asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfSeqOfLinked_T<PrivateExtension, MEPrivateExtension, 2>(use_rule)
  {
    setMaxElements(_maxNumOfPrivateExtensions);
    setValue(private_ext_list);
  }
  //
  MEPrivateExtensionList(const asn1::ASTag& outer_tag,
                         const asn1::ASTagging::Environment_e tag_env,
                         asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfSeqOfLinked_T<PrivateExtension, MEPrivateExtension, 2>(outer_tag, tag_env, use_rule)
  {
    setMaxElements(_maxNumOfPrivateExtensions);
  }
};

}}}}

#endif /* __EYELINE_MAP_7F0_PROTO_EXT_ENC_MEPRIVATEEXTENSIONLIST_HPP__ */

