#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/proto/enc/TEUserInformation.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

const asn1::ASTagging
  TEUserInformation::_typeTags(asn1::ASTag::tagContextSpecific, 30, asn1::ASTagging::tagsIMPLICIT);

asn1::ber::EncoderOfExternal *
  TEUserInformation::addUIValue(const asn1::ASExternal & val_ext)
{
  _uiArray.reserve(_uiArray.size() + 1);
  asn1::ber::EncoderOfExternal & rval = _uiArray[_uiArray.size()];

  rval.setTSRule(getTSRule());
  rval.setValue(val_ext);
  asn1::ber::EncoderOfStructure_T<1>::addField(rval);
  return &rval;
}

void TEUserInformation::addUIList(const tcap::TDlgUserInfoPtrList & ui_list)
{
  if (ui_list.empty())
    return;
  for (TDlgUserInfoPtrList::const_iterator cit = ui_list.begin(); cit != ui_list.end(); ++cit)
    addUIValue(**cit);
}

}}}}

