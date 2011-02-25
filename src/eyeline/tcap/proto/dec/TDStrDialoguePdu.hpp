/* ************************************************************************* *
 * TCAP Message Decoder: decoder of Structured Dialogue PDU.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_DEC_STRDLG_PDU_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_TCAP_PROTO_DEC_STRDLG_PDU_HPP

#include "eyeline/tcap/proto/dec/TDAPduAARQ.hpp"
#include "eyeline/tcap/proto/dec/TDAPduAARE.hpp"
#include "eyeline/tcap/proto/dec/TDAPduABRT.hpp"

#include "eyeline/asn1/BER/rtdec/DecodeChoice.hpp"
#include "eyeline/asn1/BER/rtdec/DecodersChoiceT.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace dec {

using eyeline::asn1::ber::TSGroupBER;

/* According to Q.773, StructuredDialoguePDU is defined in EXPLICIT tagging
   environment as follow:

DialoguePDU ::= CHOICE {
  dialogueRequest   AARQ-apdu,
  dialogueResponse  AARE-apdu,
  dialogueAbort     ABRT-apdu
} */
class TDStrDialoguePdu : public asn1::ber::DecoderOfChoice_T<3> {
protected:
  using asn1::ber::DecoderOfChoice_T<3>::setAlternative;

  class AltDecoder : public 
    asn1::ber::ChoiceOfDecoders3_T<TDAPduAARQ, TDAPduAARE, TDAPduABRT> {
  public:
    AltDecoder()
      : asn1::ber::ChoiceOfDecoders3_T<TDAPduAARQ, TDAPduAARE, TDAPduABRT>()
    { }
    ~AltDecoder()
    { }

    TCDlgPduAC::PduKind_e getKind(void) const { return static_cast<TCDlgPduAC::PduKind_e>(getChoiceIdx()); }

    Alternative_T<TDAPduAARQ> aarq() { return alternative0(); }
    Alternative_T<TDAPduAARE> aare() { return alternative1(); }
    Alternative_T<TDAPduABRT> abrt() { return alternative2(); }

    ConstAlternative_T<TDAPduAARQ> aarq() const { return alternative0(); }
    ConstAlternative_T<TDAPduAARE> aare() const { return alternative1(); }
    ConstAlternative_T<TDAPduABRT> abrt() const { return alternative2(); }
  };
  /* ----------------------------------------------- */
  proto::TCStrDialoguePdu * _dVal;
  AltDecoder                _pDec;
  /* ----------------------------------------------- */
  //Initializes ElementDecoder of this type
  void construct(void);

  // ----------------------------------------
  // -- DecoderOfChoiceAC interface methods
  // ----------------------------------------
  //If necessary, allocates optional element and initializes associated TypeDecoderAC
  virtual TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */;

public:
  static const asn1::ASTag _typeTag; //[APPLICATION 11] EXPLICIT

  explicit TDStrDialoguePdu(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfChoice_T<3>(use_rule), _dVal(0)
  {
    construct();
  }
  explicit TDStrDialoguePdu(proto::TCStrDialoguePdu & use_val,
                            asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfChoice_T<3>(use_rule), _dVal(&use_val)
  {
    construct();
  }
  //
  ~TDStrDialoguePdu()
  { }

  void setValue(proto::TCStrDialoguePdu & use_val)
  {
    _dVal = &use_val;
  }
};


}}}}

#endif /* __EYELINE_TCAP_PROTO_DEC_STRDLG_PDU_HPP */

