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


void TEComponentPortion::addValuesList(const tcap::TComponentsPtrList & comp_list)
  /*throw(std::exception)*/
{
  if (comp_list.empty())
    return;
  for (tcap::TComponentsPtrList::const_iterator cit = comp_list.begin(); cit != comp_list.end(); ++cit)
    addValue(**cit);
}

}}}}

