#ifndef __SS7TMC_H__
#define __SS7TMC_H__

/*********************************************************************/
/*                                                                   */
/* ss7tmc.h,v                                            */
/*                                                                   */
/*-------------------------------------------------------------------*/
/* COPYRIGHT Ericsson Infotech                                       */
/*                                                                   */
/* The copyright to the computer  program herein is the property of  */
/* Ericsson Infotech. The program may be used and/or                 */
/* copied only with the written permission from Ericsson Infotech    */
/* or in the accordance with the terms and conditions                */
/* stipulated in the agreement/contract under which the program has  */
/* been supplied.                                                    */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Document Number:                 (Rev:    )                       */
/* 15/190 55-CAA 201 29 Ux                                           */
/*                                                                   */
/* Revision:                                                         */
/* @EINVER: ss7tmc.h,v 1.39 2000/06/30 09:42:57 Exp */
/*                                                                   */
/* Programmer:                                                       */
/* EIN/S/AP <Your Name>                                              */
/*                                                                   */
/* Purpose: Definitions for Common Parts                             */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Revision record:                                                  */
/*                                                                   */
/* 00-960315  EIN/N/D Henrik Berglund                                */
/*            Major changes for new configurable socket CP           */ 
/*                                                                   */
/* 00-970313  EIN/N/S Pär Larsson                                    */
/*            Added:                                                 */ 
/*               MSG_WOULD_BLOCK                                     */
/*                                                                   */
/* 01-970625 EIN/N/P Dennis Eriksson                                 */
/*           Removed XLP.h                                           */
/*           Removed TAGs they are now in ss7util.h                  */
/*                                                                   */
/* 03-990322  EIN/N/E Dan Liljemark                                  */
/*            Updated after review.                                  */
/*            Inspection master plan: 32/17017-CAA20129              */
/*                                                                   */
/*-------------------------------------------------------------------*/
/* Notes:                                                            */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/* ss7tmc.h                                                          */
/*********************************************************************/
#include "ss7osdpn.h"
#include "portss7.h"
#include "ss7osdpn.h"

