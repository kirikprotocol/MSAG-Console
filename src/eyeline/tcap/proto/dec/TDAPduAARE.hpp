/* ************************************************************************* *
 * TCAP Structured Dialogue Response APDU decoder.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_DNC_AARE_APDU_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROTO_DNC_AARE_APDU_HPP

#include "eyeline/tcap/proto/TCStrDialogue.hpp"
#include "eyeline/tcap/proto/dec/TDProtocolVersion.hpp"
#include "eyeline/tcap/proto/dec/TDApplicationContext.hpp"
#include "eyeline/tcap/proto/dec/TDUserInformation.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeChoice.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeSeq.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace dec {

/* AARE APdu is defined in EXPLICIT tagging environment as following:

AARE-apdu ::= [APPLICATION 1] IMPLICIT SEQUENCE {
  protocol-version          ProtocolVersion DEFAULT {version1},
  application-context-name  ApplicationContext,
  result                    [2]  Associate-result,
  result-source-diagnostic  [3]  Associate-source-diagnostic,
  user-information          UserInformation OPTIONAL
}
*/
class TDAPduAARE : public asn1::ber::DecoderOfSequence_T<5> {
private:
  using asn1::ber::DecoderOfSequence_T<5>::setField;

  union {
    void *  _aligner;
    uint8_t _buf[sizeof(TDUserInformation)];
  } _memUI;

  TDUserInformation * _pUI; //OPTIONAL

protected:
  //AssociateResultField ::= [2] INTEGER {accepted(0), reject-permanent(1)}
  class TDResultField : public asn1::ber::DecoderOfINTEGER {
  public:
    static const asn1::ASTag _typeTag; //[2] EXPLICIT

    explicit TDResultField(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
      : asn1::ber::DecoderOfINTEGER(_typeTag, asn1::ASTagging::tagsEXPLICIT,
                                    TSGroupBER::getTSRule(use_rule))
    { }
    ~TDResultField()
    { }
  };

  //ResultSourceDiagnosticField ::= [3] CHOICE {
  //  dialogue-service-user [1]  INTEGER { null(0), no-reason-given(1),
  //                                       application-context-name-not-supported(2) },
  //  dialogue-service-provider [2]  INTEGER { null(0), no-reason-given(1),
  //                                           no-common-dialogue-portion(2) }
  //}
  class TDResultDiagnosticField : public asn1::ber::DecoderOfChoice_T<2> {
  private:
    using asn1::ber::DecoderOfChoice_T<2>::setAlternative;

    static const asn1::ASTag _tagUser; //[1] EXPLICIT
    static const asn1::ASTag _tagPrvd;  //[2] EXPLICIT

    asn1::ber::DecoderOfINTEGER   _pDec;
    AssociateSourceDiagnostic *   _dVal;

  protected:
    // ----------------------------------------
    // -- DecoderOfChoiceAC interface methods
    // ----------------------------------------
    //Allocates alternative data structure and initializes associated TypeDecoderAC
    virtual TypeDecoderAC * prepareAlternative(uint16_t unique_idx)
      /*throw(throw(std::exception)) */;
    //Perfoms actions finalizing alternative decoding
    virtual void markDecodedAlternative(uint16_t unique_idx)
      /*throw(throw(std::exception)) */;

  public:
    static const asn1::ASTag _typeTag; //[3] EXPLICIT

    explicit TDResultDiagnosticField(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleBER)
      : asn1::ber::DecoderOfChoice_T<2>(_typeTag, asn1::ASTagging::tagsEXPLICIT,
                                   TSGroupBER::getTSRule(use_rule))
      , _pDec(TSGroupBER::getTSRule(use_rule)), _dVal(0)
    {
      asn1::ber::DecoderOfChoice_T<2>::setAlternative(0, _tagUser, asn1::ASTagging::tagsEXPLICIT);
      asn1::ber::DecoderOfChoice_T<2>::setAlternative(1, _tagPrvd, asn1::ASTagging::tagsEXPLICIT);
    }
    ~TDResultDiagnosticField()
    { }

    void setValue(proto::AssociateSourceDiagnostic & use_val)
    {
      _dVal = &use_val;
      _chcDec.reset();
    }
  };

  /* ----------------------------------------------- */
  proto::TCPduAARE *        _dVal;
  TDProtocolVersion         _protoVer;
  TDApplicationContext      _appCtx;
  TDResultField             _ascResult;
  TDResultDiagnosticField   _ascDiagn;

/* ----------------------------------------------- */
  TDUserInformation * getUI(void)
  {
    if (!_pUI)
      _pUI = new (_memUI._buf)TDUserInformation(getVALRule());
    return _pUI;
  }
  void clearUI(void)
  {
    if (_pUI)
      _pUI->~TDUserInformation();
  }

  //Initializes ElementDecoder of this type
  void construct(void);

  // ----------------------------------------
  // -- DecoderOfStructAC interface methods
  // ----------------------------------------
  //If necessary, allocates optional element and initializes associated TypeDecoderAC
  virtual TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */;
  //Performs actions upon successfull optional element decoding
  virtual void markDecodedOptional(uint16_t unique_idx) /*throw() */ { return; }

public:
  static const asn1::ASTag _typeTag; //[APPLICATION 1] IMPLICIT

  explicit TDAPduAARE(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleBER)
    : asn1::ber::DecoderOfSequence_T<5>(_typeTag, asn1::ASTagging::tagsIMPLICIT,
                                        TSGroupBER::getTSRule(use_rule))
    , _pUI(0), _dVal(0), _protoVer(use_rule), _appCtx(use_rule)
    , _ascResult(use_rule), _ascDiagn(use_rule)
  {
    _memUI._aligner = 0;
    construct();
  }
  TDAPduAARE(proto::TCPduAARE & use_val,
            TSGroupBER::Rule_e use_rule = TSGroupBER::ruleBER)
    : asn1::ber::DecoderOfSequence_T<5>(_typeTag, asn1::ASTagging::tagsIMPLICIT,
                                        TSGroupBER::getTSRule(use_rule))
    , _pUI(0), _dVal(&use_val), _protoVer(use_rule), _appCtx(use_rule)
    , _ascResult(use_rule), _ascDiagn(use_rule)
  {
    _memUI._aligner = 0;
    construct();
  }
//
  ~TDAPduAARE()
  {
    clearUI();
  }

  void setValue(proto::TCPduAARE & use_val)
  {
    _dVal = &use_val;
    _seqDec.reset();
  }
};

}}}}

#endif /* __EYELINE_TCAP_PROTO_DEC_AARE_APDU_HPP */

