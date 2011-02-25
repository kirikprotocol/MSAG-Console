/* ************************************************************************* *
 * TCAP Continue Message Decoder.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_DEC_TCONTINUE_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_TCAP_PROTO_DEC_TCONTINUE_HPP

#include "eyeline/tcap/proto/TMsgContinue.hpp"

#include "eyeline/tcap/proto/dec/TDTransactionId.hpp"
#include "eyeline/tcap/proto/dec/TDComponentPortion.hpp"
#include "eyeline/tcap/proto/dec/TDDialoguePortion.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace dec {

/* Continue message is defined in IMPLICIT tagging environment as follow:
Continue ::= [APPLICATION 5] SEQUENCE {
    otid             OrigTransactionID,
    dtid             DestTransactionID,
    dialoguePortion  DialoguePortion OPTIONAL,
    components       ComponentPortion
}
-- NOTE: When the dialoguePortion is present it may contain TCAP structured
--       dialogue AARE-APdu
*/
class TDMsgTContinue : public asn1::ber::DecoderOfSequence_T<4> {
protected:
  proto::TMsgContinue *  _dVal;
  /* -- */
  TDOrigTransactionId   _orgTrId;
  TDDestTransactionId   _dstTrId;
  TDComponentPortion    _compPart;
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
  static const asn1::ASTag _typeTag; //[APPLICATION 5] IMPLICIT

  explicit TDMsgTContinue(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfSequence_T<4>(_typeTag, asn1::ASTagging::tagsIMPLICIT, use_rule)
    , _dVal(0), _orgTrId(use_rule), _dstTrId(use_rule), _compPart(use_rule)
  {
    construct();
  }
  explicit TDMsgTContinue(proto::TMsgContinue & use_val,
                 asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfSequence_T<4>(_typeTag, asn1::ASTagging::tagsIMPLICIT, use_rule)
    , _dVal(&use_val), _orgTrId(use_rule), _dstTrId(use_rule), _compPart(use_rule)
  {
    construct();
  }
  //
  ~TDMsgTContinue()
  { }

  void setValue(proto::TMsgContinue & use_val)
  {
    _dVal = &use_val;
  }
};

}}}}

#endif/* __EYELINE_TCAP_PROTO_DEC_TCONTINUE_HPP */

