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
  : asn1::ber::EncoderOfSequenceOf_T<PrivateExtension, MEPrivateExtension, 2>(use_rule), _idx(0)
  {}

  MEPrivateExtensionList(const PrivateExtensionList& private_ext_list,
                         asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfSequenceOf_T<PrivateExtension, MEPrivateExtension, 2>(use_rule), _idx(0)
  {
    setValue(private_ext_list);
  }

  void setValue(const PrivateExtensionList& private_ext_list) {
    for(PrivateExtensionList::const_iterator iter= private_ext_list.begin(), end_iter= private_ext_list.end();
        iter != end_iter; ++iter ) {
      if (_idx == maxNumOfPrivateExtensions)
        throw smsc::util::Exception("MEPrivateExtensionList::MEPrivateExtensionList::: too long PrivateExtensionList");
      ++_idx;
      addValue(*iter);
    }
  }
private:
  static const unsigned maxNumOfPrivateExtensions= 10;
  unsigned _idx;
};

}}}}

#endif
