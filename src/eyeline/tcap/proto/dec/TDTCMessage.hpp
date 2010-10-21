/* ************************************************************************* *
 * TCAP Message Decoder.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_DEC_TCAP_MSG_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROTO_DEC_TCAP_MSG_HPP

#include "eyeline/tcap/proto/TCMessage.hpp"

#include "eyeline/tcap/proto/dec/TDMsgTUnidir.hpp"
#include "eyeline/tcap/proto/dec/TDMsgTBegin.hpp"
#include "eyeline/tcap/proto/dec/TDMsgTEnd.hpp"
#include "eyeline/tcap/proto/dec/TDMsgTContinue.hpp"
#include "eyeline/tcap/proto/dec/TDMsgTAbort.hpp"

#include "eyeline/asn1/BER/rtdec/DecodeChoice.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace dec {

/* According to Q.773, TCMessage is defined in IMPLICIT tagging
   environment as follow:

TCMessage ::= CHOICE {
  unidirectional    Unidirectional,
  begin             Begin,
  end               End,
  continue          Continue,
  abort             Abort
} */
class TDTCMessage : public asn1::ber::DecoderOfChoice_T<5> {
protected:
  using asn1::ber::DecoderOfChoice_T<5>::setAlternative;

  class AltDecoder : public 
    asn1::ber::ChoiceOfDecoders5_T<TDMsgTUnidir, TDMsgTBegin,
                              TDMsgTEnd, TDMsgTContinue, TDMsgTAbort> {
  public:
    AltDecoder()
      : asn1::ber::ChoiceOfDecoders5_T<TDMsgTUnidir, TDMsgTBegin,
                              TDMsgTEnd, TDMsgTContinue, TDMsgTAbort>()
    { }
    ~AltDecoder()
    { }

    TCDlgPduAC::PduKind_e getKind(void) const { return static_cast<TCDlgPduAC::PduKind_e>(getChoiceIdx()); }

    Alternative_T<TDMsgTUnidir>   unidir()  { return alternative0(); }
    Alternative_T<TDMsgTBegin>    begin()   { return alternative1(); }
    Alternative_T<TDMsgTEnd>      end()     { return alternative2(); }
    Alternative_T<TDMsgTContinue> cont()    { return alternative3(); }
    Alternative_T<TDMsgTAbort>    abort()   { return alternative4(); }

    ConstAlternative_T<TDMsgTUnidir>   unidir() const { return alternative0(); }
    ConstAlternative_T<TDMsgTBegin>    begin()  const { return alternative1(); }
    ConstAlternative_T<TDMsgTEnd>      end()    const { return alternative2(); }
    ConstAlternative_T<TDMsgTContinue> cont()   const { return alternative3(); }
    ConstAlternative_T<TDMsgTAbort>    abort()  const { return alternative4(); }
  };
  /* ----------------------------------------------- */

  proto::TCMessage *  _dVal;
  AltDecoder          _pDec;
  /* ----------------------------------------------- */
  //Initializes ElementDecoder of this type
  void construct(void);

  // ----------------------------------------
  // -- DecoderOfChoiceAC interface methods
  // ----------------------------------------
  //If necessary, allocates optional element and initializes associated TypeDecoderAC
  virtual TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */;

public:
  class TaggingOptions : public asn1::ber::TaggingOptions {
  public:
    TaggingOptions() : asn1::ber::TaggingOptions()
    {
      addTagging(TDMsgTUnidir::_typeTag, asn1::ASTagging::tagsIMPLICIT);
      addTagging(TDMsgTBegin::_typeTag, asn1::ASTagging::tagsIMPLICIT);
      addTagging(TDMsgTEnd::_typeTag, asn1::ASTagging::tagsIMPLICIT);
      addTagging(TDMsgTContinue::_typeTag, asn1::ASTagging::tagsIMPLICIT);
      addTagging(TDMsgTAbort::_typeTag, asn1::ASTagging::tagsIMPLICIT);
    }
  };

  static const TaggingOptions _tagOptions;

  explicit TDTCMessage(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfChoice_T<5>(use_rule), _dVal(0)
  {
    construct();
  }
  explicit TDTCMessage(proto::TCMessage & use_val,
                       asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfChoice_T<5>(use_rule), _dVal(&use_val)
  {
    construct();
  }
  //
  ~TDTCMessage()
  { }

  void setValue(proto::TCMessage & use_val)
  {
    _dVal = &use_val;
    _chcDec.reset();
  }
};


}}}}

#endif /* __EYELINE_TCAP_PROTO_DEC_TCAP_MSG_HPP */

