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
}
*/
class TEAPduAARE : public asn1::ber::EncoderOfPlainStructure_T<5> {
private:
  using asn1::ber::EncoderOfPlainStructure_T<5>::addField;
  using asn1::ber::EncoderOfPlainStructure_T<5>::setField;

  union {
    void *  aligner;
    uint8_t buf[sizeof(TEUserInformation)];
  } _memUI;

  TEUserInformation * _pUI; //OPTIONAL

protected:
  //AssociateResultField ::= [2] INTEGER {accepted(0), reject-permanent(1)}
  class TEResultField : public asn1::ber::EncoderOfINTEGER {
  private:
    using asn1::ber::EncoderOfINTEGER::setValue;

  public:
    static const asn1::ASTag _typeTag; //[2] EXPLICIT

    TEResultField(TDialogueAssociate::Result_e use_val = TDialogueAssociate::dlg_reject_permanent,
                   TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
      : asn1::ber::EncoderOfINTEGER(_typeTag, asn1::ASTagging::tagsEXPLICIT,
                                    TSGroupBER::getTSRule(use_rule))
    {
      setValue(use_val);
    }
    ~TEResultField()
    { }

    void setValue(TDialogueAssociate::Result_e use_val)
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

    union {
      void *  aligner;
      uint8_t buf[sizeof(asn1::ber::EncoderOfINTEGER)];
    } _memSel;

    asn1::ber::EncoderOfINTEGER * _pSel;

  public:
    static const asn1::ASTag _typeTag; //[3] EXPLICIT

    TEResultDiagnosticField(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
      : asn1::ber::EncoderOfChoice(_typeTag, asn1::ASTagging::tagsEXPLICIT,
                                   TSGroupBER::getTSRule(use_rule))
      , _pSel(0)
    {
      _memSel.aligner = 0;
    }
    ~TEResultDiagnosticField()
    {
      if (_pSel)
        _pSel->~EncoderOfINTEGER();
    }

    void setUserDiagnostic(AssociateSourceDiagnostic::DiagnosticUser_e use_cause)
    {
      _pSel = new (_memSel.buf)
          asn1::ber::EncoderOfINTEGER(use_cause, _tagUser, asn1::ASTagging::tagsEXPLICIT,
                                      getTSRule());
      asn1::ber::EncoderOfChoice::setSelection(*_pSel);
    }

    void setPrvdDiagnostic(AssociateSourceDiagnostic::DiagnosticProvider_e use_cause)
    {
      _pSel = new (_memSel.buf)
          asn1::ber::EncoderOfINTEGER(use_cause, _tagPrvd, asn1::ASTagging::tagsEXPLICIT,
                                      getTSRule());
      asn1::ber::EncoderOfChoice::setSelection(*_pSel);
    }
  };

  /* ----------------------------------------------- */
  TEProtocolVersion         _protoVer;
  TEApplicationContext      _appCtx;
  TEResultField             _ascResult;
  TEResultDiagnosticField   _ascDiagn;

/* ----------------------------------------------- */
  TEUserInformation * getUI(void)
  {
    if (!_pUI) {
      _pUI = new (_memUI.buf)TEUserInformation(TSGroupBER::getBERRule(getTSRule()));
      asn1::ber::EncoderOfPlainStructure_T<5>::setField(4, *_pUI);
    }
    return _pUI;
  }
  void clearUI(void)
  {
    if (_pUI)
      _pUI->~TEUserInformation();
  }

public:
  static const asn1::ASTag _typeTag; //[APPLICATION 1] IMPLICIT

  TEAPduAARE(const asn1::EncodedOID * app_ctx = 0,
                     TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfPlainStructure_T<5>(asn1::ASTagging(_typeTag, asn1::ASTagging::tagsIMPLICIT),
                                        TSGroupBER::getTSRule(use_rule))
    , _pUI(0), _protoVer(use_rule), _appCtx(use_rule)
    , _ascResult(TDialogueAssociate::dlg_reject_permanent, use_rule)
    , _ascDiagn(use_rule)
  {
    _memUI.aligner = 0;
    if (app_ctx)
      _appCtx.setValue(*app_ctx);
    asn1::ber::EncoderOfPlainStructure_T<5>::addField(_protoVer);
    asn1::ber::EncoderOfPlainStructure_T<5>::addField(_appCtx);
    asn1::ber::EncoderOfPlainStructure_T<5>::addField(_ascResult);
    asn1::ber::EncoderOfPlainStructure_T<5>::addField(_ascDiagn);
  }
  ~TEAPduAARE()
  {
    clearUI();
  }

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

  void addUIValue(const asn1::ASExternal & use_val) /* throw(std::exception)*/
  {
    getUI()->addValue(use_val);
  }

  void addUIList(const tcap::TDlgUserInfoPtrList & ui_list)  /* throw(std::exception)*/
  {
    getUI()->addValuesList(ui_list);
  }

};

}}}}

#endif /* __EYELINE_TCAP_PROTO_ENC_AARE_APDU_HPP */

