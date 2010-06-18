/* ************************************************************************* *
 * TCAP Message Decoder: decoder of Structured Dialogue PDU.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_DEC_STRDLG_PDU_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROTO_DEC_STRDLG_PDU_HPP

#include "eyeline/tcap/proto/dec/TDAPduAARQ.hpp"
#include "eyeline/tcap/proto/dec/TDAPduAARE.hpp"
#include "eyeline/tcap/proto/dec/TDAPduABRT.hpp"

#include "eyeline/asn1/BER/rtdec/DecodeChoice.hpp"

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
private:
  using asn1::ber::DecoderOfChoice_T<3>::setAlternative;

  union {
    void *    _aligner;
    uint8_t   _buf[eyeline::util::MaxSizeOf3_T<TDAPduAARQ, TDAPduAARE, TDAPduABRT>::VALUE];
  } _memPdu;

protected:
  proto::TCStrDialoguePdu * _dVal;
  union {
    asn1::ber::TypeDecoderAC * _any;
    TDAPduAARQ *  _aarq;
    TDAPduAARE *  _aare;
    TDAPduABRT *  _abrt;
  } _pDec;

  void cleanUp(void)
  {
    if (_pDec._any) {
      _pDec._any->~TypeDecoderAC();
      _pDec._any = NULL;
    }
  }
  //Initializes ElementDecoder of this type
  void construct(void);

  // ----------------------------------------
  // -- DecoderOfChoiceAC interface methods
  // ----------------------------------------
  //If necessary, allocates optional element and initializes associated TypeDecoderAC
  virtual TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */;

public:
  static const asn1::ASTag _typeTag; //[APPLICATION 11] EXPLICIT

  explicit TDStrDialoguePdu(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleBER)
    : asn1::ber::DecoderOfChoice_T<3>(TSGroupBER::getTSRule(use_rule))
    , _dVal(0)
  {
    construct();
    _memPdu._aligner = 0;
    _pDec._any = NULL;
  }
  TDStrDialoguePdu(proto::TCStrDialoguePdu & use_val,
                      TSGroupBER::Rule_e use_rule = TSGroupBER::ruleBER)
    : asn1::ber::DecoderOfChoice_T<3>(TSGroupBER::getTSRule(use_rule))
    , _dVal(&use_val)
  {
    construct();
    _memPdu._aligner = 0;
    _pDec._any = NULL;
  }
  //
  ~TDStrDialoguePdu()
  {
    cleanUp();
  }

  void setValue(proto::TCStrDialoguePdu & use_val)
  {
    _dVal = &use_val;
    _chcDec.reset();
  }
};


}}}}

#endif /* __EYELINE_TCAP_PROTO_DEC_STRDLG_PDU_HPP */

