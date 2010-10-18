#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/tcap/proto/enc/TEMsgTAbort.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

const asn1::ASTagging
  TETAbort::_typeTags(asn1::ASTag::tagApplication,
                    7, asn1::ASTagging::tagsIMPLICIT);


TEDialoguePortionStructured *
  TETAbort::initDlgPortion(void)
{
  setField(1, _cause.uAbort().init(getTSRule()));
  return _cause.uAbort().get();
}

TEPAbortCause *
  TETAbort::setPrvdAbort(PAbort::Cause_e use_cause/* = PAbort::p_resourceLimitation*/)
{
  _cause.pAbort().init(getTSRule()).setValue(use_cause);
  setField(1, *_cause.pAbort().get());
  return _cause.pAbort().get();
}


}}}}

