/* ************************************************************************* *
 *  TCAP Structured Dialogue Request APDU decoder.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_DEC_AARQ_APDU_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_TCAP_PROTO_DEC_AARQ_APDU_HPP

#include "eyeline/tcap/proto/TCStrDialogue.hpp"

#include "eyeline/tcap/proto/dec/TDProtocolVersion.hpp"
#include "eyeline/tcap/proto/dec/TDApplicationContext.hpp"
#include "eyeline/tcap/proto/dec/TDUserInformation.hpp"

#include "eyeline/asn1/BER/rtdec/DecodeSeq.hpp"
#include "eyeline/asn1/BER/rtdec/DecoderProducer.hpp"

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
protected:
  proto::TCPduAARQ *    _dVal;
  /* ----------------------------------------------- */
  TDProtocolVersion     _protoVer;
  TDApplicationContext  _appCtx;
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
  //Performs actions upon successfull optional element decoding
  virtual void markDecodedOptional(uint16_t unique_idx) /*throw() */ { return; }

public:
  static const asn1::ASTag _typeTag; //[APPLICATION 0] IMPLICIT

  explicit TDAPduAARQ(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfSequence_T<3>(_typeTag, asn1::ASTagging::tagsIMPLICIT, use_rule)
    , _dVal(0), _protoVer(use_rule), _appCtx(use_rule)
  {
    construct();
  }
  explicit TDAPduAARQ(proto::TCPduAARQ & use_val,
                      asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfSequence_T<3>(_typeTag, asn1::ASTagging::tagsIMPLICIT, use_rule)
    , _dVal(&use_val), _protoVer(use_rule), _appCtx(use_rule)
  {
    construct();
  }
  //
  ~TDAPduAARQ()
  { }

  void setValue(proto::TCPduAARQ & use_val)
  {
    _dVal = &use_val;
  }
};


}}}}

#endif /* __EYELINE_TCAP_PROTO_DEC_AARQ_APDU_HPP */

