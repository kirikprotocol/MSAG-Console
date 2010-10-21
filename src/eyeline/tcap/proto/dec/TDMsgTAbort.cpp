#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/tcap/proto/dec/TDMsgTAbort.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace dec {
/* ************************************************************ *
 * Class TDAbortReason implementation
 * ************************************************************ */

/* Abort is defined in IMPLICIT tagging environment as follow:
AbortReason ::= CHOICE {
    p-abortCause  P-AbortCause,
    u-abortCause  DialoguePortion
} */
void TDAbortReason::construct(void)
{
  setAlternative(0, TDPAbortCause::_typeTag);
  setAlternative(1, TDDialoguePortion::_typeTag);
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

  if (!unique_idx)
    _altDec.pCause().init(getTSRule()).setValue(_dVal->prvd().init());
  else   //if (unique_idx == 1)
    _altDec.uCause().init(getTSRule()).setValue(_dVal->user().init());
  return _altDec.get();
}

/* ************************************************************ *
 * Class TDMsgTAbort implementation
 * ************************************************************ */
const asn1::ASTag TDMsgTAbort::_typeTag(asn1::ASTag::tagApplication, 7);

/* Abort is defined in IMPLICIT tagging environment as follow:
Abort ::= [APPLICATION 7] SEQUENCE {
  dtid    DestTransactionID,
  reason  AbortReason OPTIONAL
} */
void TDMsgTAbort::construct(void)
{
  setField(0, TDDestTransactionId::_typeTag, asn1::ber::EDAlternative::altMANDATORY);
  setField(1, TDAbortReason::_tagOptions, asn1::ber::EDAlternative::altOPTIONAL);
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
  _reason.init(getTSRule()).setValue(_dVal->_reason);
  return _reason.get();
}

}}}}

