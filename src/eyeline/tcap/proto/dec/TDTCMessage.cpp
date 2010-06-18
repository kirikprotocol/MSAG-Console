#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

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
  asn1::ber::DecoderOfChoice_T<5>::setAlternative(0, TDMsgTUnidir::_typeTag);
  asn1::ber::DecoderOfChoice_T<5>::setAlternative(1, TDMsgTBegin::_typeTag);
  asn1::ber::DecoderOfChoice_T<5>::setAlternative(2, TDMsgTEnd::_typeTag);
  asn1::ber::DecoderOfChoice_T<5>::setAlternative(3, TDMsgTContinue::_typeTag);
  asn1::ber::DecoderOfChoice_T<5>::setAlternative(4, TDMsgTAbort::_typeTag);
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

  cleanUp();
  if (!unique_idx)
    return _pDec._unidir = new (_memMsg._buf) TDMsgTUnidir(_dVal->initUnidir(), getVALRule());
  if (unique_idx == 1)
    return _pDec._begin = new (_memMsg._buf) TDMsgTBegin(_dVal->initBegin(), getVALRule());
  if (unique_idx == 2)
    return _pDec._end = new (_memMsg._buf) TDMsgTEnd(_dVal->initEnd(), getVALRule());
  if (unique_idx == 3)
    return _pDec._cont = new (_memMsg._buf) TDMsgTContinue(_dVal->initContinue(), getVALRule());
  //if (unique_idx == 4)
  return _pDec._abort = new (_memMsg._buf) TDMsgTAbort(_dVal->initAbort(), getVALRule());
}

}}}}

