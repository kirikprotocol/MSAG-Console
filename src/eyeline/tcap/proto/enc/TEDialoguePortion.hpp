/* ************************************************************************* *
 *  TCAP Message Dialogue Portion as Structured Dialogue PDUs carrier encoder.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_ENC_DIALOGUE_PORTION_STR_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROTO_ENC_DIALOGUE_PORTION_STR_HPP

#include "eyeline/tcap/proto/TCStrDialogue.hpp" //_ac_tcap_strDialogue_as

#include "eyeline/tcap/proto/enc/TEAPduAARQ.hpp"
#include "eyeline/tcap/proto/enc/TEAPduAARE.hpp"
#include "eyeline/tcap/proto/enc/TEAPduABRT.hpp"

#include "eyeline/asn1/BER/rtenc/EncodersChoiceT.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

/* DialoguePortion is defined in EXPLICIT tagging environment as follow:

DialoguePortion ::= [APPLICATION 11] EXPLICIT EXTERNAL
-- The dialogue portion carries the dialogue control PDUs as value of the
-- external data type. The direct reference should be set to {itu-t 
-- recommendation q 773 as(1) dialogue-as(1) version1(1)} if structured 
-- dialogue is used. 
*/
class TEDialoguePortionStructured : public asn1::ber::EncoderOfExternal {
private:
  using asn1::ber::EncoderOfExternal::setValue;

protected:
  class PduEncoder : public asn1::ber::ChoiceOfEncoders3_T<
                              TEAPduAARQ, TEAPduAARE, TEAPduABRT> {
  public:
    Alternative_T<TEAPduAARQ>     aarq()  { return alternative0(); }
    Alternative_T<TEAPduAARE>     aare()  { return alternative1(); }
    Alternative_T<TEAPduABRT>     abrt()  { return alternative2(); }

    ConstAlternative_T<TEAPduAARQ>  aarq()  const { return alternative0(); }
    ConstAlternative_T<TEAPduAARE>  aare()  const { return alternative1(); }
    ConstAlternative_T<TEAPduABRT>  abrt()  const { return alternative2(); }
  };

  PduEncoder  _pduEnc;

public:
  enum PDUKind_e { pduAARQ = 0, pduAARE = 1, pduABRT = 2,  pduNone = -1 };

  static const asn1::ASTagging _typeTags; //[APPLICATION 11] EXPLICIT

  explicit TEDialoguePortionStructured(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfExternal(_typeTags, use_rule)
  { }
  ~TEDialoguePortionStructured()
  { }

  void setValue(const asn1::ASExternal & val_ext) /*throw(std::exception)*/
  {
    _pduEnc.clear();
    asn1::ber::EncoderOfExternal::setValue(val_ext);
  }

  PDUKind_e getPDUKind(void) const { return static_cast<PDUKind_e>(_pduEnc.getChoiceIdx()); }

  // ---------------------------------------------------------------------------------------
  //NOTE: it's a caller responsibility to check PDU kind prior to using getPdu[*]() methods
  // ---------------------------------------------------------------------------------------

  //Creates and initializes AARQ_APdu
  TEAPduAARQ * initPduAARQ(const asn1::EncodedOID & use_app_ctx) /*throw(std::exception)*/;
  //Returns initialized AARQ_APdu
  TEAPduAARQ * getPduAARQ(void) { return _pduEnc.aarq().get(); }

  //Creates and initializes AARE_APdu
  TEAPduAARE * initPduAARE(const asn1::EncodedOID & use_app_ctx) /*throw(std::exception)*/;
  //Returns initialized AARE_APdu
  TEAPduAARE * getPduAARE(void) { return _pduEnc.aare().get(); }

  //Creates and initializes ABRT_APdu
  TEAPduABRT * initPduABRT(TDialogueAssociate::AbrtSource_e abrt_src) /*throw(std::exception)*/;
  //Returns initialized ABRT_APdu
  TEAPduABRT * getPduABRT(void) { return _pduEnc.abrt().get(); }
};

}}}}

#endif /* __EYELINE_TCAP_PROTO_ENC_DIALOGUE_PORTION_STR_HPP */

