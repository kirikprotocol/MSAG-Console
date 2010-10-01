#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/map/7_15_0/proto/common/enc/MEAdditionalNetworkResource.hpp"

namespace eyeline {
namespace map {
namespace common {
namespace enc {

/* Type is defined in IMPLICIT tagging environment as follow:
AdditionalNetworkResource ::= ENUMERATED {
        sgsn (0),
        ggsn (1),
        gmlc (2),
        gsmSCF (3),
        nplr (4),
        auc (5),
        ... ,
        ue (6)
} */

void MEAdditionalNetworkResource::setValue(const AdditionalNetworkResource & use_val)
  /*throw(std::exception)*/
{
  asn1::ber::EncoderOfENUM::setValue(use_val._value);
}

}}}}

