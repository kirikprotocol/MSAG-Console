/* ************************************************************************* *
 * TCAP Begin Message Decoder.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_DEC_TBEGIN_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROTO_DEC_TBEGIN_HPP

#include "eyeline/tcap/proto/TMsgBegin.hpp"
#include "eyeline/tcap/proto/dec/TDTransactionId.hpp"
#include "eyeline/tcap/proto/dec/TDComponentPortion.hpp"
#include "eyeline/tcap/proto/dec/TDDialoguePortion.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace dec {

/* Begin message is defined in IMPLICIT tagging environment as follow:
Begin ::= [APPLICATION 2] SEQUENCE {
    otid             OrigTransactionID,
    dialoguePortion  DialoguePortion OPTIONAL,
    components       ComponentPortion
}
-- NOTE: When the dialoguePortion is present it may contain TCAP structured
--       dialogue AARQ-APdu
*/
class TDMsgTBegin : public asn1::ber::DecoderOfSequence_T<3> {
private:
  using asn1::ber::DecoderOfSequence_T<3>::setField;

  union {
    void *  _aligner;
    uint8_t _buf[sizeof(TDDialoguePortion)];
  } _memDlg;

protected:
  proto::TMsgBegin *  _dVal;
  /* -- */
  TDOrigTransactionId _orgTrId;
  TDComponentPortion  _compPart;
  TDDialoguePortion * _dlgPart; //optional

  void cleanUp(void)
  {
    if (_dlgPart) {
      _dlgPart->~TDDialoguePortion();
      _dlgPart = NULL;
    }
  }

  //Initializes ElementDecoder of this type;
  void construct(void);

  // ----------------------------------------
  // -- DecoderOfStructAC interface methods
  // ----------------------------------------
  //If necessary, allocates optional element and initializes associated TypeDecoderAC
  virtual TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */;
  //Performs actions upon successfull optional element decoding
  virtual void markDecodedOptional(uint16_t unique_idx) /*throw() */ { return; }

public:
  static const asn1::ASTag _typeTag; //[APPLICATION 2] IMPLICIT

  explicit TDMsgTBegin(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleBER)
    : asn1::ber::DecoderOfSequence_T<3>(_typeTag, asn1::ASTagging::tagsIMPLICIT,
                                        TSGroupBER::getTSRule(use_rule))
    , _dVal(0), _orgTrId(use_rule), _compPart(use_rule), _dlgPart(0)
  {
    _memDlg._aligner = 0;
    construct();
  }
  TDMsgTBegin(proto::TMsgBegin & use_val,
          TSGroupBER::Rule_e use_rule = TSGroupBER::ruleBER)
    : asn1::ber::DecoderOfSequence_T<3>(_typeTag, asn1::ASTagging::tagsIMPLICIT,
                                        TSGroupBER::getTSRule(use_rule))
    , _dVal(&use_val), _orgTrId(use_rule), _compPart(use_rule), _dlgPart(0)
  {
    _memDlg._aligner = 0;
    construct();
  }
  //
  ~TDMsgTBegin()
  {
    cleanUp();
  }

  void setValue(proto::TMsgBegin & use_val)
  {
    _dVal = &use_val;
    _seqDec.reset();
  }
};

}}}}

#endif/* __EYELINE_TCAP_PROTO_DEC_TBEGIN_HPP */

