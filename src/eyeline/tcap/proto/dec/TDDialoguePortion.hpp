/* ************************************************************************* *
 * TCAP Message Decoder: decoder of DialoguePortion field.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_DEC_DLG_PORTION_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROTO_DEC_DLG_PORTION_HPP

#include "eyeline/tcap/proto/TCDlgPortion.hpp"

#include "eyeline/tcap/proto/dec/TDStrDialoguePdu.hpp"
#include "eyeline/tcap/proto/dec/TDAPduAUDT.hpp"

#include "eyeline/asn1/BER/rtdec/DecodeINT.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeEOID.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeObjDescr.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeEmbdEnc.hpp"


namespace eyeline {
namespace tcap {
namespace proto {
namespace dec {

using eyeline::asn1::ber::TSGroupBER;

/* According to Q.773, DialoguePortion is defined as follow:

DialoguePortion ::= [APPLICATION 11] EXPLICIT EXTERNAL
-- The dialogue portion carries the dialogue control PDUs as value of the
-- external data type. The direct reference should be set to {itu-t 
-- recommendation q 773 as(1) dialogue-as(1) version1(1)} if structured 
-- dialogue is used and to {itu-t recommendation q 773 as(1) unidialogue-as(2)
-- version1(1)} if unstructured dialogue is used.

Taking into account definition of EXTERNAL type, the data structure associated
with DialoguePortion may be defined as:

DialoguePortion ::= [APPLICATION 11] EXPLICIT [UNIVERSAL 8] IMPLICIT SEQUENCE {
    direct-reference       OBJECT IDENTIFIER OPTIONAL,
    indirect-reference     INTEGER OPTIONAL,
    data-value-descriptor  ObjectDescriptor OPTIONAL,
    encoding               CHOICE {
        dRef-dialogue-as          StrDialogueAS,
        dRef-unidialogue-as       TCPduAUDT,
        other                     EmbeddedEncoding
    }
}
*/
class TDDialoguePortion : public asn1::ber::DecoderOfSequence_T<4> {
private:
  using asn1::ber::DecoderOfSequence_T<4>::setField;

  union {
    void *    _aligner;
    uint8_t   _buf[sizeof(asn1::ber::DecoderOfEOID)];
  }   _memDRef;

  union {
    void *    _aligner;
    uint8_t   _buf[sizeof(asn1::ber::DecoderOfINTEGER)];
  }   _memIRef;

  union {
    void *    _aligner;
    uint8_t   _buf[sizeof(asn1::ber::DecoderOfObjDescriptor)];
  }   _memDescr;

  union {
    void *    _aligner;
    uint8_t   _buf[eyeline::util::MaxSizeOf3_T<TDAPduAUDT, TDStrDialoguePdu,
                                        asn1::ber::DecoderOfEmbdEncoding>::VALUE];
  } _memEnc;

protected:
  static const asn1::ASTagging _typeTags; //complete tagging

  proto::TCDlgPortion *    _dVal;
  /* -- */
  asn1::ber::DecoderOfEOID *           _decDRef;
  asn1::ber::DecoderOfINTEGER *        _decIRef;
  asn1::ber::DecoderOfObjDescriptor *  _decDescr;

  proto::TCDlgPortion::ASKind_e _decKind;
  union {
    TDAPduAUDT *            _uni;
    TDStrDialoguePdu *      _dlg;
    asn1::ber::DecoderOfEmbdEncoding * _embd;
  } _decEnc;

  //Cleans allocated decoder of 'encoding' field
  void cleanEnc(void);
  //Cleans all allocated optional objects
  void cleanAll(void);
  //
  void initEncDLG(proto::TCStrDialoguePdu & use_val)
  {
    cleanEnc();
    _decEnc._dlg = new (_memEnc._buf) TDStrDialoguePdu(use_val, getVALRule());
    _decKind == proto::TCDlgPortion::asDLG;
  }
  //
  void initEncUNI(proto::TCPduAUDT & use_val)
  {
    cleanEnc();
    _decEnc._uni = new (_memEnc._buf) TDAPduAUDT(use_val, getVALRule());
    _decKind == proto::TCDlgPortion::asUNI;
  }
  //
  void initEncEXT(asn1::EmbeddedEncoding & use_val)
  {
    cleanEnc();
    _decEnc._embd = new (_memEnc._buf) asn1::ber::DecoderOfEmbdEncoding(use_val, getTSRule());
    _decKind == proto::TCDlgPortion::asEXT;
  }

  //Initializes ElementDecoder of this type
  void construct(void);

  // ----------------------------------------
  // -- DecoderOfStructAC interface methods
  // ----------------------------------------
  //If necessary, allocates optional element and initializes associated TypeDecoderAC
  virtual asn1::ber::TypeDecoderAC * prepareAlternative(uint16_t unique_idx)
    /*throw(std::exception) */;
  //Performs actions upon successfull optional element decoding
  virtual void markDecodedOptional(uint16_t unique_idx) /*throw() */;

public:
  static const asn1::ASTag _typeTag; //[APPLICATION 11] EXPLICIT
  
  explicit TDDialoguePortion(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleBER)
    : asn1::ber::DecoderOfSequence_T<4>(_typeTags, TSGroupBER::getTSRule(use_rule))
    , _dVal(0), _decDRef(0), _decIRef(0), _decDescr(0)
    , _decKind(proto::TCDlgPortion::asNone)
  {
    construct();
    _decEnc._uni = NULL;
    _memEnc._aligner = _memDescr._aligner = _memDRef._aligner = _memIRef._aligner =  0;
  }
  TDDialoguePortion(proto::TCDlgPortion & use_val,
                      TSGroupBER::Rule_e use_rule = TSGroupBER::ruleBER)
    : asn1::ber::DecoderOfSequence_T<4>(_typeTags, TSGroupBER::getTSRule(use_rule))
    , _dVal(&use_val), _decDRef(0), _decIRef(0), _decDescr(0)
    , _decKind(proto::TCDlgPortion::asNone)
  {
    construct();
    _decEnc._uni = NULL;
    _memEnc._aligner = _memDescr._aligner = _memDRef._aligner = _memIRef._aligner =  0;
    _dVal->initEXT();
  }
  //
  ~TDDialoguePortion()
  {
    cleanAll();
  }

  void setValue(proto::TCDlgPortion & use_val)
  {
    _dVal = &use_val;
    _dVal->initEXT().push_front(asn1::ASExternal());
    _seqDec.reset();
  }
};


}}}}

#endif /* __EYELINE_TCAP_PROTO_DEC_DLG_PORTION_HPP */

