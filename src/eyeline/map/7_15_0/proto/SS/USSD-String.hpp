/* ************************************************************************** *
 * USSD-String type definition.
 * ************************************************************************** */
#ifndef __EYELINE_MAP_7F0_USSD_STRING_HPP
#ident "@(#)$Id$"
#define __EYELINE_MAP_7F0_USSD_STRING_HPP

#include "eyeline/asn1/OCTSTR.hpp"

namespace eyeline {
namespace map {
namespace ss {

static const uint8_t _maxUSSD_StringLength = 160;

typedef asn1::OCTSTR_TINY_T<_maxUSSD_StringLength> USSD_String_t;
// -- The structure of the contents of the USSD-String is dependent
// -- on the USSD-DataCodingScheme as described in TS 3GPP TS 23.038 [25].

}}}

#endif /* __EYELINE_MAP_7F0_USSD_STRING_HPP */

