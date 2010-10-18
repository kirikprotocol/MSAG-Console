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
protected:
  /* ----------------------------------------------- */
  TEProtocolVersion     _protoVer;
  TEApplicationContext  _appCtx;
  // Optionals:
  asn1::ber::EncoderProducer_T<TEUserInformation> _pUI;

/* ----------------------------------------------- */
  void construct(void);
  TEUserInformation * getUI(void);

public:
  static const asn1::ASTagging _typeTags; //[APPLICATION 0] IMPLICIT

  explicit TEAPduAARQ(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfStructure_T<3>(_typeTags, use_rule)
    , _protoVer(use_rule), _appCtx(use_rule)
  {
    construct();
  }
  explicit TEAPduAARQ(const asn1::EncodedOID & app_ctx,
                      asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfStructure_T<3>(_typeTags, use_rule)
    , _protoVer(use_rule), _appCtx(use_rule)
  {
    construct();
    _appCtx.setValue(app_ctx);
  }
  ~TEAPduAARQ()
  { }

  //
  void setAppContext(const asn1::EncodedOID & app_ctx)
  {
    _appCtx.setValue(app_ctx);
  }
  void addUIList(const tcap::TDlgUserInfoPtrList & ui_list) /*throw(std::exception)*/
  {
    getUI()->setValue(ui_list);
  }
};


}}}}

#endif /* __EYELINE_TCAP_PROTO_ENC_AARQ_APDU_HPP */

