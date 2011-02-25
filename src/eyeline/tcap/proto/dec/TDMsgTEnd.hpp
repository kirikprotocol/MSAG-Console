/* ************************************************************************* *
 * TCAP End Message Decoder.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_DEC_TEND_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_TCAP_PROTO_DEC_TEND_HPP

#include "eyeline/tcap/proto/TMsgEnd.hpp"

#include "eyeline/tcap/proto/dec/TDTransactionId.hpp"
#include "eyeline/tcap/proto/dec/TDComponentPortion.hpp"
#include "eyeline/tcap/proto/dec/TDDialoguePortion.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace dec {

/* End message is defined in IMPLICIT tagging environment as follow:
End ::= [APPLICATION 4] SEQUENCE {
    dtid             DestTransactionID,
    dialoguePortion  DialoguePortion OPTIONAL,
    components       ComponentPortion
}
-- NOTE: When the dialoguePortion is present it may contain TCAP structured
--       dialogue AARQ-APdu or ABRT-APdu
*/
class TDMsgTEnd : public asn1::ber::DecoderOfSequence_T<3> {
protected:
  proto::TMsgEnd *  _dVal;
  /* -- */
  TDDestTransactionId _dstTrId;
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

  explicit TDMsgTEnd(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfSequence_T<3>(_typeTag, asn1::ASTagging::tagsIMPLICIT, use_rule)
    , _dVal(0), _dstTrId(use_rule), _compPart(use_rule)
  {
    construct();
  }
  explicit TDMsgTEnd(proto::TMsgEnd & use_val,
            asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfSequence_T<3>(_typeTag, asn1::ASTagging::tagsIMPLICIT, use_rule)
    , _dVal(&use_val), _dstTrId(use_rule), _compPart(use_rule)
  {
    construct();
  }
  //
  ~TDMsgTEnd()
  { }

  void setValue(proto::TMsgEnd & use_val)
  {
    _dVal = &use_val;
  }
};

}}}}

#endif/* __EYELINE_TCAP_PROTO_DEC_TEND_HPP */

