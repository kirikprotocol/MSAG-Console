/* ************************************************************************* *
 * TCAP Structured Dialogue Abort APDU decoder.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_DEC_ABRT_APDU_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROTO_DEC_ABRT_APDU_HPP

#include "eyeline/tcap/proto/TCStrDialogue.hpp"
#include "eyeline/tcap/proto/dec/TDUserInformation.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeSeq.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace dec {

/* ABRT APdu is defined in EXPLICIT tagging environment as following:

ABRT-apdu ::= [APPLICATION 4] IMPLICIT SEQUENCE {
  abort-source      [0] IMPLICIT ABRT-source,
  user-information  UserInformation OPTIONAL
}
*/
class TDAPduABRT : public asn1::ber::DecoderOfSequence_T<2> {
private:
  using asn1::ber::DecoderOfSequence_T<2>::setField;

  union {
    void *  _aligner;
    uint8_t _buf[sizeof(TDUserInformation)];
  } _memUI;

  TDUserInformation * _pUI; //OPTIONAL

protected:
  static const asn1::ASTag _fldTagAbrtSrc; //[0] IMPLICIT

  proto::TCPduABRT *            _dVal;
  asn1::ber::DecoderOfINTEGER   _abrtSrc;

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
  static const asn1::ASTag _typeTag; //[APPLICATION 4] IMPLICIT

  explicit TDAPduABRT(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleBER)
    : asn1::ber::DecoderOfSequence_T<2>(_typeTag, asn1::ASTagging::tagsIMPLICIT,
                                        TSGroupBER::getTSRule(use_rule))
    , _pUI(0), _dVal(0)
    , _abrtSrc(_fldTagAbrtSrc, asn1::ASTagging::tagsIMPLICIT, TSGroupBER::getTSRule(use_rule))
  {
    _memUI._aligner = 0;
    construct();
  }
  TDAPduABRT(proto::TCPduABRT & use_val,
            TSGroupBER::Rule_e use_rule = TSGroupBER::ruleBER)
    : asn1::ber::DecoderOfSequence_T<2>(_typeTag, asn1::ASTagging::tagsIMPLICIT,
                                        TSGroupBER::getTSRule(use_rule))
    , _pUI(0), _dVal(&use_val)
    , _abrtSrc(_fldTagAbrtSrc, asn1::ASTagging::tagsIMPLICIT, TSGroupBER::getTSRule(use_rule))
  {
    _memUI._aligner = 0;
    construct();
  }
  //
  ~TDAPduABRT()
  {
    clearUI();
  }

  void setValue(proto::TCPduABRT & use_val)
  {
    _dVal = &use_val;
    _seqDec.reset();
  }
};

}}}}

#endif /* __EYELINE_TCAP_PROTO_DEC_ABRT_APDU_HPP */

