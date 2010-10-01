/* ************************************************************************** *
 * NetworkResource type definition.
 * ************************************************************************** */
#ifndef __EYELINE_MAP_7F0_NETWORK_RESOURCE_HPP
#ident "@(#)$Id$"
#define __EYELINE_MAP_7F0_NETWORK_RESOURCE_HPP

namespace eyeline {
namespace map {
namespace common {

struct NetworkResource {
  typedef unsigned char value_type;

  enum Id_e {
    nr_plmn = 0,
    nr_hlr = 1,
    nr_vlr = 2,
    nr_pvlr = 3,
    nr_controllingMSC = 4,
    nr_vmsc = 5,
    nr_eir = 6,
    nr_rss = 7
  };

  value_type _value;

  bool validate(value_type use_val) const { return use_val <= nr_rss; }
};

typedef NetworkResource::value_type NetworkResource_t;

} //common
} //map
} //eyeline

#endif /* __EYELINE_MAP_7F0_NETWORK_RESOURCE_HPP */

