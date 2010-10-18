#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/tcap/proto/enc/TEMsgTContinue.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

const asn1::ASTagging
  TETContinue::_typeTags(asn1::ASTag::tagApplication,
                    5, asn1::ASTagging::tagsIMPLICIT);

/* Continue is defined in IMPLICIT tagging environment as follow:
Continue ::= [APPLICATION 5] SEQUENCE {
  otid             OrigTransactionID,
  dtid             DestTransactionID,
  dialoguePortion  DialoguePortion OPTIONAL,
  components       ComponentPortion OPTIONAL
} */

TEDialoguePortionStructured * TETContinue::initDlgPortion(void)
{
  setField(2, _partDlg.init(getTSRule()));
  return _partDlg.get();
}

TEComponentPortion * TETContinue::initCompPortion(void)
{
  setField(3, _partComp.init(getTSRule()));
  return _partComp.get();
}

void TETContinue::clearDlgPortion(void)
{
  _partDlg.clear(); 
  clearField(2);
}

void TETContinue::clearCompPortion(void)
{
  _partComp.clear(); 
  clearField(3);
}

}}}}

