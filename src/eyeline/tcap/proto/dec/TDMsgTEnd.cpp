#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/proto/dec/TDMsgTEnd.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace dec {

/* ************************************************************ *
 * Class TDMsgTEnd implementation
 * ************************************************************ */
const asn1::ASTag
  TDMsgTEnd::_typeTag(asn1::ASTag::tagApplication, 4);

/* End message is defined in IMPLICIT tagging environment as follow:
End ::= [APPLICATION 4] SEQUENCE {
    dtid             DestTransactionID,
    dialoguePortion  DialoguePortion OPTIONAL,
    components       ComponentPortion
} */
void TDMsgTEnd::construct(void)
{
  asn1::ber::DecoderOfSequence_T<3>::setField(0, TDDestTransactionId::_typeTag, asn1::ber::EDAlternative::altMANDATORY);
  asn1::ber::DecoderOfSequence_T<3>::setField(1, TDDialoguePortion::_typeTag, asn1::ber::EDAlternative::altOPTIONAL);
  asn1::ber::DecoderOfSequence_T<3>::setField(2, TDComponentPortion::_typeTag, asn1::ber::EDAlternative::altMANDATORY);
}

// ----------------------------------------
// -- DecoderOfStructAC interface methods
// ----------------------------------------
//If necessary, allocates optional element and initializes associated TypeDecoderAC
asn1::ber::TypeDecoderAC *
  TDMsgTEnd::prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */
{
  if (!_dVal) //assertion!!!
    throw smsc::util::Exception("tcap::proto::dec::TDMsgTEnd : value isn't set!");
  if (unique_idx > 2)
    throw smsc::util::Exception("tcap::proto::dec::TDMsgTEnd::prepareAlternative() : undefined UId");

  if (!unique_idx) {
    _dstTrId.setValue(_dVal->_dstTrId);
    return &_dstTrId;
  }
  if (unique_idx == 1) {
    cleanUp();
    return _dlgPart = new (_memDlg._buf) TDDialoguePortion(_dVal->_dlgPart, getVALRule());
  }
  //if (unique_idx == 2) {}
  _compPart.setValue(_dVal->_compPart);
  return &_compPart;
}

}}}}

