/* ************************************************************************** *
 * USSD-Arg type definition.
 * ************************************************************************** */
#ifndef __EYELINE_MAP_7F0_USSD_ARG_HPP
#ident "@(#)$Id$"
#define __EYELINE_MAP_7F0_USSD_ARG_HPP

#include "eyeline/util/OptionalObjT.hpp"
#include "eyeline/asn1/UnknownExtensions.hpp"

#include "eyeline/map/7_15_0/proto/common/AlertingPattern.hpp"
#include "eyeline/map/7_15_0/proto/common/ISDN-AddressString.hpp"
#include "eyeline/map/7_15_0/proto/SS/USSD-DataCodingScheme.hpp"
#include "eyeline/map/7_15_0/proto/SS/USSD-String.hpp"


namespace eyeline {
namespace map {
namespace ss {

using eyeline::util::OptionalObj_T;
using eyeline::map::common::AlertingPattern;
using eyeline::map::common::ISDN_AddressString_t;

struct USSD_Arg {
  USSD_DataCodingScheme_t ussd_DataCodingScheme;
  USSD_String_t	          ussd_String;

  /* This type is extensible, possible extensions are below. */
  OptionalObj_T<AlertingPattern>      alertingPattern;
  OptionalObj_T<ISDN_AddressString_t> msisdn;
  asn1::UnknownExtensions             _unkExt;

  USSD_Arg() : ussd_DataCodingScheme(0)
  { }
  ~USSD_Arg()
  { }
};

}}}

#endif /* __EYELINE_MAP_7F0_USSD_ARG_HPP */

