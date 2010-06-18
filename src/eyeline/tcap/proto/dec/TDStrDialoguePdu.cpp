#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/proto/dec/TDStrDialoguePdu.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace dec {
/* ************************************************************ *
 * Class TDStrDialoguePdu implementation
 * ************************************************************ */

/* According to Q.773, StructuredDialoguePDU is defined in EXPLICIT tagging
   environment as follow:

DialoguePDU ::= CHOICE {
  dialogueRequest   AARQ-apdu,
  dialogueResponse  AARE-apdu,
  dialogueAbort     ABRT-apdu
} */
void TDStrDialoguePdu::construct(void)
{
  asn1::ber::DecoderOfChoice_T<3>::setAlternative(0, TDAPduAARQ::_typeTag);
  asn1::ber::DecoderOfChoice_T<3>::setAlternative(1, TDAPduAARE::_typeTag);
  asn1::ber::DecoderOfChoice_T<3>::setAlternative(2, TDAPduABRT::_typeTag);
}

// ----------------------------------------
// -- DecoderOfChoiceAC interface methods
// ----------------------------------------
//If necessary, allocates optional element and initializes associated TypeDecoderAC
asn1::ber::TypeDecoderAC *
  TDStrDialoguePdu::prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */
{
  if (!_dVal) //assertion!!!
    throw smsc::util::Exception("tcap::proto::dec::TDStrDialoguePdu : value isn't set!");
  if (unique_idx > 2)
    throw smsc::util::Exception("tcap::proto::dec::TDStrDialoguePdu::prepareAlternative() : undefined UId");

  cleanUp();
  if (!unique_idx) {
    _pDec._aarq = new (_memPdu._buf) TDAPduAARQ(getVALRule());
    _pDec._aarq->setValue(_dVal->initAARQ());
    return _pDec._aarq;
  }
  if (unique_idx == 1) {
    _pDec._aare = new (_memPdu._buf) TDAPduAARE(getVALRule());
    _pDec._aare->setValue(_dVal->initAARE());
    return _pDec._aare;
  }
  //if (unique_idx == 2)
  _pDec._abrt = new (_memPdu._buf) TDAPduABRT(getVALRule());
  _pDec._abrt->setValue(_dVal->initABRT());
  return _pDec._abrt;
}

}}}}

