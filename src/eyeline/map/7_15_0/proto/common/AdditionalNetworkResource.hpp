/* ************************************************************************** *
 * AdditionalNetworkResource type definition.
 * ************************************************************************** */
#ifndef __EYELINE_MAP_7F0_ADDITIONAL_NETWORK_RESOURCE_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_MAP_7F0_ADDITIONAL_NETWORK_RESOURCE_HPP

namespace eyeline {
namespace map {
namespace common {

struct AdditionalNetworkResource {
  typedef unsigned char value_type;

  enum Id_e {
    nra_sgsn = 0,
    nra_ggsn = 1,
    nra_gmlc = 2,
    nra_gsmSCF = 3,
    nra_nplr = 4,
    nra_auc = 5,
    nra_ue = 6,
    /* ... extensible type */
    nra_unknown = 0xFF
  };

  value_type _value;

  static Id_e value2Id(value_type use_val)
  {
    return (use_val > 6) ? nra_unknown : static_cast<Id_e>(use_val);
  }

  Id_e value2Id(void) const { return value2Id(_value); }
};

typedef AdditionalNetworkResource::value_type AdditionalNetworkResource_t;

} //common
} //map
} //eyeline

#endif /* __EYELINE_MAP_7F0_ADDITIONAL_NETWORK_RESOURCE_HPP */

