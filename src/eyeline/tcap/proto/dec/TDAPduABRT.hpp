/* ************************************************************************* *
 * TCAP Structured Dialogue Abort APDU decoder.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_DEC_ABRT_APDU_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_TCAP_PROTO_DEC_ABRT_APDU_HPP

#include "eyeline/tcap/proto/TCStrDialogue.hpp"

#include "eyeline/tcap/proto/dec/TDUserInformation.hpp"

#include "eyeline/asn1/BER/rtdec/DecodeSeq.hpp"
#include "eyeline/asn1/BER/rtdec/DecoderProducer.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace dec {

/* ABRT APdu is defined in EXPLICIT tagging environment as following:

ABRT-apdu ::= [APPLICATION 4] IMPLICIT SEQUENCE {
  abort-source      [0] IMPLICIT ABRT-source,
  user-information  UserInformation OPTIONAL
} */
class TDAPduABRT : public asn1::ber::DecoderOfSequence_T<2> {
protected:
  static const asn1::ASTag _fldTagAbrtSrc; //[0] IMPLICIT

  proto::TCPduABRT *            _dVal;
/* ----------------------------------------------- */
  asn1::ber::DecoderOfINTEGER   _abrtSrc;
  //Optionals:
  asn1::ber::DecoderProducer_T<TDUserInformation> _pUI;
/* ----------------------------------------------- */

  //Initializes ElementDecoder of this type
  void construct(void);

  // ----------------------------------------
  // -- DecoderOfStructAC interface methods
  // ----------------------------------------
  //If necessary, allocates optional element and initializes associated TypeDecoderAC
  virtual TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */;

public:
  static const asn1::ASTag _typeTag; //[APPLICATION 4] IMPLICIT

  explicit TDAPduABRT(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfSequence_T<2>(_typeTag, asn1::ASTagging::tagsIMPLICIT, use_rule)
    , _dVal(0)
    , _abrtSrc(_fldTagAbrtSrc, asn1::ASTagging::tagsIMPLICIT, use_rule)
  {
    construct();
  }
  explicit TDAPduABRT(proto::TCPduABRT & use_val,
                      asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfSequence_T<2>(_typeTag, asn1::ASTagging::tagsIMPLICIT, use_rule)
    , _dVal(&use_val)
    , _abrtSrc(_fldTagAbrtSrc, asn1::ASTagging::tagsIMPLICIT, use_rule)
  {
    construct();
  }
  //
  ~TDAPduABRT()
  { }

  void setValue(proto::TCPduABRT & use_val)
  {
    _dVal = &use_val;
  }
};

}}}}

#endif /* __EYELINE_TCAP_PROTO_DEC_ABRT_APDU_HPP */

