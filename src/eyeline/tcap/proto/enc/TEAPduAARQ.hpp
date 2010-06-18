/* ************************************************************************* *
 *  TCAP Structured Dialogue Request APDU encoder.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_ENC_AARQ_APDU_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROTO_ENC_AARQ_APDU_HPP

#include "eyeline/tcap/proto/enc/TEProtocolVersion.hpp"
#include "eyeline/tcap/proto/enc/TEApplicationContext.hpp"
#include "eyeline/tcap/proto/enc/TEUserInformation.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

/* AARQ APdu is defined in EXPLICIT tagging environment as following:

AARQ-apdu ::= [APPLICATION 0] IMPLICIT SEQUENCE {
  protocol-version          ProtocolVersion DEFAULT {version1},
  application-context-name  ApplicationContext,
  user-information          UserInformation OPTIONAL
} */
class TEAPduAARQ : public asn1::ber::EncoderOfStructure_T<3> {
private:
  using asn1::ber::EncoderOfStructure_T<3>::addField;
  using asn1::ber::EncoderOfStructure_T<3>::setField;

  union {
    void *  aligner;
    uint8_t buf[sizeof(TEUserInformation)];
  } _memUI;

  TEUserInformation * _pUI; //OPTIONAL

protected:
  /* ----------------------------------------------- */
  TEProtocolVersion     _protoVer;
  TEApplicationContext  _appCtx;

/* ----------------------------------------------- */
  TEUserInformation * getUI(void)
  {
    if (!_pUI) {
      _pUI = new (_memUI.buf)TEUserInformation(TSGroupBER::getBERRule(getTSRule()));
      asn1::ber::EncoderOfStructure_T<3>::setField(2, *_pUI);
    }
    return _pUI;
  }
  void clearUI(void)
  {
    if (_pUI)
      _pUI->~TEUserInformation();
  }

public:
  static const asn1::ASTagging _typeTags; //[APPLICATION 0] IMPLICIT

  explicit TEAPduAARQ(const asn1::EncodedOID * app_ctx = 0,
                     TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfStructure_T<3>(_typeTags, TSGroupBER::getTSRule(use_rule))
    , _pUI(0), _protoVer(use_rule), _appCtx(use_rule)
  {
    _memUI.aligner = 0;
    if (app_ctx)
      _appCtx.setValue(*app_ctx);

    asn1::ber::EncoderOfStructure_T<3>::addField(_protoVer);
    asn1::ber::EncoderOfStructure_T<3>::addField(_appCtx);
  }
  ~TEAPduAARQ()
  {
    clearUI();
  }

  //
  void setAppContext(const asn1::EncodedOID & app_ctx)
  {
    _appCtx.setValue(app_ctx);
  }
  //
  asn1::ber::EncoderOfExternal * addUIValue(const asn1::ASExternal & val_ext)
  {
    return getUI()->addUIValue(val_ext);
  }
  void addUIList(const tcap::TDlgUserInfoPtrList & ui_list)
  {
    return getUI()->addUIList(ui_list);
  }
};


}}}}

#endif /* __EYELINE_TCAP_PROTO_ENC_AARQ_APDU_HPP */

