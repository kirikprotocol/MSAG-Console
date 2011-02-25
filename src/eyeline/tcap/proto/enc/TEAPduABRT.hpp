/* ************************************************************************* *
 * TCAP Structured Dialogue Abort APDU encoder.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_ENC_ABRT_APDU_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
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
class TEAPduABRT : public asn1::ber::EncoderOfStructure_T<2> {
protected:
  //abort-source field encoder
  class FEAbortSource : public asn1::ber::EncoderOfINTEGER {
  private:
    using asn1::ber::EncoderOfINTEGER::setValue;

  public:
    static const asn1::ASTagging _typeTags; //[0] IMPLICIT

    explicit FEAbortSource(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
      : asn1::ber::EncoderOfINTEGER(_typeTags, use_rule)
    { }
    ~FEAbortSource()
    { }

    void setValue(TDialogueAssociate::AbrtSource_e abort_source)
    {
      asn1::ber::EncoderOfINTEGER::setValue(abort_source);
    }
  };

/* ----------------------------------------------- */
  FEAbortSource       _abrtSrc;
  // Optionals:
  asn1::ber::EncoderProducer_T<TEUserInformation> _pUI;

/* ----------------------------------------------- */
  TEUserInformation * getUI(void);

public:
  static const asn1::ASTagging _typeTags; //[APPLICATION 4] IMPLICIT

  explicit TEAPduABRT(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfStructure_T<2>(_typeTags, use_rule), _abrtSrc(use_rule)
  {
    asn1::ber::EncoderOfStructure_T<2>::setField(0, _abrtSrc);
  }

  explicit TEAPduABRT(TDialogueAssociate::AbrtSource_e abort_source,
                      asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfStructure_T<2>(_typeTags, use_rule), _abrtSrc(use_rule)
  {
    asn1::ber::EncoderOfStructure_T<2>::setField(0, _abrtSrc);
    _abrtSrc.setValue(abort_source);
  }
  ~TEAPduABRT()
  { }

  void setAbortSource(TDialogueAssociate::AbrtSource_e abort_source)
  {
    _abrtSrc.setValue(abort_source);
  }

  void addUIList(const tcap::TDlgUserInfoPtrList & ui_list)
  {
    getUI()->setValue(ui_list);
  }
};

}}}}

#endif /* __EYELINE_TCAP_PROTO_ENC_ABRT_APDU_HPP */

