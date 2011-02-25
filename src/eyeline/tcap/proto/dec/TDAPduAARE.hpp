/* ************************************************************************* *
 * TCAP Structured Dialogue Response APDU decoder.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_DNC_AARE_APDU_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_TCAP_PROTO_DNC_AARE_APDU_HPP

#include "eyeline/tcap/proto/TCStrDialogue.hpp"

#include "eyeline/tcap/proto/dec/TDProtocolVersion.hpp"
#include "eyeline/tcap/proto/dec/TDApplicationContext.hpp"
#include "eyeline/tcap/proto/dec/TDUserInformation.hpp"

#include "eyeline/asn1/BER/rtdec/DecodeChoice.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeSeq.hpp"
#include "eyeline/asn1/BER/rtdec/DecoderProducer.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace dec {

/* AARE APdu is defined in EXPLICIT tagging environment as following:

AARE-apdu ::= [APPLICATION 1] IMPLICIT SEQUENCE {
  protocol-version          ProtocolVersion DEFAULT {version1},
  application-context-name  ApplicationContext,
  result                    [2]  Associate-result,
  result-source-diagnostic  [3]  Associate-source-diagnostic,
  user-information          UserInformation OPTIONAL
}
*/
class TDAPduAARE : public asn1::ber::DecoderOfSequence_T<5> {
protected:
  static const asn1::ASTag _f2Tag; //[2] EXPLICIT

  //ResultSourceDiagnosticField ::= [3] CHOICE {
  //  dialogue-service-user [1]  INTEGER { null(0), no-reason-given(1),
  //                                       application-context-name-not-supported(2) },
  //  dialogue-service-provider [2]  INTEGER { null(0), no-reason-given(1),
  //                                           no-common-dialogue-portion(2) }
  //}
  class TDResultDiagnosticField : public asn1::ber::DecoderOfChoice_T<2> {
  private:
    using asn1::ber::DecoderOfChoice_T<2>::setAlternative;

    static const asn1::ASTag _tagUser; //[1] EXPLICIT
    static const asn1::ASTag _tagPrvd;  //[2] EXPLICIT

    asn1::ber::DecoderOfINTEGER   _pDec;
    AssociateSourceDiagnostic *   _dVal;

  protected:
    // ----------------------------------------
    // -- DecoderOfChoiceAC interface methods
    // ----------------------------------------
    //Allocates alternative data structure and initializes associated TypeDecoderAC
    virtual TypeDecoderAC * prepareAlternative(uint16_t unique_idx)
      /*throw(throw(std::exception)) */;
    //Perfoms actions finalizing alternative decoding
    virtual void markDecodedAlternative(uint16_t unique_idx)
      /*throw(throw(std::exception)) */;

  public:
    static const asn1::ASTag _typeTag; //[3] EXPLICIT

    explicit TDResultDiagnosticField(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
      : asn1::ber::DecoderOfChoice_T<2>(_typeTag, asn1::ASTagging::tagsEXPLICIT, use_rule)
      , _pDec(use_rule), _dVal(0)
    {
      asn1::ber::DecoderOfChoice_T<2>::setAlternative(0, _tagUser, asn1::ASTagging::tagsEXPLICIT);
      asn1::ber::DecoderOfChoice_T<2>::setAlternative(1, _tagPrvd, asn1::ASTagging::tagsEXPLICIT);
    }
    ~TDResultDiagnosticField()
    { }

    void setValue(proto::AssociateSourceDiagnostic & use_val)
    {
      _dVal = &use_val;
    }
  };

  /* ----------------------------------------------- */
  proto::TCPduAARE *            _dVal;
  /* -- */
  TDProtocolVersion             _protoVer;
  TDApplicationContext          _appCtx;
  asn1::ber::DecoderOfINTEGER   _ascResult;
  TDResultDiagnosticField       _ascDiagn;
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
  static const asn1::ASTag _typeTag; //[APPLICATION 1] IMPLICIT

  explicit TDAPduAARE(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfSequence_T<5>(_typeTag, asn1::ASTagging::tagsIMPLICIT, use_rule)
    , _dVal(0), _protoVer(use_rule), _appCtx(use_rule)
    , _ascResult(use_rule), _ascDiagn(use_rule)
  {
    construct();
  }
  explicit TDAPduAARE(proto::TCPduAARE & use_val,
                      asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfSequence_T<5>(_typeTag, asn1::ASTagging::tagsIMPLICIT, use_rule)
    , _dVal(&use_val), _protoVer(use_rule), _appCtx(use_rule)
    , _ascResult(use_rule), _ascDiagn(use_rule)
  {
    construct();
  }
//
  ~TDAPduAARE()
  { }

  void setValue(proto::TCPduAARE & use_val)
  {
    _dVal = &use_val;
  }
};

}}}}

#endif /* __EYELINE_TCAP_PROTO_DEC_AARE_APDU_HPP */

