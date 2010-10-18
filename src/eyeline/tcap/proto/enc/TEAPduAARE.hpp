/* ************************************************************************* *
 * TCAP Structured Dialogue Response APDU encoder.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_ENC_AARE_APDU_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROTO_ENC_AARE_APDU_HPP

#include "eyeline/tcap/proto/TCAssociateDiagnostic.hpp"
#include "eyeline/tcap/proto/enc/TEProtocolVersion.hpp"
#include "eyeline/tcap/proto/enc/TEApplicationContext.hpp"
#include "eyeline/tcap/proto/enc/TEUserInformation.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeChoice.hpp"
#include "eyeline/asn1/BER/rtenc/EncoderProducer.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

/* AARE APdu is defined in EXPLICIT tagging environment as following:

AARE-apdu ::= [APPLICATION 1] IMPLICIT SEQUENCE {
  protocol-version          ProtocolVersion DEFAULT {version1},
  application-context-name  ApplicationContext,
  result                    [2]  Associate-result,
  result-source-diagnostic  [3]  Associate-source-diagnostic,
  user-information          UserInformation OPTIONAL
} */
class TEAPduAARE : public asn1::ber::EncoderOfStructure_T<5> {
protected:
  //AssociateResultField ::= [2] INTEGER {accepted(0), reject-permanent(1)}
  class TEResultField : public asn1::ber::EncoderOfINTEGER {
  private:
    using asn1::ber::EncoderOfINTEGER::setValue;

  public:
    static const asn1::ASTag _typeTag; //[2] EXPLICIT

    explicit TEResultField(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
      : asn1::ber::EncoderOfINTEGER(_typeTag, asn1::ASTagging::tagsEXPLICIT, use_rule)
    { }
    ~TEResultField()
    { }
    //
    void setValue(const TDialogueAssociate::Result_e & use_val)
    {
      asn1::ber::EncoderOfINTEGER::setValue(use_val);
    }
  };

  //ResultSourceDiagnosticField ::= [3] CHOICE {
  //  dialogue-service-user [1]  INTEGER { null(0), no-reason-given(1),
  //                                       application-context-name-not-supported(2) },
  //  dialogue-service-provider [2]  INTEGER { null(0), no-reason-given(1),
  //                                           no-common-dialogue-portion(2) }
  //}
  class TEResultDiagnosticField : public asn1::ber::EncoderOfChoice {
  private:
    using asn1::ber::EncoderOfChoice::setSelection;

    static const asn1::ASTag _tagUser; //[1] EXPLICIT
    static const asn1::ASTag _tagPrvd;  //[2] EXPLICIT

    asn1::ber::EncoderProducer_T<asn1::ber::EncoderOfINTEGER> _altEnc;

  public:
    static const asn1::ASTag _typeTag; //[3] EXPLICIT

    explicit TEResultDiagnosticField(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
      : asn1::ber::EncoderOfChoice(_typeTag, asn1::ASTagging::tagsEXPLICIT, use_rule)
    { }
    ~TEResultDiagnosticField()
    { }

    void setUserDiagnostic(const AssociateSourceDiagnostic::DiagnosticUser_e & use_cause)
    {
      _altEnc.init(_tagUser, asn1::ASTagging::tagsEXPLICIT, getTSRule()).setValue(use_cause);
      asn1::ber::EncoderOfChoice::setSelection(*_altEnc.get());
    }

    void setPrvdDiagnostic(const AssociateSourceDiagnostic::DiagnosticProvider_e & use_cause)
    {
      _altEnc.init(_tagPrvd, asn1::ASTagging::tagsEXPLICIT, getTSRule()).setValue(use_cause);
      asn1::ber::EncoderOfChoice::setSelection(*_altEnc.get());
    }
  };

  /* ----------------------------------------------- */
  TEProtocolVersion         _protoVer;
  TEApplicationContext      _appCtx;
  TEResultField             _ascResult;
  TEResultDiagnosticField   _ascDiagn;
  // Optionals:
  asn1::ber::EncoderProducer_T<TEUserInformation> _pUI;

  /* ----------------------------------------------- */
  TEUserInformation * getUI(void);
  void construct(void);

public:
  static const asn1::ASTag _typeTag; //[APPLICATION 1] IMPLICIT

  explicit TEAPduAARE(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfStructure_T<5>(asn1::ASTagging(_typeTag, asn1::ASTagging::tagsIMPLICIT), use_rule)
    , _protoVer(use_rule), _appCtx(use_rule), _ascResult(use_rule), _ascDiagn(use_rule)
  {
    construct();
  }
  explicit TEAPduAARE(const asn1::EncodedOID & app_ctx,
                     asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfStructure_T<5>(asn1::ASTagging(_typeTag, asn1::ASTagging::tagsIMPLICIT), use_rule)
    , _protoVer(use_rule), _appCtx(use_rule), _ascResult(use_rule), _ascDiagn(use_rule)
  {
    construct();
    _appCtx.setValue(app_ctx);
  }
  //
  ~TEAPduAARE()
  { }

  void setAppContext(const asn1::EncodedOID & app_ctx) { _appCtx.setValue(app_ctx); }

  void acceptByUser(void)
  {
    _ascResult.setValue(TDialogueAssociate::dlg_accepted);
    _ascDiagn.setUserDiagnostic(AssociateSourceDiagnostic::dsu_null);
  }

  void acceptByPrvd(void)
  {
    _ascResult.setValue(TDialogueAssociate::dlg_accepted);
    _ascDiagn.setPrvdDiagnostic(AssociateSourceDiagnostic::dsp_null);
  }

  void rejectByUser(AssociateSourceDiagnostic::DiagnosticUser_e use_cause
                    = AssociateSourceDiagnostic::dsu_null)
  {
    _ascResult.setValue(TDialogueAssociate::dlg_reject_permanent);
    _ascDiagn.setUserDiagnostic(use_cause);
  }

  void rejectByPrvd(AssociateSourceDiagnostic::DiagnosticProvider_e use_cause
                    = AssociateSourceDiagnostic::dsp_null)
  {
    _ascResult.setValue(TDialogueAssociate::dlg_reject_permanent);
    _ascDiagn.setPrvdDiagnostic(use_cause);
  }

  void addUIList(const tcap::TDlgUserInfoPtrList & ui_list)  /* throw(std::exception)*/
  {
    getUI()->setValue(ui_list);
  }
};

}}}}

#endif /* __EYELINE_TCAP_PROTO_ENC_AARE_APDU_HPP */

