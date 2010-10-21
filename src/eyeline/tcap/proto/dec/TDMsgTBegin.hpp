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
protected:
/* ----------------------------------------------- */
  proto::TMsgBegin *  _dVal;
  /* -- */
  TDOrigTransactionId _orgTrId;
  TDComponentPortion  _compPart;
  //Ooptionals:
  asn1::ber::DecoderProducer_T<TDDialoguePortion> _dlgPart;

/* ----------------------------------------------- */
  //Initializes ElementDecoder of this type;
  void construct(void);

  // ----------------------------------------
  // -- DecoderOfStructAC interface methods
  // ----------------------------------------
  //If necessary, allocates optional element and initializes associated TypeDecoderAC
  virtual TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */;

public:
  static const asn1::ASTag _typeTag; //[APPLICATION 2] IMPLICIT

  explicit TDMsgTBegin(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfSequence_T<3>(_typeTag, asn1::ASTagging::tagsIMPLICIT, use_rule)
    , _dVal(0), _orgTrId(use_rule), _compPart(use_rule)
  {
    construct();
  }
  explicit TDMsgTBegin(proto::TMsgBegin & use_val,
          asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfSequence_T<3>(_typeTag, asn1::ASTagging::tagsIMPLICIT, use_rule)
    , _dVal(&use_val), _orgTrId(use_rule), _compPart(use_rule)
  {
    construct();
  }
  //
  ~TDMsgTBegin()
  { }

  void setValue(proto::TMsgBegin & use_val)
  {
    _dVal = &use_val;
    _seqDec.reset();
  }
};

}}}}

#endif/* __EYELINE_TCAP_PROTO_DEC_TBEGIN_HPP */

