/* ************************************************************************* *
 * TCAP Unidirectional Message Decoder.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_DEC_TUNIDIR_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
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
protected:
  proto::TMsgUnidir *  _dVal;
  /* -- */
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
  static const asn1::ASTag _typeTag; //[APPLICATION 1] IMPLICIT

  explicit TDMsgTUnidir(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfSequence_T<2>(_typeTag, asn1::ASTagging::tagsIMPLICIT, use_rule)
    , _dVal(0), _compPart(use_rule)
  {
    construct();
  }
  explicit TDMsgTUnidir(proto::TMsgUnidir & use_val,
                        asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfSequence_T<2>(_typeTag, asn1::ASTagging::tagsIMPLICIT, use_rule)
    , _dVal(&use_val), _compPart(use_rule)
  {
    construct();
  }
  //
  ~TDMsgTUnidir()
  { }

  void setValue(proto::TMsgUnidir & use_val)
  {
    _dVal = &use_val;
  }
};

}}}}

#endif/* __EYELINE_TCAP_PROTO_DEC_TUNIDIR_HPP */