#if defined (__cplusplus) || defined (c_plusplus)
extern "C" {
#endif

/*********************************************************************/
/*                                                                   */
/*                        C O N S T A N T S                          */
/*                                                                   */
/*********************************************************************/
/* Common part version */
#define EINSS7_CP_VERSION 7

/* 
 * Default max size of data part in one message 
 * Can be changed by supplying a value in the
 * configuration file.
 */

#define MSG_MAX_DATALEN        5000


/* Max length of mpBaseName: <socket rendezvous directory><base name> */
#define EINSS7CP_MPBASENAME_MAX_LEN  (100)

/* 
 *  Number of message port identities.
 *  The message port identifiers are the unit id's as
 *  defined in portss7.h
 */

#define PREDEFINED_PORTS MAX_USERS

/* 
 * For backwards Compability
 */

#define PREDEFINED_QUEUES PREDEFINED_PORTS 


/*
 * These are to be used in the interface 
 */

#define MSG_INFTIM            -1

#define MSG_EVENT_READ         0x80000000
#define MSG_EVENT_WRITE        0x40000000
#define MSG_EVENT_READ_OBAND   0x20000000

#define MSG_HEADER_LENGTH 8


/*
 * Closed socket is represented with EINSS7CP_CLOSED_SOCKET    
 */
#ifndef EINSS7_WINNT
static const SHORT_T EINSS7CP_CLOSED_SOCKET = -1;
#else
static const EINSS7CPSOCKET_T EINSS7CP_CLOSED_SOCKET = 0;
#endif /* EINSS7_WINNT */

/***************************************************************************/
/* Macros                                                                  */
/***************************************************************************/
#define DPRINT(text,var) printf(text,var)
#define DPUTS(text) puts(text)

/* UlMe adds for compiler reasons 960717 */

/* Macros when using XLIB */
#ifdef SS7_LINK_XLIB
#define XFreeAll_m  CmFreeAll
#define XMemInit_m  CmMemInit
#define XMalloc_m   CmMalloc
#define XRealloc_m  CmRealloc
#define XGetBlock_m CmGetBlock
#define XFreeBlock  CmFreeBlock
#else
#define XFree       XMemFree
#define XFreeAll_m  XFreeAll
#define XMemInit_m  XMemInit
#define XMalloc_m   XMalloc
#define XRealloc_m  XRealloc
#define XGetBlock_m XGetBlock
#define XFreeBlock_m XFreeBlock
#endif /* SS7_LINK_XLIB */

/* Added to be "forward compatible" to CP R9.    ulni 010905 */
#define EINSS7CpMsgErrInfoGetMPOwners(a,b,c,d) MsgErrInfoGetMPOwners(a,b,c)
#define EINSS7CpMsgEnableUser(a,b) MsgEnableUser(a)
#define EINSS7CpMsgDisableUser(a,b) MsgDisableUser(a)
#define EINSS7CpMsgAllowConn(a,b,c) MsgAllowConn(a,b)
#define EINSS7CpMsgDisAllowConn(a,b,c) MsgDisAllowConn(a,b)
#define EINSS7CpMsgObtainSocketInst(a,b,c) EINSS7CpMsgObtainSocket(a,b)
#define EINSS7CpMsgConnInst(a,b,c) MsgConn(a,b)
#define EINSS7CpMsgConnInst_CW(a,b,c) MsgConn_CW(a,b)
#define EINSS7CpMsgRelInst(a,b,c) MsgRel(a,b)
#define EINSS7CpMsgInitInst(a,b) MsgInit(a)
#define EINSS7CpMsgSendInst(a) MsgSend(a)
#define EINSS7CpMsgConnBroadcast(a,b) MsgConn(a,b)
#define EINSS7CpMsgRelBroadcast(a,b) MsgRel(a,b)
#define EINSS7CpMsgInitInstNoSig(a,b) EINSS7CpMsgInitNoSig(a)
#define EINSS7CpMsgConnBroadcastRes(a,b,c,d,e) MsgConn(a,b)
#define EINSS7CpMsgSendBroadcast(a) MsgSend(a)
#define EINSS7CpMsgSendBroadcastRes(a,b,c) MsgSend(a)
#define EINSS7CpMsgConnBroadcastRes_CW(a,b,c,d,e) MsgConn(a,b)

/***************************************************************************/
/*                                                                         */
/*                             Return values                               */
/*                                                                         */
/***************************************************************************/

/* 
 * General 
 */

#define RETURN_OK       0
#define RETURN_ERR      65535

/* 
 * Message handling 
 */

#define        MSG_OK                        0  
#define        MSG_CLOSE_OK                  0
#define        MSG_INIT_OK                   0  
#define        MSG_OPEN_OK                   0
#define        MSG_RECEIVE_OK                0
#define        MSG_CONNECT_OK                0
#define        MSG_RELEASE_OK                0
#define        MSG_SEND_OK                   0

#define        MSG_RCVSND_TIMEOUT            1000
#define        MSG_OUT_OF_MEMORY             1001

#define        MSG_ID_NOT_FOUND              1002  
#define        MSG_QUEUE_NOT_OPEN            1003  
#define        MSG_NOT_OPEN                  1004  
#define        MSG_SIZE_ERROR                1005
#define        MSG_INTERNAL_QUEUE_FULL       1006
#define        MSG_ERR                       1007 
#define        MSG_SYSTEM_INTERRUPT          1008
#define        MSG_NOT_CONNECTED             1009
#define        MSG_NOT_ACCEPTED              1010
#define        MSG_BROKEN_CONNECTION         1011
#define        MSG_BROKEN_SOCKET             1011 /* obsolete */

#define        MSG_TIMEOUT                   MSG_RCVSND_TIMEOUT

#define        MSG_NOT_MPOWNER               1012
#define        MSG_BAD_FD                    1013    
#define        MSG_ARG_VAL                   1014
#define        MSG_APPL_EVENT                1015
#define        MSG_OPEN_FAILED               1016
#define        MSG_WOULD_BLOCK               1017 
#define        EINSS7CP_MSG_GETBUF_FAIL      1018
#define        EINSS7CP_MSG_RELBUF_FAIL      1019

/* Timer handling */
#define        TIME_INIT_FAILED              1020
#define        TO_MANY_TIMERS                1021
#define        TOO_MANY_TIMERS               TO_MANY_TIMERS

/* Interrupt handling */
#define        EINSS7CP_NAME_NOT_FOUND       1022

#define        MSG_ID_NOT_IN_CONFIG_FILE     1023

/* Xtended memory handling */
#define        XMEM_INVALID_USERID           1030
#define        XMEM_OUT_OF_MEMORY            1031
#define        XMEM_INVALID_POINTER          1032
#define        XMEM_INIT_DONE                1033   
#define        XMEM_INVALID_PARAMETER        1034
#define        XMEM_INVALID_SIZE             1035
#define        XMEM_CORRUPT_MEM              1036

#ifdef EINSS7_THREADSAFE
/* Thread problems */
#define        EINSS7CP_THREAD_MI_PROBLEM       1040
#define        EINSS7CP_THREAD_KC_PROBLEM       1041
#define        EINSS7CP_THREAD_CI_PROBLEM       1042

#define        EINSS7CP_MPOWNER_CLOSED          1043

#define        EINSS7CP_TIME_NOT_INIT           1044

#define        EINSS7CP_NO_TOKEN                1045
#endif /* EINSS7_THREADSAFE */

/*
 * Errors in socket and event handling
 */
#define	       EINSS7CP_INVALID_SOCKET		1060
#define	       EINSS7CP_SOCKET_ERROR		1061
#define	       EINSS7CP_INVALID_EVENT		1062
#define	       EINSS7CP_EVENT_ERROR		1063

#define	       EINSS7CP_MSG_MSGDELAY_ERROR     	1124

/*
 * Errors in config file
 */

#define        MSG_UNKNOWN_FILE                 1100

#define        MSG_IPA_UNKNOWN_MODULE           1101
#define        MSG_IPA_SYNTAX_ERROR             1102
#define        MSG_IPA_NOT_MPOWNER              1103
#define        MSG_IPA_MISSING_ADDRESS          1104
#define        MSG_IPA_MULTIPLE_ENTRIES         1105

#define        MSG_INTERACT_UNKNOWN_MODULE      1106
#define        MSG_INTERACT_SYNTAX_ERROR        1107
#define        MSG_INTERACT_MULTIPLE_ENTRIES    1108
#define        MSG_INTERACT_NOT_MPOWNER         1109

#define        MSG_BUFSIZE_ERROR                1110
#define        MSG_MAXENTRIES_ERROR             1111
#define        MSG_CONFIG_ERR                   1112

#define        LOG_WRITEMODE_ERROR              1113  
#define        LOG_FILESIZE_ERROR               1114  
#define        LOG_FILEPATH_ERROR               1115 
#define        LOG_INTERNALBUFFER_ERROR         1116 
#define        LOG_FILEFORMAT_ERROR             1117  

#define        MSG_HBLOST_ERROR                 1118  
#define        MSG_HBRATE_ERROR                 1119  
#define        MSG_MSGNONBLOCK_ERROR            1120  
#define        EINSS7CP_MSG_SYSTIME_ERROR       1121  
#define        EINSS7CP_LOG_PROCPATH_ERROR      1122 
#define        EINSS7CP_LOG_FILEPROP_ERROR      1123  

#define        MSG_HBOFF_UNKNOWN_MODULE         1124
#define        MSG_HBOFF_NOT_MPOWNER            1125  
#define        MSG_HBOFF_SYNTAX_ERROR           1126 

#define        EINSS7CP_MSG_UXDOM_SYNTAX_ERROR  1127
#define        EINSS7CP_MSG_UXDOM_NOT_AVAILABLE 1128
#define        EINSS7CP_MSG_UXDOM_ARG_ERROR     1129
#define        EINSS7CP_LOG_BACKFILEPROP_ERROR  1130
#define        EINSS7CP_LOG_LD_IPA_ERROR        1131
#define        EINSS7CP_LOG_FLUSH_ERROR         1132
#define        EINSS7CP_LOGD_SOCKET_ERROR       1133
#define        EINSS7CP_LOGD_BIND_ERROR         1134
#define        EINSS7CP_LOG_NOT_INITIATED       1135   
#define        EINSS7CP_MSGINIT_DONE            1136
#define        EINSS7CP_NO_THREAD_CREATED       1137 
#define        EINSS7CP_STOP_SOCKET_ERROR       1138 
#define        EINSS7CP_SIGNAL_ERROR            1139 
#define        EINSS7CP_MSG_NO_BUFFER           1140
#define        EINSS7CP_MUTEX_INIT_FAILED       1141
#define        EINSS7CP_COND_INIT_FAILED        1142
#define        EINSS7CP_ANOTHER_THREAD_CLOSING_CON  1143
#define        EINSS7CP_AUTOSETUP_ADDR          1144

#define EINSS7CP_NO_IPC_EXIST                   1145
#define EINSS7CP_CREATE_IPC_ERROR               1146
#define EINSS7_EMPTY_BUFFER                     1147
#define EINSS7CP_READ_SOCKET_BUFFER_ERROR       1148
#define EINSS7CP_WRITE_SOCKET_BUFFER_ERROR      1149
#define EINSS7CP_SEND_BUFFER_ERROR              1150
#define EINSS7CP_MSG_SEND_FAIL                  1151
#define        EINSS7CP_MSGINIT_NOT_DONE        1152
#define EINSS7CP_NOT_IMPLEMENTED                1153
#define EINSS7CP_TIMER_NOT_IN_USE               1154

/* vxworks error codes */
#define MSG_ZBUF_CREATE_FAIL                    1400
#define MSG_ZBUF_DELETE_FAIL                    1401
#define MSG_ZBUF_SEND_FAIL                      1402

/* Codes for calling SysLog */
enum
{
    CP_MC_CONN_TO = 1170,
    CP_MC_RECV_TO,
    CP_MRL_FRC_PROTERR,
    CP_HB_SEND_FAIL,
    CP_HB_RECV_FAIL,
    CP_XM_BAD_USER_ID,
    CP_XRA_BAD_USER_ID,
    CP_XGB_BAD_USER_ID,
    CP_XFA_BAD_USER_ID,
    CP_LOG_MSG_TOO_LONG = 1179,
    CP_TI_BAD_USER_ID,
    CP_TRQ_BAD_USER_ID,
    CP_TIDRQ_BAD_USER_ID,
    CP_TCA_BAD_USER_ID,
    CP_TE_BAD_USER_ID,
    CP_XFB_BAD_USER_ID,
    CP_TIMER_NOT_IN_USE,
    EINSS7CP_TIMER_NOT_INIT,
    EINSS7CP_TIMER_START_FAIL,
    CP_TA_BAD_USER_ID
};


/*
 * Error in ss7main.c
 */
#define ERROR_DESTINATION_UNKNOWN 1501

#define         INIT_OK      0

/* 
 * Timer handling 
 */

#define        TIME_INIT_OK          0
 
#define        HELP_ENTRY_INSERT     100
    
/* 
 * Xtended memory handling 
 */

#define          XMEM_INIT_OK    0
#define          XFREE_OK        0
#define          XFREEALL_OK     0


/****************************************************************************/
/*                                                                          */
/*      Structures used in the CP API                                       */
/*                                                                          */
/****************************************************************************/ 
/* einrifr PR170 scope */              
/* How to handle new IP address in EINSS7CpSetIPAddressModule   */
typedef enum
{
    EINSS7_NONE  = 0,
    EINSS7_CLOSE = 1
} EINSS7_SOCK_ACTION_T;
              
typedef struct MSGBUF_TAG
{
    ULONG_T msgType_l;
    USHORT_T receiver;
    USHORT_T size;
    ULONG_T primitive;
    TEXT_T sendHead[MSG_HEADER_LENGTH];
    UCHAR_T data[MSG_MAX_DATALEN];  /* Note!!! the maximum data length is 
                       configurable, the definition is like 
                       this for compability reasons  */
} MSGBUF_T;

typedef struct MQENTRY_TAG
{
    struct MQENTRY_TAG *nextAll_sp;
    struct MQENTRY_TAG *prevAll_sp;
    struct MQENTRY_TAG *nextMP_sp;
    struct MQENTRY_TAG *prevMP_sp;
    struct MSGBUF_TAG msg;
} MQENTRY_T;

typedef struct MSG_TAG
{
        USHORT_T sender;
        USHORT_T receiver;
        UCHAR_T primitive;
        USHORT_T size;
        UCHAR_T *msg_p;
#ifdef EINSS7_THREADSAFE
        MQENTRY_T* cp_p;
#endif /* EINSS7_THREADSAFE */
} MSG_T;

typedef struct 
{
    BOOLEAN_T    mgmtAlarm;
    ULONG_T      statistics;
} REPORT_T;

#ifndef EINSS7_WINNT
typedef struct
{
    EINSS7SOCKET_T   descriptor;  
    ULONG_T events;
} APP_EVENT_T;
#else
typedef struct
{
        INT_T   events;
        EINSS7CPSOCKET_T socket;
        EINSS7CPHANDLE_T fHandle;
} APP_EVENT_T;
#endif /* EINSS7_WINNT */

typedef void* EINSS7CPMEMORYPOOL_T;

/****************************************************************************/
/*                                                                          */
/*      Structures used internally in the message handling                  */
/*      (Some of the API:s use them as well )                               */
/*                                                                          */
/****************************************************************************/  

typedef struct MQHEAD_TAG
{
    /* TR1823 Name: Dan Liljemark Date: 19991105 */
    /* MQHEAD_T: nextAll_sp => first_sp, prevAll_sp => last_sp */
    struct MQENTRY_TAG *first_sp;
    struct MQENTRY_TAG *last_sp;
/*    struct MQENTRY_TAG *nextAll_sp;
    struct MQENTRY_TAG *prevAll_sp;  */
    struct MQENTRY_TAG *nextMP_sp;
    struct MQENTRY_TAG *prevMP_sp;
#ifdef EINSS7_THREADSAFE
    EINSS7THREADMUTEX_T mutex_mt; 
    EINSS7THREADCOND_T  condv_cv; 
    ULONG_T waiting;  /* Number of threads wanting for a buffer */
    ULONG_T nrOfMsg;  /* Number of msg in the queue */
#endif /* EINSS7_THREADSAFE */
} MQHEAD_T;

/*********************************************************************/
/*                                                                   */
/*                         G L O B A L  O B J E C T S                */
/*                                                                   */
/*********************************************************************/

/*
 * These are visible for backwards compability reasons. Do not 
 * assign them directly. Use the new functions 
 * MsgSetBaseName() and MsgSetBaseQNum() instead.
 */
#ifndef EINSS7_THREADSAFE
extern CHAR_T mpBaseName[EINSS7CP_MPBASENAME_MAX_LEN]; 

extern USHORT_T baseQNum;

extern MQHEAD_T mqAvail_s;
#endif /* EINSS7_THREADSAFE */

/*********************************************************************/
/*                                                                   */
/*                         P R O T O T Y P E S                       */
/*                                                                   */
/*********************************************************************/

/*
 * IPC 
 */

void     MsgSetBaseName( CHAR_T* baseName);
void     MsgSetBaseQNum( USHORT_T baseQNum );
USHORT_T MsgMakeNonBlock(USHORT_T from, USHORT_T to);
USHORT_T MsgInit(USHORT_T maxentries);
USHORT_T EINSS7CpMsgInitNoSig(USHORT_T maxentries);
void EINSS7CpMsgClean( void );
void     MsgExit(void);
USHORT_T MsgOpen(USHORT_T userId);
USHORT_T MsgClose(USHORT_T userId);

USHORT_T MsgConn(USHORT_T userId,
                 USHORT_T otherId);
USHORT_T MsgConn_CW(USHORT_T userId,
                    USHORT_T otherId);
USHORT_T MsgRel(USHORT_T userId,
                USHORT_T otherId);

USHORT_T EINSS7CpMsgRetrieveBuffer(MQENTRY_T **entry_spp);
USHORT_T EINSS7CpMsgReleaseBuffer(MQENTRY_T *entry_sp);
USHORT_T EINSS7CpGetMsgBuffer(MSG_T* entry_sp);
USHORT_T EINSS7CpReleaseMsgBuffer(MSG_T* const entry_sp);

USHORT_T MsgSend(MSG_T *smg_sp);

USHORT_T MsgErrInfoGetMPOwners(MSG_T *msg_sp, 
                   USHORT_T *localMPO_p,
                   USHORT_T *remoteMPO_p );


#ifndef EINSS7_THREADSAFE
USHORT_T MsgSetEvents( EINSS7SOCKET_T descriptor, ULONG_T events );
USHORT_T MsgRemoveEvents( EINSS7SOCKET_T descriptor  );

USHORT_T MsgRecv(MSG_T *msr_sp);
USHORT_T MsgRecv_NW(MSG_T *msr_sp); 
USHORT_T MsgRecvEvent(MSG_T *msg_sp, APP_EVENT_T eventList[], 
              INT_T *listSize, SHORT_T timeout );
#endif /* EINSS7_THREADSAFE */

USHORT_T MsgEnableUser(USHORT_T fromId);
USHORT_T MsgDisableUser(USHORT_T fromId);
USHORT_T MsgAllowConn(USHORT_T fromId, USHORT_T toId);
USHORT_T MsgDisAllowConn(USHORT_T fromId, USHORT_T toId);


#ifdef EINSS7_THREADSAFE
USHORT_T EINSS7CpMsgRecv_r(MSG_T* const msg_sp,
                           const SHORT_T timeout);

USHORT_T EINSS7CpTimerRecv(MSG_T *msg_sp);

void EINSS7CleanCPAfterSignal_r(void);

#endif /* EINSS7_THREADSAFE */

EINSS7SOCKET_T EINSS7CpMsgObtainSocket(const USHORT_T fromId, 
                                       const USHORT_T toId);

#ifndef EINSS7_THREADSAFE
EINSS7SOCKET_T MsgObtainSocket(USHORT_T fromId, USHORT_T toId);
EINSS7SOCKET_T ObtainSocket(USHORT_T fromId, USHORT_T toId);  /* obsolete */

MQENTRY_T *RetreiveMsgBuffer(void);
#endif /* EINSS7_THREADSAFE */
USHORT_T MsgBuffSize(void);

#ifdef EINSS7_WINNT
EINSS7CPHANDLE_T EINSS7CpRegEvent(EINSS7CPHANDLE_T);
void EINSS7CpDeRegEvent(EINSS7CPHANDLE_T);
EINSS7CPHANDLE_T EINSS7CpRegEventSocket(EINSS7CPSOCKET_T);
void EINSS7CpDeRegEventSocket(EINSS7CPSOCKET_T);
#endif /* EINSS7_WINNT */

/*
 * Trace
 */

USHORT_T MsgTraceOn(USHORT_T userId);
USHORT_T MsgTraceOff(USHORT_T userId);


/* 
 * Timers
 */

USHORT_T TimeInit(USHORT_T userId, ULONG_T nTimers);
UCHAR_T *TimerReq(USHORT_T userId,
                  UCHAR_T *timerID_p,
                  USHORT_T usertimeValue);
UCHAR_T *TimerIdReq(USHORT_T userId,
                    UCHAR_T *timerInfo_p, 
                    UCHAR_T *timerId_p, 
                    USHORT_T timeValue);
BOOLEAN_T TimerChangeData(UCHAR_T *timerEntry, 
                          UCHAR_T *newtimerId_p,
                          UCHAR_T *newtimerInfo_p);
UCHAR_T *EINSS7CpTimerIdReqMilli(USHORT_T userId,
                                 UCHAR_T *timerInfo_p, 
                                 UCHAR_T *timerId_p, 
                                 USHORT_T timeValue);

void TimerCan(UCHAR_T *timerp_sp);
void TimerCanAll(USHORT_T userId);
void TimeExit(USHORT_T userId);
/* XVOL added 000427 */
USHORT_T EINSS7CpTimerAdd(USHORT_T userId, ULONG_T nTimers);
USHORT_T TimerAdd(USHORT_T userId,ULONG_T nTimers);
USHORT_T EINSS7CpTimerRestart(UCHAR_T *timerp_sp, 
                       USHORT_T newtimeValue);

/* 
 * Memory handling
 */

USHORT_T XMemInit_m(USHORT_T,SIZE_T,ULONG_T,ULONG_T);
UCHAR_T *XMalloc_m(USHORT_T, SIZE_T);
UCHAR_T *XRealloc_m(USHORT_T, UCHAR_T *, SIZE_T);
USHORT_T XMemFree(USHORT_T, UCHAR_T *);
UCHAR_T *XGetBlock_m(USHORT_T);
USHORT_T XFreeBlock_m(USHORT_T, UCHAR_T *);
void     XFreeAll_m(USHORT_T);

void EINSS7CpFreeAllMemory(void);

/* 
 * Common routines
 */

void Report(UCHAR_T userId, UCHAR_T alarmId, REPORT_T *item,
            UCHAR_T *alarmStatus, UCHAR_T statusLength);


void XSleep(SHORT_T seconds);
void EINSS7CpXSleepMilli(ULONG_T milliSeconds);


USHORT_T EINSS7CpGetIPAddressModule(const USHORT_T mpOwner, 
					  SOCKADDR_T** addr_pp,
					  SIZE_T* addrSize_p);

USHORT_T EINSS7CpRemoveIPAddressModule(const USHORT_T mpOwner,
                                       const EINSS7_SOCK_ACTION_T action);

USHORT_T EINSS7CpSetIPAddressModule(const USHORT_T mpOwner,  
                                    const SOCKADDR_T* addr_p,
                                    const SIZE_T addrSize,
                                    const EINSS7_SOCK_ACTION_T action);

#ifndef EINSS7_WINNT
USHORT_T PollEvent(      APP_EVENT_T eventList[], 
                             INT_T* listLength_p );
#endif /* EINSS7_WINNT */

USHORT_T EINSS7CpReadFast(INT_T sockNo, MQENTRY_T* mqp_sp,
                          const BOOLEAN_T firstRead);

USHORT_T EINSS7CpReadMsgsFromBuffer(const USHORT_T mpOwner);

USHORT_T EINSS7CpInitMemoryPool(const USHORT_T userId, 
                                SIZE_T blockSize, 
                                ULONG_T poolSize,
                                ULONG_T addSize,
                                EINSS7CPMEMORYPOOL_T* pool_p);

USHORT_T EINSS7CpAddBlocksToMemoryPool(const ULONG_T addSize,
                                       EINSS7CPMEMORYPOOL_T pool);

USHORT_T EINSS7CpXMemAddBlocks(const USHORT_T userId, 
                              const ULONG_T addSize);

USHORT_T EINSS7CpFreeMemoryPool(EINSS7CPMEMORYPOOL_T pool);

void* EINSS7CpGetPoolBlock(EINSS7CPMEMORYPOOL_T pool);

USHORT_T EINSS7CpFreePoolBlock(void *data_p,
                               EINSS7CPMEMORYPOOL_T pool);

USHORT_T EINSS7CpFlushSockBuffer(void);

BOOLEAN_T EINSS7CpReadBuffersEmpty();

/********************************************************************/
/* These are the functions to handle the double linked lists        */
/* used by timer and message functions.                             */
/*                                                                  */
/********************************************************************/

typedef struct LIST_TAG                  
{
  struct LIST_TAG    *next_sp;
  struct LIST_TAG    *prev_sp;
}DLIST_T;


void InitList(DLIST_T *p_sp);
void InsertList(DLIST_T *p1_sp,DLIST_T *p2_sp);
void MoveList(DLIST_T *bp_sp,DLIST_T *qp_sp);
void AppendList(DLIST_T *qe_sp,DLIST_T *qh_sp);
void RemoveListElement(DLIST_T *bp_sp);
void InserFirstInList(DLIST_T *qe_sp,DLIST_T *qh_sp);

#if defined (__cplusplus) || defined (c_plusplus)
}
#endif

#endif /* __SS7TMC_H__ */



