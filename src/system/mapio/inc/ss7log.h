/*********************************************************************/
/* Don't remove these two lines, container depends on them!          */
/* Document Number: %Container% Revision: %Revision%                 */
/*                                                                   */
/* $RCSfile: ss7log.h,v                                              */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* COPYRIGHT Ericsson Infotech AB 1998                               */
/*                                                                   */
/* The copyright to the computer program herein is the property of   */
/* Ericsson Infotech AB. The program may be used and/or  copied only */
/* with the written permission from Ericsson Infotech  AB or in the  */
/* accordance with the terms and conditions stipulated in the        */
/* agreement/contract under which the program has been supplied.     */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Document Number:                 (Rev: PA1 )                      */
/* 9/190 55-CAA 201 29 Ux   (<revision>)                             */
/*                                                                   */
/* Revision:                                                         */
/* @EINVER: ss7log.h,v 1.15 2000/05/26 14:21:54 Exp */
/*                                                                   */
/* Programmer:                                                       */
/* EIN/N/S Per Larsson                                               */
/*                                                                   */
/* Purpose: Function prototypes for ss7log.c                         */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Revision record:                                                  */
/*                                                                   */
/* 01-980612  PA1 EIN/N/B Richard Frostmark                          */
/*            Added UDP interface                                    */
/*                                                                   */
/* 04-990322  EIN/N/E Dan Liljemark                                  */
/*            Updated after review.                                  */
/*            Inspection master plan: 32/17017-CAA20129              */
/*                                                                   */
/*-------------------------------------------------------------------*/
/* Notes:                                                            */
/*                                                                   */
/*********************************************************************/

#ifndef __SS7LOG_H__
#define __SS7LOG_H__


/*********************************************************************/
/****                                                             ****/
/****                   I  N  C  L  U  D  E  S                    ****/
/****                                                             ****/
/*********************************************************************/

#include "ss7cp.h"


#if defined (__cplusplus) || defined (c_plusplus)
extern "C" {
#endif

/*********************************************************************/
/****                                                             ****/
/****                  C  O  N  S  T  A  N  T  S                  ****/
/****                                                             ****/
/*********************************************************************/
#ifndef DAEMON_IPADDR
  #define DAEMON_IPADDR "localhost:7999"
#endif /* DAEMON_IPADDR */
#ifdef EINSS7_WINNT
	#define EINSS7CPLOGDNAME_C "EinSS7LogD.exe -c"
	#define EINSS7CPLOGDNAME "EinSS7LogD.exe"
#else
	#define EINSS7CPLOGDNAME "EinSS7LogD"
#endif /* EINSS7_WINNT */ 
/*********************************************************************/
/****                                                             ****/
/****                    G  L  O  B  A  L  S                      ****/
/****                                                             ****/
/*********************************************************************/

/*********************************************************************/
/****                                                             ****/
/****                      L  O  C  A  L  S                       ****/
/****                                                             ****/
/*********************************************************************/

/*********************************************************************/
/****                                                             ****/
/****                  S  T  R  U  C  T  U  R  E  S               ****/
/****                                                             ****/
/*********************************************************************/


/*********************************************************************/
/****                                                             ****/
/****                 P  R  O  T  O  T  Y  P  E  S                ****/
/****                                                             ****/
/*********************************************************************/

extern void SysLog(USHORT_T, CHAR_T*, USHORT_T, SHORT_T, SHORT_T, 
                    SHORT_T,SHORT_T, LONG_T);

extern void EINSS7CpSysLog(const USHORT_T userId, 
            const CHAR_T* fileName_p, 
            const USHORT_T line, 
            const LONG_T curState, 
            const LONG_T curEvent, 
            const LONG_T P1, 
            const LONG_T P2, 
            const LONG_T errorCode);

extern void SysTrace(USHORT_T, CHAR_T *, USHORT_T,SHORT_T, SHORT_T, 
                      SHORT_T,SHORT_T, LONG_T);

extern void EINSS7CpSysTrace(const USHORT_T userId, 
		      const CHAR_T* fileName_p, 
		      const USHORT_T line,   
		      const LONG_T curState, 
		      const LONG_T curEvent, 
		      const LONG_T p1, 
		      const LONG_T p2, 
		      const LONG_T  p3);

extern void SysMTrace(MQENTRY_T *, USHORT_T);

extern void SysXTrace(USHORT_T,TEXT_T *,USHORT_T);

USHORT_T EINSS7CpStartLogD(void);
USHORT_T EINSS7CpExitLogD(void);

#ifdef EINSS7_MTT
void SysMTSNote(TEXT_T *mtsstr); 
#endif /* EINSS7_MTT */

#if defined (__cplusplus) || defined (c_plusplus)
}
#endif


#endif /* Recursion semaphore */








