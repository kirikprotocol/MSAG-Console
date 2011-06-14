/* ************************************************************************* *
 * SMSX Extra service mask bits definition. 
 * ************************************************************************* */
#ifndef __INMAN_SMBILLING_SMSX_SRVMASK_DEFS
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_SMBILLING_SMSX_SRVMASK_DEFS

#include <inttypes.h>

namespace smsc    {
namespace inman   {
namespace smbill  {

static const uint32_t SMSX_NOCHARGE_SRV = 0x80000000;   //SMSX Extra service free of charge
static const uint32_t SMSX_INCHARGE_SRV = 0x40000000;   //Charge SMS via IN point despite of billMode setting

//_smsXSrvs bits to mask, i.e. exclude from processing logic
static const uint32_t SMSX_RESERVED_MASK = (SMSX_NOCHARGE_SRV|SMSX_INCHARGE_SRV);

} //smbill
} //inman
} //smsc
#endif /* __INMAN_SMBILLING_SMSX_SRVMASK_DEFS */

