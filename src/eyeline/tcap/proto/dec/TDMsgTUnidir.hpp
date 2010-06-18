/* ************************************************************************* *
 * TCAP Unidirectional Message Decoder.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_DEC_TUNIDIR_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROTO_DEC_TUNIDIR_HPP

#include "eyeline/tcap/proto/TMsgUnidir.hpp"
#include "eyeline/tcap/proto/dec/TDComponentPortion.hpp"
#include "eyeline/tcap/proto/dec/TDDialoguePortion.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace dec {

/* Unidirectional message is defined in IMPLICIT tagging environment as follow:
Unidirectional ::= [APPLICATION 1] SEQUENCE {
    dialoguePortion  DialoguePortion OPTIONAL,
    components       ComponentPortion
}

NOTE: When the dialoguePortion is present it may contain TCAP unidialogue AUDT-APdu
*/
class TDMsgTUnidir : public asn1::ber::DecoderOfSequence_T<2> {
private:
  using asn1::ber::DecoderOfSequence_T<2>::setField;

  union {
    void *  _aligner;
    uint8_t _buf[sizeof(TDDialoguePortion)];
  } _memDlg;

protected:
  proto::TMsgUnidir *  _dVal;
  /* -- */
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
  static const asn1::ASTag _typeTag; //[APPLICATION 1] IMPLICIT

  explicit TDMsgTUnidir(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleBER)
    : asn1::ber::DecoderOfSequence_T<2>(_typeTag, asn1::ASTagging::tagsIMPLICIT,
                                        TSGroupBER::getTSRule(use_rule))
    , _dVal(0), _compPart(use_rule), _dlgPart(0)
  {
    _memDlg._aligner = 0;
    construct();
  }
  TDMsgTUnidir(proto::TMsgUnidir & use_val,
          TSGroupBER::Rule_e use_rule = TSGroupBER::ruleBER)
    : asn1::ber::DecoderOfSequence_T<2>(_typeTag, asn1::ASTagging::tagsIMPLICIT,
                                        TSGroupBER::getTSRule(use_rule))
    , _dVal(&use_val), _compPart(use_rule), _dlgPart(0)
  {
    _memDlg._aligner = 0;
    construct();
  }
  //
  ~TDMsgTUnidir()
  {
    cleanUp();
  }

  void setValue(proto::TMsgUnidir & use_val)
  {
    _dVal = &use_val;
    _seqDec.reset();
  }
};

}}}}

#endif/* __EYELINE_TCAP_PROTO_DEC_TUNIDIR_HPP */

