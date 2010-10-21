#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

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
  setField(0, TDDestTransactionId::_typeTag, asn1::ber::EDAlternative::altMANDATORY);
  setField(1, TDDialoguePortion::_typeTag, asn1::ber::EDAlternative::altOPTIONAL);
  setField(2, TDComponentPortion::_typeTag, asn1::ber::EDAlternative::altMANDATORY);
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
    _dlgPart.init(getTSRule()).setValue(_dVal->_dlgPart);
    return _dlgPart.get();
  }
  //if (unique_idx == 2) {}
  _compPart.setValue(_dVal->_compPart);
  return &_compPart;
}

}}}}

