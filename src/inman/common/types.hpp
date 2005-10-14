#ident "$Id$"

#ifndef __SMSC_INMAN_COMMON_TYPES__
#define __SMSC_INMAN_COMMON_TYPES__

#define MAX_SCCP_ADDRESS_LEN 32
#define MAX_ADDRESS_BUF_LEN 32
#define MAX_APP_CONTEXT_LEN 32
typedef struct
{
        UCHAR_T addrLen;
        UCHAR_T addr[MAX_SCCP_ADDRESS_LEN];
} SCCP_ADDRESS_T;

typedef struct
{
        UCHAR_T addrLen;
        UCHAR_T addr[MAX_ADDRESS_BUF_LEN];
} ADDRESS_BUF_T;

typedef struct
{
        UCHAR_T acLen;
        UCHAR_T ac[MAX_APP_CONTEXT_LEN];
} APP_CONTEXT_T;

typedef struct
{
        UCHAR_T operation;

} OPER_T;
typedef struct
{
        UCHAR_T operation;

} COMP_T;
#endif
