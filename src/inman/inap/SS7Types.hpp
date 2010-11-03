/* ************************************************************************* *
 * Common Types used by SS7 releated stuff.
 * ************************************************************************* */
#ifndef __SMSC_SS7_COMMON_TYPES__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_SS7_COMMON_TYPES__

#include <inttypes.h>

#define MAX_SCCP_ADDRESS_LEN 32
#define MAX_ADDRESS_BUF_LEN 32
#define MAX_APP_CONTEXT_LEN 32
typedef struct
{
        uint8_t addrLen;
        uint8_t addr[MAX_SCCP_ADDRESS_LEN];
} SCCP_ADDRESS_T;

typedef struct
{
        uint8_t addrLen;
        uint8_t addr[MAX_ADDRESS_BUF_LEN];
} ADDRESS_BUF_T;

typedef struct
{
        uint8_t acLen;
        uint8_t ac[MAX_APP_CONTEXT_LEN];
} APP_CONTEXT_T;

#endif /* __SMSC_SS7_COMMON_TYPES__ */

