#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/tcap/proto/enc/TEAPduABRT.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

const asn1::ASTagging
  TEAPduABRT::_typeTags(asn1::ASTag::tagApplication,
                       4, asn1::ASTagging::tagsIMPLICIT);

const asn1::ASTagging 
  TEAPduABRT::FEAbortSource::_typeTags(asn1::ASTag::tagContextSpecific,
                                    0, asn1::ASTagging::tagsIMPLICIT);


TEUserInformation * TEAPduABRT::getUI(void)
{
  if (!_pUI.get()) {
    _pUI.init(getTSRule());
    setField(1, *_pUI.get());
  }
  return _pUI.get();
}

}}}}

