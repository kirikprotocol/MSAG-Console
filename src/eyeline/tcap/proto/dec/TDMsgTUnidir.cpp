#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/tcap/proto/dec/TDMsgTUnidir.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace dec {

/* ************************************************************ *
 * Class TDMsgTUnidir implementation
 * ************************************************************ */
const asn1::ASTag
  TDMsgTUnidir::_typeTag(asn1::ASTag::tagApplication, 1);

/* Unidirectional message is defined in IMPLICIT tagging environment as follow:
Unidirectional ::= [APPLICATION 1] SEQUENCE {
    dialoguePortion  DialoguePortion OPTIONAL,
    components       ComponentPortion
} */
void TDMsgTUnidir::construct(void)
{
  setField(0, TDDialoguePortion::_typeTag, asn1::ber::EDAlternative::altOPTIONAL);
  setField(1, TDComponentPortion::_typeTag, asn1::ber::EDAlternative::altMANDATORY);
}

// ----------------------------------------
// -- DecoderOfStructAC interface methods
// ----------------------------------------
//If necessary, allocates optional element and initializes associated TypeDecoderAC
asn1::ber::TypeDecoderAC *
  TDMsgTUnidir::prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */
{
  if (!_dVal) //assertion!!!
    throw smsc::util::Exception("tcap::proto::dec::TDMsgTUnidir : value isn't set!");
  if (unique_idx > 1)
    throw smsc::util::Exception("tcap::proto::dec::TDMsgTUnidir::prepareAlternative() : undefined UId");

  if (!unique_idx) {
    _dlgPart.init(getTSRule()).setValue(_dVal->_dlgPart);
    return _dlgPart.get();
  }
  //if (unique_idx == 1) {}
  _compPart.setValue(_dVal->_compPart);
  return &_compPart;
}

}}}}

