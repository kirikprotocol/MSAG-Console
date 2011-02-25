/* ************************************************************************* *
 * TCAP Message Decoder: decoder of DialoguePortion field.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_DEC_DLG_PORTION_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_TCAP_PROTO_DEC_DLG_PORTION_HPP

#include "eyeline/tcap/proto/TCDlgPortion.hpp"

#include "eyeline/tcap/proto/dec/TDStrDialoguePdu.hpp"
#include "eyeline/tcap/proto/dec/TDAPduAUDT.hpp"

#include "eyeline/asn1/BER/rtdec/DecodeINT.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeEOID.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeObjDescr.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeEmbdEnc.hpp"
#include "eyeline/asn1/BER/rtdec/DecoderProducer.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace dec {

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
} */
class TDDialoguePortion : public asn1::ber::DecoderOfSequence_T<4> {
protected:
  static const asn1::ASTagging _typeTags; //complete tagging

  typedef asn1::ber::DecoderOfEmbdEncoding TDEmbdEncoding;

  //Decoder of Field #3 'encoding' 
  class F3Decoder : public 
    asn1::ber::ChoiceOfDecoders3_T<TDStrDialoguePdu, TDAPduAUDT, TDEmbdEncoding> {
  public:
    //Associated TransferSyntax id
    enum TSKind_e { tsNone = -1, tsDLG = 0, tsUNI = 1, tsEMBD = 2 };

    F3Decoder()
      : asn1::ber::ChoiceOfDecoders3_T<TDStrDialoguePdu, TDAPduAUDT, TDEmbdEncoding>()
    { }
    ~F3Decoder()
    { }

    TSKind_e getKind(void) const { return static_cast<TSKind_e>(getChoiceIdx()); }

    Alternative_T<TDStrDialoguePdu> dlg() { return alternative0(); }
    Alternative_T<TDAPduAUDT>       uni() { return alternative1(); }
    Alternative_T<TDEmbdEncoding>   embd() { return alternative2(); }

    ConstAlternative_T<TDStrDialoguePdu> dlg() const { return alternative0(); }
    ConstAlternative_T<TDAPduAUDT>       uni() const { return alternative1(); }
    ConstAlternative_T<TDEmbdEncoding>   embd() const { return alternative2(); }
  };
  /* ----------------------------------------------- */

  proto::TCDlgPortion *   _dVal;
  /* -- */
  F3Decoder               _decAS;
  asn1::ber::DecoderProducer_T<asn1::ber::DecoderOfEOID>           _decDRef;
  asn1::ber::DecoderProducer_T<asn1::ber::DecoderOfINTEGER>        _decIRef;
  asn1::ber::DecoderProducer_T<asn1::ber::DecoderOfObjDescriptor>  _decDescr;
  /* ----------------------------------------------- */

  void clearAll(void);

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
  
  explicit TDDialoguePortion(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfSequence_T<4>(_typeTags, use_rule), _dVal(0)
  {
    construct();
  }
  explicit TDDialoguePortion(proto::TCDlgPortion & use_val,
                             asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfSequence_T<4>(_typeTags, use_rule), _dVal(&use_val)
  {
    construct();
    _dVal->ext().init().push_front(asn1::ASExternal());
  }
  //
  ~TDDialoguePortion()
  { }

  void setValue(proto::TCDlgPortion & use_val)
  {
    _dVal = &use_val;
    _dVal->ext().init().push_front(asn1::ASExternal());
  }
};


}}}}

#endif /* __EYELINE_TCAP_PROTO_DEC_DLG_PORTION_HPP */

