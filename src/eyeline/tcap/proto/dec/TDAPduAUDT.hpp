/* ************************************************************************* *
 * TCAP UniDialogue Datagramm APDU decoder.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_DEC_AUDT_APDU_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROTO_DEC_AUDT_APDU_HPP

#include "eyeline/tcap/proto/TCUniDialogue.hpp"

#include "eyeline/tcap/proto/dec/TDProtocolVersion.hpp"
#include "eyeline/tcap/proto/dec/TDApplicationContext.hpp"
#include "eyeline/tcap/proto/dec/TDUserInformation.hpp"

#include "eyeline/asn1/BER/rtdec/DecodeSeq.hpp"
#include "eyeline/asn1/BER/rtdec/DecoderProducer.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace dec {

/* AUDT APdu is defined in EXPLICIT tagging environment as following:

AUDT-apdu ::= [APPLICATION 0] IMPLICIT SEQUENCE {
  protocol-version          ProtocolVersion DEFAULT {version1},
  application-context-name  ApplicationContext,
  user-information          UserInformation OPTIONAL
}
*/
class TDAPduAUDT : public asn1::ber::DecoderOfSequence_T<3> {
protected:
  proto::TCPduAUDT *    _dVal;
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

public:
  static const asn1::ASTag _typeTag; //[APPLICATION 0] IMPLICIT

  explicit TDAPduAUDT(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfSequence_T<3>(_typeTag, asn1::ASTagging::tagsIMPLICIT, use_rule)
    , _dVal(0), _protoVer(use_rule), _appCtx(use_rule)
  {
    construct();
  }
  explicit TDAPduAUDT(proto::TCPduAUDT & use_val,
                      asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfSequence_T<3>(_typeTag, asn1::ASTagging::tagsIMPLICIT, use_rule)
    , _dVal(&use_val), _protoVer(use_rule), _appCtx(use_rule)
  {
    construct();
  }
  //
  ~TDAPduAUDT()
  { }

  void setValue(proto::TCPduAUDT & use_val)
  {
    _dVal = &use_val;
    _seqDec.reset();
  }
};

}}}}

#endif /* __EYELINE_TCAP_PROTO_DEC_AUDT_APDU_HPP */

