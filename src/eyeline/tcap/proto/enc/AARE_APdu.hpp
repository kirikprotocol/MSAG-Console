#ifndef __EYELINE_TCAP_PROTO_ENC_AAREAPDU_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROTO_ENC_AAREAPDU_HPP__

# include "eyeline/asn1/TransferSyntax.hpp"
# include "eyeline/tcap/TDlgUserInfo.hpp"
# include "eyeline/tcap/proto/TCAssociateDiagnostic.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

class AARE_APdu : public asn1::ber::EncoderOfSequence {
public:
  enum AssociateResult_e {
    dlgAccepted = 0, dlgRejectPermanent = 1
  };
  enum ProtoVersion_e { protoVersion1 = 0 };

  enum {
    AARE_Tag_Value = 1, AARE_ProtocolVersion_Tag_Value = 0,
    AARE_AppCtxName_Tag_Value = 1, AARE_Result_Tag_Value = 2,
    AARE_ResultSourceDiag_Tag_Value = 3, AARE_UserInfo_Tag_Value = 30
  };
private:
  unsigned _protoVer;  //BIT STING
  const asn1::EncodedOID* _appCtx;      //mandatory!!!

  AssociateResult_e _result;
  AssociateSourceDiagnostic _diagnostic;
  const TDlgUserInfo* _usrInfo;

  static  asn1::ASTagging formPduTags() {
    asn1::ASTagging pduTags(2, asn1::ASTag(asn1::ASTag::tagApplication, AARE_Tag_Value),
                            asn1::_tagSEQOF);
    pduTags.setEnvironment(asn1::ASTagging::tagsIMPLICIT);

    return pduTags;
  }

  using EncoderOfSequence::addField;

public:
  AARE_APdu(ProtoVersion_e protocol_version,
            TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
  : EncoderOfSequence(formPduTags(), use_rule),
    _protoVer(protocol_version), _appCtx(NULL), _usrInfo(NULL)
  {}

  void setAppCtxName(const asn1::EncodedOID* app_ctx) {
    _appCtx = app_ctx;
  }

  void acceptByUser(void)
  {
    _result = dlgAccepted; _diagnostic.setUserDiagnostic();
  }
  void acceptByPrvd(void)
  {
    _result = dlgAccepted; _diagnostic.setPrvdDiagnostic();
  }

  void rejectByUser(AssociateSourceDiagnostic::DiagnosticUser_e use_cause =
                    AssociateSourceDiagnostic::dsu_null)
  {
    _result = dlgRejectPermanent; _diagnostic.setUserDiagnostic(use_cause);
  }
  void rejectByPrvd(AssociateSourceDiagnostic::DiagnosticProvider_e use_cause =
                        AssociateSourceDiagnostic::dsp_null)
  {
    _result = dlgRejectPermanent; _diagnostic.setPrvdDiagnostic(use_cause);
  }

  void setUserInfo(const TDlgUserInfo* usr_info) {
    _usrInfo = usr_info;
  }
};

}}}}

#endif
