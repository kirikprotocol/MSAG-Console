/* ************************************************************************* *
 * Definition of types used in SM/USSD Collected Data Records.
 * ************************************************************************* */
#ifndef __SMSC_INMAN_CDR_TYPES
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_CDR_TYPES

#include "core/buffers/FixedLengthString.hpp"

namespace smsc  {
namespace inman {

const unsigned MAX_ESERVICE_TYPE_LENGTH = 5;
const unsigned MAX_SMESYSID_TYPE_LENGTH = 15;
const unsigned MAX_ROUTE_ID_TYPE_LENGTH = 32;

//SMPP DATA_SM service type
typedef smsc::core::buffers::FixedLengthString<MAX_ESERVICE_TYPE_LENGTH + 1>
  SMPPServiceType;
//SM Entity system id
typedef smsc::core::buffers::FixedLengthString<MAX_SMESYSID_TYPE_LENGTH + 1>
  SMESysId;
//SM Route ident
typedef smsc::core::buffers::FixedLengthString<MAX_ROUTE_ID_TYPE_LENGTH + 1>
  SMRouteId;

} //inman
} //smsc

#endif /* __SMSC_INMAN_CDR_TYPES */

