#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/proto/dec/TDAPduABRT.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace dec {
/* ************************************************************ *
 * Class TDAPduABRT implementation
 * ************************************************************ */
const asn1::ASTag
  TDAPduABRT::_typeTag(asn1::ASTag::tagApplication, 4);

const asn1::ASTag
  TDAPduABRT::_fldTagAbrtSrc(asn1::ASTag::tagContextSpecific, 0);



/* ABRT APdu is defined in EXPLICIT tagging environment as following:

ABRT-apdu ::= [APPLICATION 4] IMPLICIT SEQUENCE {
  abort-source      [0] IMPLICIT ABRT-source,
  user-information  UserInformation OPTIONAL
} */
void TDAPduABRT::construct(void)
{
  asn1::ber::DecoderOfSequence_T<2>::setField(0, _fldTagAbrtSrc,
              asn1::ASTagging::tagsIMPLICIT, asn1::ber::EDAlternative::altMANDATORY);
  asn1::ber::DecoderOfSequence_T<2>::setField(1, TDUserInformation::_typeTag,
                                              asn1::ber::EDAlternative::altOPTIONAL);
}

// ----------------------------------------
// -- DecoderOfStructAC interface methods
// ----------------------------------------
//If necessary, allocates optional element and initializes associated TypeDecoderAC
asn1::ber::TypeDecoderAC *
  TDAPduABRT::prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */
{
  if (!_dVal) //assertion!!!
    throw smsc::util::Exception("tcap::proto::dec::TDAPduABRT : value isn't set!");
  if (unique_idx > 1)
    throw smsc::util::Exception("tcap::proto::dec::TDAPduABRT::prepareAlternative() : undefined UId");

  if (!unique_idx) {
    _abrtSrc.setValue(_dVal->_abrtSrc);
    return &_abrtSrc;
  }
  //if (unique_idx == 1) {}
  getUI()->setValue(_dVal->_usrInfo);
  return _pUI;
}

}}}}

