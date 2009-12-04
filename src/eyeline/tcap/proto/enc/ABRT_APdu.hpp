#ifndef __EYELINE_TCAP_PROTO_ENC_ABRTAPDU_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROTO_ENC_ABRTAPDU_HPP__

# include "eyeline/tcap/TDlgUserInfo.hpp"
# include "eyeline/tcap/TDialogueDefs.hpp"

# include "eyeline/asn1/TransferSyntax.hpp"
# include "eyeline/asn1/ASNTags.hpp"
# include "eyeline/asn1/BER/rtenc/TLVEncoder.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeINT.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

class ABRT_APdu : public asn1::ber::EncoderOfSequence {
public:
  explicit ABRT_APdu(TDialogueAssociate::AbrtSource_e abort_source,
                     asn1::TSGroupBER::Rule_e use_rule = asn1::TSGroupBER::ruleDER);

  void setUserInfo(const TDlgUserInfo* usr_info) {
    _usrInfo = usr_info;
  }

  void arrangeFields();

private:
  enum TypeTags_e {
    ABRT_Tag = 4
  };

  enum FieldTags_e {
    AbortSource_FieldTag = 0
  };

  asn1::ber::EncoderOfINTEGER* _abortSourceEncoder;
  union {
    void* aligner;
    uint8_t allocation[sizeof(asn1::ber::EncoderOfINTEGER)];
  } _memForAbortSourceEncoder;

  const TDlgUserInfo* _usrInfo;

  static const asn1::ASTagging _typeTags;
  static const asn1::ASTagging _abortSource_fieldTags;
};

}}}}

#endif
