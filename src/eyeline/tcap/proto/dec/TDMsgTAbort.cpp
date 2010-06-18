#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/proto/dec/TDMsgTAbort.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace dec {
/* ************************************************************ *
 * Class TDAbortReason implementation
 * ************************************************************ */
void TDAbortReason::cleanUp(void)
{
  if (_altDec._ptr) {
    _altDec._ptr->~TypeDecoderAC();
    _altDec._ptr = NULL;
    _altKind = proto::TAbortReason::causeNone;
  }
}

/* Abort is defined in IMPLICIT tagging environment as follow:
AbortReason ::= CHOICE {
    p-abortCause  P-AbortCause,
    u-abortCause  DialoguePortion
} */
void TDAbortReason::construct(void)
{
  asn1::ber::DecoderOfChoice_T<2>::setAlternative(0, TDPAbortCause::_typeTag);
  asn1::ber::DecoderOfChoice_T<2>::setAlternative(1, TDDialoguePortion::_typeTag);
}
// ----------------------------------------
// -- DecoderOfChoiceAC interface methods
// ----------------------------------------
//If necessary, allocates alternative and initializes associated TypeDecoderAC
asn1::ber::TypeDecoderAC *
  TDAbortReason::prepareAlternative(uint16_t unique_idx) /*throw(std::exception)*/
{
  if (!_dVal) //assertion!!!
    throw smsc::util::Exception("tcap::proto::dec::TDAbortReason : value isn't set!");
  if (unique_idx > 1)
    throw smsc::util::Exception("tcap::proto::dec::TDAbortReason::prepareAlternative() : undefined UId");

  cleanUp();
  if (!unique_idx)
    return _altDec._pCause = new (_memAlt._buf) TDPAbortCause(_dVal->initPrvd(), getVALRule());
  //if (unique_idx == 1)
  return _altDec._uCause = new (_memAlt._buf) TDDialoguePortion(_dVal->initUser(), getVALRule());
}

/* ************************************************************ *
 * Class TDMsgTAbort implementation
 * ************************************************************ */
const asn1::ASTag
  TDMsgTAbort::_typeTag(asn1::ASTag::tagApplication, 7);

/* Abort is defined in IMPLICIT tagging environment as follow:
Abort ::= [APPLICATION 7] SEQUENCE {
  dtid    DestTransactionID,
  reason  AbortReason OPTIONAL
} */
void TDMsgTAbort::construct(void)
{
  asn1::ber::DecoderOfSequence_T<2>::setField(0, TDDestTransactionId::_typeTag, asn1::ber::EDAlternative::altMANDATORY);
  asn1::ber::DecoderOfSequence_T<2>::setField(1, TDAbortReason::_tagOptions, asn1::ber::EDAlternative::altOPTIONAL);
}

// ----------------------------------------
// -- DecoderOfStructAC interface methods
// ----------------------------------------
//If necessary, allocates optional element and initializes associated TypeDecoderAC
asn1::ber::TypeDecoderAC *
  TDMsgTAbort::prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */
{
  if (!_dVal) //assertion!!!
    throw smsc::util::Exception("tcap::proto::dec::TDMsgTAbort : value isn't set!");
  if (unique_idx > 1)
    throw smsc::util::Exception("tcap::proto::dec::TDMsgTAbort::prepareAlternative() : undefined UId");

  if (!unique_idx) {
    _dstTrId.setValue(_dVal->_dstTrId);
    return &_dstTrId;
  }
  //if (unique_idx == 1) {}
  cleanUp();
  return _reason = new (_memReason._buf) TDAbortReason(_dVal->_reason, getVALRule());
}

}}}}

