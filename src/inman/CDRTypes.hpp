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

const unsigned MAX_ESERVICE_TYPE_LENGTH = 6;
const unsigned MAX_SMESYSID_TYPE_LENGTH = 16;
const unsigned MAX_ROUTE_ID_TYPE_LENGTH = 33;

//SMPP DATA_SM service type
typedef smsc::core::buffers::FixedLengthString<MAX_ESERVICE_TYPE_LENGTH>
  SMPPServiceType;
//SM Entity system id
typedef smsc::core::buffers::FixedLengthString<MAX_SMESYSID_TYPE_LENGTH>
  SMESysId;
//SM Route ident
typedef smsc::core::buffers::FixedLengthString<MAX_ROUTE_ID_TYPE_LENGTH>
  SMRouteId;

} //inman
} //smsc

#endif /* __SMSC_INMAN_CDR_TYPES */

