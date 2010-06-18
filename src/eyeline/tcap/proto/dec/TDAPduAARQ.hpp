/* ************************************************************************* *
 *  TCAP Structured Dialogue Request APDU decoder.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_DEC_AARQ_APDU_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROTO_DEC_AARQ_APDU_HPP

#include "eyeline/tcap/proto/TCStrDialogue.hpp"
#include "eyeline/tcap/proto/dec/TDProtocolVersion.hpp"
#include "eyeline/tcap/proto/dec/TDApplicationContext.hpp"
#include "eyeline/tcap/proto/dec/TDUserInformation.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeSeq.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace dec {

/* AARQ APdu is defined in EXPLICIT tagging environment as following:

AARQ-apdu ::= [APPLICATION 0] IMPLICIT SEQUENCE {
  protocol-version          ProtocolVersion DEFAULT {version1},
  application-context-name  ApplicationContext,
  user-information          UserInformation OPTIONAL
} */
class TDAPduAARQ : public asn1::ber::DecoderOfSequence_T<3> {
private:
  using asn1::ber::DecoderOfSequence_T<3>::setField;

  union {
    void *  _aligner;
    uint8_t _buf[sizeof(TDUserInformation)];
  } _memUI;

  TDUserInformation * _pUI; //OPTIONAL

protected:
  proto::TCPduAARQ *    _dVal;
  /* ----------------------------------------------- */
  TDProtocolVersion     _protoVer;
  TDApplicationContext  _appCtx;
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
  static const asn1::ASTag _typeTag; //[APPLICATION 0] IMPLICIT

  explicit TDAPduAARQ(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleBER)
    : asn1::ber::DecoderOfSequence_T<3>(_typeTag, asn1::ASTagging::tagsIMPLICIT,
                                         TSGroupBER::getTSRule(use_rule))
    , _dVal(0), _pUI(0), _protoVer(use_rule), _appCtx(use_rule)
  {
    _memUI._aligner = 0;
    construct();
  }
  TDAPduAARQ(proto::TCPduAARQ & use_val,
            TSGroupBER::Rule_e use_rule = TSGroupBER::ruleBER)
    : asn1::ber::DecoderOfSequence_T<3>(_typeTag, asn1::ASTagging::tagsIMPLICIT,
                                         TSGroupBER::getTSRule(use_rule))
    , _dVal(&use_val), _pUI(0), _protoVer(use_rule), _appCtx(use_rule)
  {
    _memUI._aligner = 0;
    construct();
  }
  //
  ~TDAPduAARQ()
  {
    clearUI();
  }

  void setValue(proto::TCPduAARQ & use_val)
  {
    _dVal = &use_val;
    _seqDec.reset();
  }
};


}}}}

#endif /* __EYELINE_TCAP_PROTO_DEC_AARQ_APDU_HPP */

