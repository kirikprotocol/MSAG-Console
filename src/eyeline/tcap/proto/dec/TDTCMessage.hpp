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

using eyeline::asn1::ber::TSGroupBER;

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
private:
  using asn1::ber::DecoderOfChoice_T<5>::setAlternative;

  union {
    void *    _aligner;
    uint8_t   _buf[eyeline::util::MaxSizeOf5_T<TDMsgTUnidir, TDMsgTBegin,
                              TDMsgTEnd, TDMsgTContinue, TDMsgTAbort>::VALUE];
  } _memMsg;

protected:
  proto::TCMessage * _dVal;
  union {
    asn1::ber::TypeDecoderAC * _any;
    TDMsgTUnidir *    _unidir;
    TDMsgTBegin *     _begin;
    TDMsgTEnd *       _end;
    TDMsgTContinue *  _cont;
    TDMsgTAbort *     _abort;
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

  explicit TDTCMessage(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleBER)
    : asn1::ber::DecoderOfChoice_T<5>(TSGroupBER::getTSRule(use_rule))
    , _dVal(0)
  {
    construct();
    _memMsg._aligner = 0;
    _pDec._any = NULL;
  }
  TDTCMessage(proto::TCMessage & use_val,
              TSGroupBER::Rule_e use_rule = TSGroupBER::ruleBER)
    : asn1::ber::DecoderOfChoice_T<5>(TSGroupBER::getTSRule(use_rule))
    , _dVal(&use_val)
  {
    construct();
    _memMsg._aligner = 0;
    _pDec._any = NULL;
  }
  //
  ~TDTCMessage()
  {
    cleanUp();
  }

  void setValue(proto::TCMessage & use_val)
  {
    _dVal = &use_val;
    _chcDec.reset();
  }
};


}}}}

#endif /* __EYELINE_TCAP_PROTO_DEC_TCAP_MSG_HPP */

