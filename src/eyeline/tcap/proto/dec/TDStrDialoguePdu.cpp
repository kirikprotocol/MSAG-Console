#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

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
  setAlternative(0, TDAPduAARQ::_typeTag);
  setAlternative(1, TDAPduAARE::_typeTag);
  setAlternative(2, TDAPduABRT::_typeTag);
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

  if (!unique_idx)
    _pDec.aarq().init(getTSRule()).setValue(_dVal->aarq().init());
  else if (unique_idx == 1)
    _pDec.aare().init(getTSRule()).setValue(_dVal->aare().init());
  else //if (unique_idx == 2)
    _pDec.abrt().init(getTSRule()).setValue(_dVal->abrt().init());
  return _pDec.get();
}

}}}}

