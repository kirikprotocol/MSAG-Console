#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/map/7_15_0/proto/common/enc/MENetworkResource.hpp"

namespace eyeline {
namespace map {
namespace common {
namespace enc {

/* Type is defined in IMPLICIT tagging environment as follow:
NetworkResource ::= ENUMERATED {
        plmn  (0),
        hlr  (1),
        vlr  (2),
        pvlr  (3),
        controllingMSC  (4),
        vmsc  (5),
        eir  (6),
        rss  (7)
}
*/

void MENetworkResource::setValue(const NetworkResource & use_val)
  /*throw(std::exception)*/
{
  asn1::ber::EncoderOfENUM::setValue(use_val._value);
}

}}}}

