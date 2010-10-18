#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/tcap/proto/enc/TEMsgTBegin.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

const asn1::ASTagging
  TETBegin::_typeTags(asn1::ASTag::tagApplication,
                    2, asn1::ASTagging::tagsIMPLICIT);

/* Begin is defined in IMPLICIT tagging environment as follow:
Begin ::= [APPLICATION 2] SEQUENCE {
  otid             OrigTransactionID,
  dialoguePortion  DialoguePortion OPTIONAL,
  components       ComponentPortion OPTIONAL
} */

TEDialoguePortionStructured * TETBegin::initDlgPortion(void)
{
  setField(1, _partDlg.init(getTSRule()));
  return _partDlg.get();
}

TEComponentPortion * TETBegin::initCompPortion(void)
{
  setField(2, _partComp.init(getTSRule()));
  return _partComp.get();
}

void TETBegin::clearDlgPortion(void)
{
  _partDlg.clear(); 
  clearField(1);
}

void TETBegin::clearCompPortion(void)
{
  _partComp.clear(); 
  clearField(2);
}

}}}}

