#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/tcap/proto/enc/TEMsgTEnd.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

const asn1::ASTagging
  TETEnd::_typeTags(asn1::ASTag::tagApplication,
                    4, asn1::ASTagging::tagsIMPLICIT);

/* End is defined in IMPLICIT tagging environment as follow:
End ::= [APPLICATION 4] SEQUENCE {
  dtid             DestTransactionID,
  dialoguePortion  DialoguePortion OPTIONAL,
  components       ComponentPortion OPTIONAL
} */

TEDialoguePortionStructured * TETEnd::initDlgPortion(void)
{
  setField(1, _partDlg.init(getTSRule()));
  return _partDlg.get();
}

TEComponentPortion * TETEnd::initCompPortion(void)
{
  setField(2, _partComp.init(getTSRule()));
  return _partComp.get();
}

void TETEnd::clearDlgPortion(void)
{
  _partDlg.clear(); 
  clearField(1);
}

void TETEnd::clearCompPortion(void)
{
  _partComp.clear(); 
  clearField(2);
}

}}}}

