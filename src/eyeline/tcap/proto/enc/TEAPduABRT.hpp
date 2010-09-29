/* ************************************************************************* *
 * TCAP Structured Dialogue Abort APDU encoder.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_ENC_ABRT_APDU_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROTO_ENC_ABRT_APDU_HPP

#include "eyeline/tcap/TDialogueDefs.hpp"
#include "eyeline/tcap/proto/enc/TEUserInformation.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

using eyeline::tcap::TDialogueAssociate;

/* ABRT APdu is defined in EXPLICIT tagging environment as following:

ABRT-apdu ::= [APPLICATION 4] IMPLICIT SEQUENCE {
  abort-source      [0] IMPLICIT ABRT-source,
  user-information  UserInformation OPTIONAL
}
*/
class TEAPduABRT : public asn1::ber::EncoderOfPlainStructure_T<2> {
private:
  using asn1::ber::EncoderOfPlainStructure_T<2>::addField;
  using asn1::ber::EncoderOfPlainStructure_T<2>::setField;

  union {
    void *  aligner;
    uint8_t buf[sizeof(TEUserInformation)];
  } _memUI;

  TEUserInformation * _pUI; //OPTIONAL

protected:
  class AbortSource : public asn1::ber::EncoderOfINTEGER {
  private:
    using asn1::ber::EncoderOfINTEGER::setValue;

  protected:
    TDialogueAssociate::AbrtSource_e  _srcId;

  public:
    static const asn1::ASTagging _typeTags; //[0] IMPLICIT

    AbortSource(TDialogueAssociate::AbrtSource_e abort_source = TDialogueAssociate::abrtServiceUser,
                TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
      : asn1::ber::EncoderOfINTEGER(_typeTags, TSGroupBER::getTSRule(use_rule))
      , _srcId(abort_source)
    { }

    void setValue(TDialogueAssociate::AbrtSource_e abort_source) { _srcId = abort_source; }
  };

  AbortSource       _abrtSrc;

  TEUserInformation * getUI(void)
  {
    if (!_pUI) {
      _pUI = new (_memUI.buf)TEUserInformation(TSGroupBER::getBERRule(getTSRule()));
      asn1::ber::EncoderOfPlainStructure_T<2>::setField(1, *_pUI);
    }
    return _pUI;
  }
  void clearUI(void)
  {
    if (_pUI)
      _pUI->~TEUserInformation();
  }

public:
  static const asn1::ASTagging _typeTags; //[APPLICATION 4] IMPLICIT

  TEAPduABRT(TDialogueAssociate::AbrtSource_e abort_source,
              TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfPlainStructure_T<2>(_typeTags, TSGroupBER::getTSRule(use_rule))
    , _pUI(0), _abrtSrc(abort_source)
  {
    _memUI.aligner = 0;
    asn1::ber::EncoderOfPlainStructure_T<2>::addField(_abrtSrc);
  }
  ~TEAPduABRT()
  {
    clearUI();
  }

  void setAbortSource(TDialogueAssociate::AbrtSource_e abort_source)
  {
    _abrtSrc.setValue(abort_source);
  }

  void addUIValue(const asn1::ASExternal & use_val)
  {
    getUI()->addValue(use_val);
  }

  void addUIList(const tcap::TDlgUserInfoPtrList & ui_list)
  {
    getUI()->addValuesList(ui_list);
  }
};

}}}}

#endif /* __EYELINE_TCAP_PROTO_ENC_ABRT_APDU_HPP */

