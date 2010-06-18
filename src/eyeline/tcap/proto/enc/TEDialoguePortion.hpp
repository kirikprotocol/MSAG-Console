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
public:
  enum PDUKind_e { pduNone = 0, pduAARQ, pduAARE, pduABRT };

private:
  using asn1::ber::EncoderOfExternal::setValue;

  union {
    void * aligner;
    uint64_t buf[eyeline::util::MultiplierOfSize_T<
                  eyeline::util::MaxSizeOf3_T<TEAPduAARQ, TEAPduAARE, TEAPduABRT>::VALUE
                  , uint64_t>::VALUE];
  } _memPdu;

protected:
  PDUKind_e   _pduKind;
  union {
    void *      ptr;
    TEAPduAARQ * aarq;
    TEAPduAARE * aare;
    TEAPduABRT * abrt;
  } _pdu;
 

public:
  static const asn1::ASTagging _typeTags; //[APPLICATION 11] EXPLICIT

  TEDialoguePortionStructured(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfExternal(_typeTags, TSGroupBER::getTSRule(use_rule))
    , _pduKind(pduNone)
  {
    _pdu.ptr = _memPdu.aligner = 0;
  }
  ~TEDialoguePortionStructured()
  {
    if (_pdu.ptr) {
      if (_pduKind == pduAARQ)
        _pdu.aarq->~TEAPduAARQ();
      else if (_pduKind == pduAARE)
        _pdu.aare->~TEAPduAARE();
      else
        _pdu.abrt->~TEAPduABRT();
    }
  }

  void setValue(const asn1::ASExternal & val_ext) /*throw(std::exception)*/
  {
    _pduKind = pduNone;
    asn1::ber::EncoderOfExternal::setValue(val_ext);
  }


  PDUKind_e getPDUKind(void) const { return _pduKind; }

  // ---------------------------------------------------------------------------------------
  //NOTE: it's a caller responsibility to check PDU kind prior to using getPdu[*]() methods
  // ---------------------------------------------------------------------------------------

  //Creates and initializes AARQ_APdu
  TEAPduAARQ * getPduAARQ(const asn1::EncodedOID & use_app_ctx) /*throw(std::exception)*/
  {
    if (!_pdu.ptr) {
      _pdu.aarq = new (_memPdu.buf)TEAPduAARQ(&use_app_ctx, TSGroupBER::getBERRule(getTSRule()));
      asn1::ber::EncoderOfExternal::setValue(_ac_tcap_strDialogue_as, *_pdu.aarq);
      _pduKind = pduAARQ;
    }
    return _pdu.aarq;
  }
  //Returns initialized AARQ_APdu
  TEAPduAARQ * getPduAARQ(void) { return _pdu.aarq; }

  //Creates and initializes AARE_APdu
  TEAPduAARE * getPduAARE(const asn1::EncodedOID & use_app_ctx) /*throw(std::exception)*/
  {
    if (!_pdu.ptr) {
      _pdu.aare = new (_memPdu.buf)TEAPduAARE(&use_app_ctx, TSGroupBER::getBERRule(getTSRule()));
      asn1::ber::EncoderOfExternal::setValue(_ac_tcap_strDialogue_as, *_pdu.aare);
      _pduKind = pduAARE;
    }
    return _pdu.aare;
  }
  //Returns initialized AARE_APdu
  TEAPduAARE * getPduAARE(void) { return _pdu.aare; }

  //Creates and initializes ABRT_APdu
  TEAPduABRT * getPduABRT(TDialogueAssociate::AbrtSource_e abrt_src) /*throw(std::exception)*/
  {
    if (!_pdu.ptr) {
      _pdu.abrt = new (_memPdu.buf)TEAPduABRT(abrt_src, TSGroupBER::getBERRule(getTSRule()));
      asn1::ber::EncoderOfExternal::setValue(_ac_tcap_strDialogue_as, *_pdu.abrt);
      _pduKind = pduABRT;
    }
    return _pdu.abrt;
  }
  //Returns initialized ABRT_APdu
  TEAPduABRT * getPduABRT(void) { return _pdu.abrt; }
};

}}}}

#endif /* __EYELINE_TCAP_PROTO_ENC_DIALOGUE_PORTION_STR_HPP */

