/*********************************************************************/
/* Don't remove these two lines, container depends on them!          */
/* Document Number: %Container% Revision: %Revision%                 */
/*                                                                   */
/* ss7util.h,v                                           */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* COPYRIGHT Ericsson Infotech AB 1998                               */
/*                                                                   */
/* The copyright to the computer program herein is the property of   */
/* Ericsson Infotech AB. The program may be used and/or  copied only */
/* with the written permission from Ericsson Infotech AB or in the   */
/* accordance with the terms and conditions stipulated in the        */
/* agreement/contract under which the program has been supplied.     */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Document Number:                 (Rev:    )                       */
/* 16/190 55-CAA 201 29 Ux                                           */
/*                                                                   */
/* Revision:                                                         */
/* @EINVER: ss7util.h,v 1.16 2000/06/30 09:42:59 Exp
 */
/*                                                                   */
/* Programmer:                                                       */
/* EIN/N/P Dennis Eriksson                                           */
/*                                                                   */
/* Purpose:                                                          */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Revision record:                                                  */
/*                                                                   */
/* 00-970625  EIN/N/P Dennis Eriksson                                */
/*                                                                   */
/* 01-981019  EIN/N/B Dan Liljemark                                  */
/*            Added more return values from EINSS7CpGetLastError.    */
/*            EINSS7Cp_GetLastError -> EINSS7CpGetLastError.         */
/*                                                                   */
/*-------------------------------------------------------------------*/
/* Notes:                                                            */
/*                                                                   */
/*********************************************************************/
#ifndef __SS7UTIL_H__
#define __SS7UTIL_H__
#if defined (__cplusplus) || defined (c_plusplus)
extern "C" {
#endif
  
/*********************************************************************/
/****                                                             ****/
/****                   I  N  C  L  U  D  E  S                    ****/
/****                                                             ****/
/*********************************************************************/
#include "ss7osdpn.h"

/*********************************************************************/
/****                                                             ****/
/****                  C  O  N  S  T  A  N  T  S                  ****/
/****                                                             ****/
/*********************************************************************/


/*
 *   Interrupt servers
 */
#define MAX_SIGNO 64
#define IS_PRI_LAST -128
#define IS_PRI_LO   -5
#define IS_PRI_NORM 0
#define IS_PRI_HI   5
#define IS_PRI_SYS  20
#define IS_PRI_RT   50
#define MAX_INTSERVNAME_LEN 16

/*
 *   ERROR CODES
 */
#define INTSRV_NOSUCHSIGNO 1201
#define INTSRV_NOSUCHSRV   1202
#define INTSRV_ADDSRVFAIL  1203
#define INTSRV_INUSE       1204

/* Return values from EINSS7CpGetLastError */
#ifndef EINSS7_WINNT
enum 
{
    EINSS7CP_EPERM = 0x01,
    EINSS7CP_ENOENT,
    EINSS7CP_ESRCH,
    EINSS7CP_EINTR,
    EINSS7CP_EIO,
    EINSS7CP_ENXIO,
    EINSS7CP_E2BIG,
    EINSS7CP_ENOEXEC,
    EINSS7CP_EBADF,
    EINSS7CP_ECHILD,

    EINSS7CP_EAGAIN = 11,
    EINSS7CP_ENOMEM,
    EINSS7CP_EACCES,
    EINSS7CP_EFAULT,
    EINSS7CP_ENOTBLK,
    EINSS7CP_EBUSY,
    EINSS7CP_EEXIST,
    EINSS7CP_EXDEV,
    EINSS7CP_ENODEV,
    EINSS7CP_ENOTDIR,

    EINSS7CP_EISDIR = 21,
    EINSS7CP_EINVAL,
    EINSS7CP_ENFILE,
    EINSS7CP_EMFILE,
    EINSS7CP_ENOTTY,
    EINSS7CP_ETXTBSY,
    EINSS7CP_EFBIG,
    EINSS7CP_ENOSPC,
    EINSS7CP_ESPIPE,
    EINSS7CP_EROFS,

    EINSS7CP_EMLINK = 31,
    EINSS7CP_EPIPE,
    EINSS7CP_EDOM,
    EINSS7CP_ERANGE,
    EINSS7CP_ENOMSG,
    EINSS7CP_EIDRM,
    EINSS7CP_ECHRNG,
    EINSS7CP_EL2NSYNC,
    EINSS7CP_EL3HLT,

    EINSS7CP_EL3RST = 41,
    EINSS7CP_ELNRNG,
    EINSS7CP_EUNATCH,
    EINSS7CP_ENOCSI,
    EINSS7CP_EL2HLT,
    EINSS7CP_EDEADLK,
    EINSS7CP_ENOLCK,
    EINSS7CP_ECANCELED,
    EINSS7CP_ENOTSUP,
    EINSS7CP_EDQUOT,

    EINSS7CP_EBADE = 51,
    EINSS7CP_EBADR,
    EINSS7CP_EXFULL,
    EINSS7CP_ENOANO,
    EINSS7CP_EBADRQC,
    EINSS7CP_EBADSLT,
    EINSS7CP_EDEADLOCK,
    EINSS7CP_EBFONT,
    EINSS7CP_ENOSTR,
    EINSS7CP_ENODATA,

    EINSS7CP_ETIME = 61,
    EINSS7CP_ENOSR,
    EINSS7CP_ENONET,
    EINSS7CP_ENOPKG,
    EINSS7CP_EREMOTE,
    EINSS7CP_ENOLINK,
    EINSS7CP_EADV,
    EINSS7CP_ESRMNT,
    EINSS7CP_ECOMM,
    EINSS7CP_EPROTO,

    EINSS7CP_EMULTIHOP = 71,
    EINSS7CP_EBADMSG,
    EINSS7CP_ENAMETOOLONG,
    EINSS7CP_EOVERFLOW,
    EINSS7CP_ENOTUNIQ,
    EINSS7CP_EBADFD,
    EINSS7CP_EREMCHG,
    EINSS7CP_ELIBACC,
    EINSS7CP_ELIBBAD,
    EINSS7CP_ELIBSCN,

    EINSS7CP_ELIBMAX = 81,
    EINSS7CP_ELIBEXEC,
    EINSS7CP_EILSEQ,
    EINSS7CP_ENOSYS,
    EINSS7CP_ELOOP,
    EINSS7CP_ERESTART,
    EINSS7CP_ESTRPIPE,
    EINSS7CP_ENOTEMPTY,
    EINSS7CP_EUSERS,
    EINSS7CP_ENOTSOCK,

    EINSS7CP_EDESTADDRREQ = 91,
    EINSS7CP_EMSGSIZE,
    EINSS7CP_EPROTOTYPE,
    EINSS7CP_ENOPROTOOPT,
    EINSS7CP_EPROTONOSUPPORT,
    EINSS7CP_ESOCKTNOSUPPORT,
    EINSS7CP_EOPNOTSUPP,
    EINSS7CP_EPFNOSUPPORT,
    EINSS7CP_EAFNOSUPPORT,
    EINSS7CP_EADDRINUSE,

    EINSS7CP_EADDRNOTAVAIL = 101,
    EINSS7CP_ENETDOWN,
    EINSS7CP_ENETUNREACH,
    EINSS7CP_ENETRESET,
    EINSS7CP_ECONNABORTED,
    EINSS7CP_ECONNRESET,
    EINSS7CP_ENOBUFS,
    EINSS7CP_EISCONN,
    EINSS7CP_ENOTCONN,
    EINSS7CP_ESHUTDOWN,

    EINSS7CP_ETOOMANYREFS = 111,
    EINSS7CP_ETIMEDOUT,
    EINSS7CP_ECONNREFUSED,
    EINSS7CP_EHOSTDOWN,
    EINSS7CP_EHOSTUNREACH = 115,

    /* In Solaris 2.5.1 and 2.6 EWOULDBLOCK is defined to be EAGAIN */
    /* Then EINSS7CP_EWOULDBLOCK is set to be the same as EINSS7CP_EAGAIN */
#if EWOULDBLOCK != EAGAIN
    EINSS7CP_EWOULDBLOCK,
#endif /* EWOULDBLOCK != EAGAIN */

    EINSS7CP_EALREADY = 117,
    EINSS7CP_EINPROGRESS,
    EINSS7CP_ESTALE,
#ifdef SCO_UNIX
    EINSS7CP_ELBIN,

    EINSS7CP_EDOTDOT,
#endif /* SCO_UNIX */
#ifdef AIX
    EINSS7CP_ENOTREADY = 122,
    EINSS7CP_EWRPROTECT,
    EINSS7CP_EFORMAT,
    EINSS7CP_ENOCONNECT,
    EINSS7CP_EDIST,
    EINSS7CP_EPROCLIM,
    EINSS7CP_EMEDIA,
    EINSS7CP_ESOFT,
    EINSS7CP_ENOATTR,

    EINSS7CP_ESAD = 131,
    EINSS7CP_ENOTRUST,
    EINSS7CP_ECLONEME,
#endif /* AIX */

    EINSS7CP_UNKNOWN_ERROR = 255
}; /* enum (return values from EINSS7CpGetLastError) */
#else
  /* Added for compatibility with Unix, only a few are used in the NT environment */
enum 
{
    EINSS7CP_EPERM = 0x01,
    EINSS7CP_ENOENT,
    EINSS7CP_ESRCH,
    EINSS7CP_EINTR = WSAEINTR,
    EINSS7CP_EIO = 0x05,
    EINSS7CP_ENXIO,
    EINSS7CP_E2BIG,
    EINSS7CP_ENOEXEC,
    EINSS7CP_EBADF = WSAEBADF,
    EINSS7CP_ECHILD = 10,

    EINSS7CP_EAGAIN = 11,
    EINSS7CP_ENOMEM,
    EINSS7CP_EACCES,
    EINSS7CP_EFAULT,
    EINSS7CP_ENOTBLK,
    EINSS7CP_EBUSY,
    EINSS7CP_EEXIST,
    EINSS7CP_EXDEV,
    EINSS7CP_ENODEV,
    EINSS7CP_ENOTDIR,

    EINSS7CP_EISDIR = 21,
    EINSS7CP_EINVAL,
    EINSS7CP_ENFILE,
    EINSS7CP_EMFILE,
    EINSS7CP_ENOTTY,
    EINSS7CP_ETXTBSY,
    EINSS7CP_EFBIG,
    EINSS7CP_ENOSPC,
    EINSS7CP_ESPIPE,
    EINSS7CP_EROFS,

    EINSS7CP_EMLINK = 31,
    EINSS7CP_EPIPE,
    EINSS7CP_EDOM,
    EINSS7CP_ERANGE,
    EINSS7CP_ENOMSG,
    EINSS7CP_EIDRM,
    EINSS7CP_ECHRNG,
    EINSS7CP_EL2NSYNC,
    EINSS7CP_EL3HLT,

    EINSS7CP_EL3RST = 41,
    EINSS7CP_ELNRNG,
    EINSS7CP_EUNATCH,
    EINSS7CP_ENOCSI,
    EINSS7CP_EL2HLT,
    EINSS7CP_EDEADLK,
    EINSS7CP_ENOLCK,
    EINSS7CP_ECANCELED,
    EINSS7CP_ENOTSUP,
    EINSS7CP_EDQUOT,

    EINSS7CP_EBADE = 51,
    EINSS7CP_EBADR,
    EINSS7CP_EXFULL,
    EINSS7CP_ENOANO,
    EINSS7CP_EBADRQC,
    EINSS7CP_EBADSLT,
    EINSS7CP_EDEADLOCK,
    EINSS7CP_EBFONT,
    EINSS7CP_ENOSTR,
    EINSS7CP_ENODATA,

    EINSS7CP_ETIME = 61,
    EINSS7CP_ENOSR,
    EINSS7CP_ENONET,
    EINSS7CP_ENOPKG,
    EINSS7CP_EREMOTE,
    EINSS7CP_ENOLINK,
    EINSS7CP_EADV,
    EINSS7CP_ESRMNT,
    EINSS7CP_ECOMM,
    EINSS7CP_EPROTO,

    EINSS7CP_EMULTIHOP = 71,
    EINSS7CP_EBADMSG,
    EINSS7CP_ENAMETOOLONG,
    EINSS7CP_EOVERFLOW,
    EINSS7CP_ENOTUNIQ,
    EINSS7CP_EBADFD,
    EINSS7CP_EREMCHG,
    EINSS7CP_ELIBACC,
    EINSS7CP_ELIBBAD,
    EINSS7CP_ELIBSCN,

    EINSS7CP_ELIBMAX = 81,
    EINSS7CP_ELIBEXEC,
    EINSS7CP_EILSEQ,
    EINSS7CP_ENOSYS,
    EINSS7CP_ELOOP,
    EINSS7CP_ERESTART,
    EINSS7CP_ESTRPIPE,
    EINSS7CP_ENOTEMPTY,
    EINSS7CP_EUSERS,
    EINSS7CP_ENOTSOCK,

    EINSS7CP_EDESTADDRREQ = 91,
    EINSS7CP_EMSGSIZE,
    EINSS7CP_EPROTOTYPE,
    EINSS7CP_ENOPROTOOPT,
    EINSS7CP_EPROTONOSUPPORT,
    EINSS7CP_ESOCKTNOSUPPORT,
    EINSS7CP_EOPNOTSUPP,
    EINSS7CP_EPFNOSUPPORT,
    EINSS7CP_EAFNOSUPPORT,
    EINSS7CP_EADDRINUSE,

    EINSS7CP_EADDRNOTAVAIL = 101,
    EINSS7CP_ENETDOWN,
    EINSS7CP_ENETUNREACH,
    EINSS7CP_ENETRESET,
    EINSS7CP_ECONNABORTED,
    EINSS7CP_ECONNRESET = WSAECONNRESET,
    EINSS7CP_ENOBUFS = WSAENOBUFS,
    EINSS7CP_EISCONN = WSAEISCONN,
    EINSS7CP_ENOTCONN = 109,
    EINSS7CP_ESHUTDOWN,

    EINSS7CP_ETOOMANYREFS = 111,
    EINSS7CP_ETIMEDOUT,
    EINSS7CP_ECONNREFUSED,
    EINSS7CP_EHOSTDOWN,
    EINSS7CP_EHOSTUNREACH = 115,

    EINSS7CP_EWOULDBLOCK = WSAEWOULDBLOCK,

    EINSS7CP_EALREADY = WSAEALREADY,
    EINSS7CP_EINPROGRESS = 118,
    EINSS7CP_ESTALE,

    EINSS7CP_UNKNOWN_ERROR = 255
}; /* enum (return values from EINSS7CpGetLastError) */

/* More return values from EINSS7CpGetLastError() */
#define EINSS7CP_ERROR_IO_PENDING	ERROR_IO_PENDING
#define EINSS7CP_WSAEINVAL		WSAEINVAL

/*
 * Options for MessageBoxes
 */
#define EINSS7CP_MSGBOXBLOCKING	       -1
#define EINSS7CP_MSGBOXNONBLOCKING	0

/*
 *   TAGs
 */
#define TAG_SKIP      ((ULONG_T)~3)
#define TAG_END       ((ULONG_T)~2)
#define TAG_DONE      ((ULONG_T)TAG_END) /*  TAG_DONE = TAG_END*/
#define TAG_IGNORE    ((ULONG_T)~1)
#define TAG_MORE      ((ULONG_T)~0)

#ifndef BOOL
#define BOOL INT_T
#endif /* BOOL */

#endif /* EINSS7_WINNT */


/* In Solaris 2.5.1 and 2.6 EWOULDBLOCK is defined to be EAGAIN */
/* Then EINSS7CP_EWOULDBLOCK is set to be the same as EINSS7CP_EAGAIN */
#if EWOULDBLOCK == EAGAIN
enum
{
    EINSS7CP_EWOULDBLOCK = EINSS7CP_EAGAIN
};
#endif /* EWOULDBLOCK != EAGAIN */

/*********************************************************************/
/****                                                             ****/
/****                    G  L  O  B  A  L  S                      ****/
/****                                                             ****/
/*********************************************************************/
  
/*********************************************************************/
/****                                                             ****/
/****                  S  T  R  U  C  T  U  R  E  S               ****/
/****                                                             ****/
/*********************************************************************/

#ifdef EINSS7_WINNT
/*
 *   TAGs
 */
struct TagItem {
  ULONG_T ti_Tag;
  ULONG_T ti_Data;
};
#endif /* EINSS7_WINNT */
   
/*
 *   Doubly linked lists
 */
typedef struct XL_MinNode
{
    /* Linkage part */
    struct XL_MinNode *xlmn_Succ;
    struct XL_MinNode *xlmn_Pred;

    /* Here starts Content part */
} XMINNODE_T;
typedef struct XL_Node
{
    /* Linkage part */
    struct XL_Node *xln_Succ;
    struct XL_Node *xln_Pred;

    /* Information part */
    UCHAR_T          xln_Type;  /* CR 215 rifr 991125 */
    TEXT_T           xln_Pri;
    TEXT_T           *xln_Name;

    /* Here starts the content part */
} XNODE_T;
typedef struct XL_MinList
{
    struct XL_MinNode *xlml_Head;
    struct XL_MinNode *xlml_Tail;
    struct XL_MinNode *xlml_TailPred;
} XMINLIST_T;
typedef struct XL_List
{
    struct XL_Node *xll_Head;
    struct XL_Node *xll_Tail;
    struct XL_Node *xll_TailPred;

    /* Information part */
    UCHAR_T  xll_Type;
    UCHAR_T  xll_Pad;
} XLIST_T;

/*
 *   Interrupt servers
 */
struct IntServ
{
  XNODE_T node;
  USHORT_T (*intServ_fp)(void *);
  void (*intServErrHandler_fp)(USHORT_T);
  void *servCntx;
  UCHAR_T intServName[MAX_INTSERVNAME_LEN];
};    

#ifdef EINSS7_WINNT
typedef struct 
{
	INT_T ngt;
} VERSION_INFO_T;
#endif /* EINSS7_WINNT */

/*********************************************************************/
/****                                                             ****/
/****                     T  Y  P  E  D  E  F  S                  ****/
/****                                                             ****/
/*********************************************************************/

/*  Interrupt servers */
typedef struct IntServ INTSERV_T;

#ifdef EINSS7_WINNT
/*
 *   TAGs 
 */
typedef struct TagItem TAGITEM_T;
#endif /* EINSS7_WINNT */

/*********************************************************************/
/****                                                             ****/
/****                 P  R  O  T  O  T  Y  P  E  S                ****/
/****                                                             ****/
/*********************************************************************/

#ifdef EINSS7_WINNT
/* 
 *   TAGs
 */
TAGITEM_T *AllocateTagItems(ULONG_T numTags);
TAGITEM_T *FindTagItem(ULONG_T tagValue,TAGITEM_T *tagList);
void FreeTagItems(TAGITEM_T *tagList);
ULONG_T GetTagData(ULONG_T tagValue,ULONG_T defaultVal,TAGITEM_T *tagList);
TAGITEM_T *NextTagItem(TAGITEM_T **tagListPtr);
#endif /* EINSS7_WINNT */

TEXT_T *UserIdToString(USHORT_T userId);
/*
 *   Interrupt servers
 */
void InitIntServers(void);
void SetIntServerData(INTSERV_T *sh,UCHAR_T *servName,CHAR_T prio,USHORT_T (*serv_fp)(void *),void (*errHandler_fp)(USHORT_T),void *context_p);
USHORT_T AddIntServer(INTSERV_T *sh,INT_T signo);
void RemIntServer(INTSERV_T *sh);
USHORT_T EINSS7CpRemIntServerByName(UCHAR_T *name, INTSERV_T **sh_spp);
INTSERV_T *FindIntServerByName(UCHAR_T *name);

#ifdef EINSS7_THREADSAFE
/* Mutex locking/ unlocking */
#define EINSS7CpEnterMutex_m(m,k,o,l)    EINSS7CpEnterMutex((m),(k),(o),(l))
#define EINSS7CpLeaveMutex_m(m,k,o,l)    EINSS7CpLeaveMutex((m),(k),(o),(l))

extern void EINSS7CpEnterMutex(EINSS7THREADMUTEX_T* mutex_mt_p,
                               EINSS7THREADKEY_T* hasMutex_key_p,
                               INT_T* oldState_p,
                               BOOLEAN_T* lockedThisTime_p);

extern void EINSS7CpLeaveMutex(EINSS7THREADMUTEX_T* mutex_mt_p,
                               EINSS7THREADKEY_T* hasMutex_key_p,
                               INT_T* oldState_p,
                               BOOLEAN_T lockedThisTime);
#else
#define EINSS7CpEnterMutex_m(m,k,o,l)    
#define EINSS7CpLeaveMutex_m(m,k,o,l)    

#endif /* EINSS7_THREADSAFE */

/*   
 *   Doubly linked lists 
 */
void XL_NewList(struct XL_List *list);
BOOLEAN_T XL_IsEmpty(struct XL_List *list);
void XL_AddHead(struct XL_List *list, struct XL_Node *node);
void XL_AddTail(struct XL_List *list, struct XL_Node *node);
struct XL_Node *XL_RemHead(struct XL_List *list);
struct XL_Node *XL_RemTail(struct XL_List *list);
void XL_Remove(struct XL_Node *node);
void XL_Insert(struct XL_List *list, struct XL_Node *node, struct XL_Node *pred);
void XL_Enqueue(struct XL_List *list, struct XL_Node *node);
struct XL_Node *XL_FindName(struct XL_List *list, TEXT_T *name);

#ifdef EINSS7_WINNT
/*
 *  Utilities 
 */
void EINSS7CpMsgBox(const CHAR_T *msgTxt, SHORT_T blocking);
USHORT_T GetVersionInfo(VERSION_INFO_T currentVer );
#endif /* EINSS7_WINNT */


/* To be used instead of errno */
extern USHORT_T EINSS7CpGetLastError(void);   

#if defined (__cplusplus) || defined (c_plusplus)
}
#endif
#endif /* Recursion semaphore */
