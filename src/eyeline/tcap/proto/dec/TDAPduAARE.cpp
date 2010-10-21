#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/tcap/proto/dec/TDAPduAARE.hpp"


namespace eyeline {
namespace tcap {
namespace proto {
namespace dec {

/* ************************************************************ *
 * Class TDAPduAARE::TDResultDiagnosticField implementation
 * ************************************************************ */
const asn1::ASTag
  TDAPduAARE::TDResultDiagnosticField::_typeTag(asn1::ASTag::tagContextSpecific, 3);
const asn1::ASTag
  TDAPduAARE::TDResultDiagnosticField::_tagUser(asn1::ASTag::tagContextSpecific, 1);
const asn1::ASTag
  TDAPduAARE::TDResultDiagnosticField::_tagPrvd(asn1::ASTag::tagContextSpecific, 2);

//ResultSourceDiagnosticField ::= [3] CHOICE {
//  dialogue-service-user [1]  INTEGER { null(0), no-reason-given(1),
//                                       application-context-name-not-supported(2) },
//  dialogue-service-provider [2]  INTEGER { null(0), no-reason-given(1),
//                                           no-common-dialogue-portion(2) }
//}

// ----------------------------------------
// -- DecoderOfChoiceAC interface methods
// ----------------------------------------
//Allocates alternative data structure and initializes associated TypeDecoderAC
asn1::ber::TypeDecoderAC *
  TDAPduAARE::TDResultDiagnosticField::prepareAlternative(uint16_t unique_idx)
  /*throw(throw(std::exception)) */
{
  if (!_dVal) //assertion!!!
    throw smsc::util::Exception("tcap::proto::dec::TDResultDiagnosticField : value isn't set!");
  if (unique_idx > 2)
    throw smsc::util::Exception("tcap::proto::dec::TDResultDiagnosticField::prepareAlternative() : undefined UId");

  _pDec.setValue(_dVal->_sdResult);
  return &_pDec;
}
//Perfoms actions finalizing alternative decoding
void TDAPduAARE::TDResultDiagnosticField::markDecodedAlternative(uint16_t unique_idx)
  /*throw(throw(std::exception)) */
{
  _dVal->_sdKind = static_cast<AssociateSourceDiagnostic::SourceKind_e>(unique_idx);
}

/* ************************************************************ *
 * Class TDAPduAARE implementation
 * ************************************************************ */
const asn1::ASTag TDAPduAARE::_typeTag(asn1::ASTag::tagApplication, 1);
const asn1::ASTag TDAPduAARE::_f2Tag(asn1::ASTag::tagContextSpecific, 2);

/* AARE APdu is defined in EXPLICIT tagging environment as following:

AARE-apdu ::= [APPLICATION 1] IMPLICIT SEQUENCE {
  protocol-version          ProtocolVersion DEFAULT {version1},
  application-context-name  ApplicationContext,
  result                    [2]  Associate-result,
  result-source-diagnostic  [3]  Associate-source-diagnostic,
  user-information          UserInformation OPTIONAL
}*/
//Initializes ElementDecoder of this type
void TDAPduAARE::construct(void)
{
  setField(0, TDProtocolVersion::_typeTag, asn1::ber::EDAlternative::altOPTIONAL);
  setField(1, TDApplicationContext::_typeTag, asn1::ber::EDAlternative::altMANDATORY);
  setField(2, _f2Tag, asn1::ASTagging::tagsEXPLICIT, asn1::ber::EDAlternative::altMANDATORY);
  setField(3, TDResultDiagnosticField::_typeTag, asn1::ber::EDAlternative::altMANDATORY);
  setField(4, TDUserInformation::_typeTag, asn1::ber::EDAlternative::altOPTIONAL);
}

// ----------------------------------------
// -- DecoderOfStructAC interface methods
// ----------------------------------------
//If necessary, allocates optional element and initializes associated TypeDecoderAC
asn1::ber::TypeDecoderAC *
  TDAPduAARE::prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */
{
  if (!_dVal) //assertion!!!
    throw smsc::util::Exception("tcap::proto::dec::TDAPduAARE : value isn't set!");
  if (unique_idx > 4)
    throw smsc::util::Exception("tcap::proto::dec::TDAPduAARE::prepareAlternative() : undefined UId");

  if (!unique_idx) {
    _protoVer.setValue(_dVal->_protoVer);
    return &_protoVer;
  }
  if (unique_idx == 1) {
    if (_dVal->_protoVer.empty()) //set default protocol version
      _dVal->_protoVer = proto::_dfltProtocolVersion;
    _appCtx.setValue(_dVal->_acId);
    return &_appCtx;
  }
  if (unique_idx == 2) {
    _ascResult.setValue(_dVal->_result);
    return &_ascResult;
  }
  if (unique_idx == 3) {
    _ascDiagn.setValue(_dVal->_diagnostic);
    return &_ascDiagn;
  }
  //if (unique_idx == 4) {}
  _pUI.init(getTSRule()).setValue(_dVal->_usrInfo);
  return _pUI.get();
}

}}}}

