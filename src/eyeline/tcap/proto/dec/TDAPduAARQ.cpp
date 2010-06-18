#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/proto/dec/TDAPduAARQ.hpp"


namespace eyeline {
namespace tcap {
namespace proto {
namespace dec {
/* ************************************************************ *
 * Class TDAPduAARE implementation
 * ************************************************************ */
const asn1::ASTag TDAPduAARQ::_typeTag(asn1::ASTag::tagApplication, 0);

/* AARQ APdu is defined in EXPLICIT tagging environment as following:

AARQ-apdu ::= [APPLICATION 0] IMPLICIT SEQUENCE {
  protocol-version          ProtocolVersion DEFAULT {version1},
  application-context-name  ApplicationContext,
  user-information          UserInformation OPTIONAL
} */
void TDAPduAARQ::construct(void)
{
  asn1::ber::DecoderOfSequence_T<3>::setField(0, TDProtocolVersion::_typeTag, asn1::ber::EDAlternative::altOPTIONAL);
  asn1::ber::DecoderOfSequence_T<3>::setField(1, TDApplicationContext::_typeTag, asn1::ber::EDAlternative::altMANDATORY);
  asn1::ber::DecoderOfSequence_T<3>::setField(2, TDUserInformation::_typeTag, asn1::ber::EDAlternative::altOPTIONAL);
}

// ----------------------------------------
// -- DecoderOfStructAC interface methods
// ----------------------------------------
//If necessary, allocates optional element and initializes associated TypeDecoderAC
asn1::ber::TypeDecoderAC *
  TDAPduAARQ::prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */
{
  if (!_dVal) //assertion!!!
    throw smsc::util::Exception("tcap::proto::dec::TDAPduAARQ : value isn't set!");
  if (unique_idx > 2)
    throw smsc::util::Exception("tcap::proto::dec::TDAPduAARQ::prepareAlternative() : undefined UId");

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
  //if (unique_idx == 2) {}
  getUI()->setValue(_dVal->_usrInfo);
  return _pUI;
}

}}}}

