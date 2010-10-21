#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/tcap/proto/dec/TDTCMessage.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace dec {
/* ************************************************************ *
 * Class TDTCMessage implementation
 * ************************************************************ */

/* According to Q.773, TCMessage is defined in IMPLICIT tagging
   environment as follow:

TCMessage ::= CHOICE {
  unidirectional    Unidirectional,
  begin             Begin,
  end               End,
  continue          Continue,
  abort             Abort
} */
void TDTCMessage::construct(void)
{
  setAlternative(0, TDMsgTUnidir::_typeTag);
  setAlternative(1, TDMsgTBegin::_typeTag);
  setAlternative(2, TDMsgTEnd::_typeTag);
  setAlternative(3, TDMsgTContinue::_typeTag);
  setAlternative(4, TDMsgTAbort::_typeTag);
}

// ----------------------------------------
// -- DecoderOfChoiceAC interface methods
// ----------------------------------------
//If necessary, allocates optional element and initializes associated TypeDecoderAC
asn1::ber::TypeDecoderAC *
  TDTCMessage::prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */
{
  if (!_dVal) //assertion!!!
    throw smsc::util::Exception("tcap::proto::dec::TDTCMessage : value isn't set!");
  if (unique_idx > 4)
    throw smsc::util::Exception("tcap::proto::dec::TDTCMessage::prepareAlternative() : undefined UId");

  if (!unique_idx)
    _pDec.unidir().init(getTSRule()).setValue(_dVal->unidir().init());
  else if (unique_idx == 1)
    _pDec.begin().init(getTSRule()).setValue(_dVal->begin().init());
  else if (unique_idx == 2)
    _pDec.end().init(getTSRule()).setValue(_dVal->end().init());
  else if (unique_idx == 3)
    _pDec.cont().init(getTSRule()).setValue(_dVal->cont().init());
  else //if (unique_idx == 4)
    _pDec.abort().init(getTSRule()).setValue(_dVal->abort().init());

  return _pDec.get();
}

}}}}

