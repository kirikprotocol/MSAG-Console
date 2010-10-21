#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/tcap/proto/dec/TDMsgTContinue.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace dec {

/* ************************************************************ *
 * Class TDMsgTContinue implementation
 * ************************************************************ */
const asn1::ASTag
  TDMsgTContinue::_typeTag(asn1::ASTag::tagApplication, 5);

/* Continue message is defined in IMPLICIT tagging environment as follow:
Continue ::= [APPLICATION 5] SEQUENCE {
    otid             OrigTransactionID,
    dtid             DestTransactionID,
    dialoguePortion  DialoguePortion OPTIONAL,
    components       ComponentPortion
} */
void TDMsgTContinue::construct(void)
{
  setField(0, TDOrigTransactionId::_typeTag, asn1::ber::EDAlternative::altMANDATORY);
  setField(0, TDDestTransactionId::_typeTag, asn1::ber::EDAlternative::altMANDATORY);
  setField(2, TDDialoguePortion::_typeTag, asn1::ber::EDAlternative::altOPTIONAL);
  setField(3, TDComponentPortion::_typeTag, asn1::ber::EDAlternative::altMANDATORY);
}

// ----------------------------------------
// -- DecoderOfStructAC interface methods
// ----------------------------------------
//If necessary, allocates optional element and initializes associated TypeDecoderAC
asn1::ber::TypeDecoderAC *
  TDMsgTContinue::prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */
{
  if (!_dVal) //assertion!!!
    throw smsc::util::Exception("tcap::proto::dec::TDMsgTContinue : value isn't set!");
  if (unique_idx > 3)
    throw smsc::util::Exception("tcap::proto::dec::TDMsgTContinue::prepareAlternative() : undefined UId");

  if (!unique_idx) {
    _orgTrId.setValue(_dVal->_orgTrId);
    return &_orgTrId;
  }
  if (unique_idx == 1) {
    _dstTrId.setValue(_dVal->_dstTrId);
    return &_dstTrId;
  }
  if (unique_idx == 2) {
    _dlgPart.init(getTSRule()).setValue(_dVal->_dlgPart);
    return _dlgPart.get();
  }
  //if (unique_idx == 3) {}
  _compPart.setValue(_dVal->_compPart);
  return &_compPart;
}

}}}}

