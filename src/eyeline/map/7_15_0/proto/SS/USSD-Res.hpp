/* ************************************************************************** *
 * USSD-Res type definition.
 * ************************************************************************** */
#ifndef __EYELINE_MAP_7F0_USSD_RES_HPP
#ident "@(#)$Id$"
#define __EYELINE_MAP_7F0_USSD_RES_HPP

#include "eyeline/asn1/UnknownExtensions.hpp"

#include "eyeline/map/7_15_0/proto/SS/USSD-DataCodingScheme.hpp"
#include "eyeline/map/7_15_0/proto/SS/USSD-String.hpp"

namespace eyeline {
namespace map {
namespace ss {

struct USSD_Res {
  USSD_DataCodingScheme_t ussd_DataCodingScheme;
  USSD_String_t	          ussd_String;

  /* This type is extensible, possible extensions are below. */
  asn1::UnknownExtensions _unkExt;
};

}}}

#endif /* __EYELINE_MAP_7F0_USSD_RES_HPP */

