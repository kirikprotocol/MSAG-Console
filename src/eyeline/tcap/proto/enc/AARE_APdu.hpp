#ifndef __EYELINE_TCAP_PROTO_ENC_AAREAPDU_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROTO_ENC_AAREAPDU_HPP__

# include "eyeline/tcap/TDlgUserInfo.hpp"
# include "eyeline/tcap/proto/TCAssociateDiagnostic.hpp"

# include "eyeline/asn1/TransferSyntax.hpp"
# include "eyeline/asn1/EncodedOID.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeEOID.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeINT.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeChoice.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

class AARE_APdu : public asn1::ber::EncoderOfSequence {
public:
  explicit AARE_APdu(const asn1::EncodedOID* app_ctx,
                     asn1::TSGroupBER::Rule_e use_rule = asn1::TSGroupBER::ruleDER);

  void acceptByUser(void);

  void acceptByPrvd(void);

  void rejectByUser(AssociateSourceDiagnostic::DiagnosticUser_e cause);

  void rejectByPrvd(AssociateSourceDiagnostic::DiagnosticProvider_e cause);

  void setUserInfo(const TDlgUserInfo* usr_info) {
    _usrInfo = usr_info;
  }

  void arrangeFields();

private:
  enum AssociateResult_e {
    dlgAccepted = 0, dlgRejectPermanent = 1
  };

  enum TypeTags_e {
    AARE_Tag = 1
  };

  enum FieldTags_e {
    ProtocolVersion_FieldTag = 0,
    AppCtxName_FieldTag = 1, Result_FieldTag = 2,
    ResultSourceDiag_FieldTag = 3, UserInfo_FieldTag = 30
  };

  enum SelectionTags_e {
    DlgSvcUser_SelectionTag = 1, DlgSvcProvider_SelectionTag = 2
  };

  asn1::ber::EncoderOfEOID* _appCtxNameEncoder;
  union {
    void* aligner;
    uint8_t allocation[sizeof(asn1::ber::EncoderOfEOID)];
  } _memForAppCtxNameEncoder;

  asn1::ber::EncoderOfINTEGER* _resultEncoder;
  union {
    void* aligner;
    uint8_t allocation[sizeof(asn1::ber::EncoderOfINTEGER)];
  } _memForResultEncoder;

  asn1::ber::EncoderOfChoice* _resultSrcDiagEncoder;
  union {
    void* aligner;
    uint8_t allocation[sizeof(asn1::ber::EncoderOfChoice)];
  } _memForResultSrcDiagEncoder;

  const TDlgUserInfo* _usrInfo;

  static const asn1::ASTagging _typeTags;

  static const asn1::ASTagging _protocolVersion_fieldTags;
  static const asn1::ASTagging _appCtxName_fieldTags;
  static const asn1::ASTagging _associateResult_fieldTags;
  static const asn1::ASTagging _associateSrcDiag_fieldTags;
  static const asn1::ASTagging _usrInfo_fieldTags;

  static const asn1::ASTagging _dlgSvcUser_selectionTags;
  static const asn1::ASTagging _dlgSvcProvider_selectionTags;

  enum DiagnosticUser_e {
    dsu_null = 0, dsu_no_reason_given = 1, dsu_ac_not_supported = 2
  };
  enum DiagnosticProvider_e {
    dsp_null = 0, dsp_no_reason_given = 1, dsp_no_common_dialogue_portion = 2
  };

  static asn1::ber::EncoderOfINTEGER _dlgSvcUserValues[3];
  static asn1::ber::EncoderOfINTEGER _dlgSvcPrvdValues[3];
};

}}}}

#endif
