#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/proto/enc/TEComponentPortion.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

const asn1::ASTagging
  TEComponentPortion::_typeTags(asn1::ASTag::tagApplication, 12,
                              asn1::ASTagging::tagsIMPLICIT);


//Creates and appends new component to list
TEComponent * TEComponentPortion::addComponent(void)
{
  _comps.reserve(_comps.size() + 1);
  TEComponent & rval = _comps[_comps.size()]; //just initialize component

  rval.setTSRule(getTSRule());
  asn1::ber::EncoderOfStructure_T<_dfltNumOfComponents>::addField(rval);
  return &rval;
}

}}}}

