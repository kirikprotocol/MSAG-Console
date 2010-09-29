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


void TEUserInformation::addValuesList(const tcap::TDlgUserInfoPtrList & ui_list)
  /*throw(std::exception)*/
{
  if (ui_list.empty())
    return;
  for (TDlgUserInfoPtrList::const_iterator cit = ui_list.begin(); cit != ui_list.end(); ++cit)
    addValue(**cit);
}

}}}}

