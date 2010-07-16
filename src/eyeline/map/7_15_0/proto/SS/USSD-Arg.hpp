/* ************************************************************************** *
 * USSD-Arg type definition.
 * ************************************************************************** */
#ifndef __EYELINE_MAP_7F0_USSD_ARG_HPP
#ident "@(#)$Id$"
#define __EYELINE_MAP_7F0_USSD_ARG_HPP

#include "eyeline/asn1/UnknownExtensions.hpp"

#include "eyeline/map/7_15_0/proto/common/AlertingPattern.hpp"
#include "eyeline/map/7_15_0/proto/common/ISDN-AddressString.hpp"
#include "eyeline/map/7_15_0/proto/SS/USSD-DataCodingScheme.hpp"
#include "eyeline/map/7_15_0/proto/SS/USSD-String.hpp"


namespace eyeline {
namespace map {
namespace ss {

using eyeline::map::common::AlertingPattern;
using eyeline::map::common::ISDN_AddressString_t;

struct USSD_Arg {
  enum FieldPresence_e { has_alertingPattern = 0x01, has_msisdn = 0x02 };

  unsigned char           _fieldsMask;
  /* -- */
  USSD_DataCodingScheme_t ussd_DataCodingScheme;
  USSD_String_t	          ussd_String;

  /* This type is extensible, possible extensions are below. */
  AlertingPattern	      alertingPattern;	/* OPTIONAL */
  ISDN_AddressString_t	  msisdn;	        /* OPTIONAL */

  asn1::UnknownExtensions _unkExt;

  USSD_Arg() : _fieldsMask(0)
  { }
};

}}}

#endif /* __EYELINE_MAP_7F0_USSD_ARG_HPP */

