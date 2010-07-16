/* ************************************************************************** *
 * ISDN Address String type definition.
 * ************************************************************************** */
#ifndef __EYELINE_MAP_7F0_ISDN_ADDRESS_STRING_HPP
#ident "@(#)$Id$"
#define __EYELINE_MAP_7F0_ISDN_ADDRESS_STRING_HPP

#include "eyeline/asn1/OCTSTR.hpp"

namespace eyeline {
namespace map {
namespace common {

/* This type is used to represent ISDN numbers.
 -- ISDN-AddressString ::= AddressString (SIZE (1..maxISDN-AddressLength))
 -- maxISDN-AddressLength  INTEGER ::= 9
*/
static const uint8_t _maxISDN_AddressLength = 9;

typedef asn1::OCTSTR_TINY_T<_maxISDN_AddressLength> ISDN_AddressString_t;

} //common
} //map
} //eyeline

#endif /* __EYELINE_MAP_7F0_ISDN_ADDRESS_STRING_HPP */

