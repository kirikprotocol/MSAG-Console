#ident "$Id$"

#ifndef __SMSC_INMAN_COMMON_TYPES__
#define __SMSC_INMAN_COMMON_TYPES__

#ifndef UCHAR_T
#define UCHAR_T           unsigned char      
#endif /* UCHAR_T */
#ifndef USHORT_T
#define USHORT_T          unsigned short     
#endif /* USHORT_T */
#ifndef UINT_T
#define UINT_T            unsigned int       
#endif /* UINT_T */


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
#endif /* __SMSC_INMAN_COMMON_TYPES__ */

