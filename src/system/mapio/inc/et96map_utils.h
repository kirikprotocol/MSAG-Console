/*********************************************************************/
/* Don't remove these two lines, container depends on them!          */
/* Document Number: %Container% Revision: %Revision%                 */
/*                                                                   */
/* et96map_utils.h,v                                                         */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/* et96map_utils.h                                                   */
/*                                                                   */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* COPYRIGHT Ericsson Infotech AB 1998	                 	     */
/*								     */
/* All rights reserved. The copyright of the computer program herein */
/* is the property of Ericsson Infocom Consultants AB, Sweden. The   */
/* program may be used and/or copied only with the permission of     */
/* Ericsson Infocom Consultants AB or in accordance with the 	     */
/* terms and conditions stipulated in the agreement/contract under   */
/* which the program has been supplied.				     */
/*								     */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Document Number:                                                  */
/* 10/190 55-CAA 201 45 Ux                                           */
/*                                                                   */
/* Revision:                                                         */
/* @EINVER: $RCfile$ 1.3 1998/05/26 12:05:46 Exp */
/*                                                                   */
/* Programmer:                                                       */
/* Paul Concannon                                                    */
/* Euristix Ltd.                                                     */
/*                                                                   */
/* Purpose:                                                          */
/* Provide common utility functions for the ET96 MAP Module.         */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/*                                                                   */
/* Revision record:                                                  */
/* 980406	Ulf Hellsten                            	     */
/* 981013       Anders Wallden  Added function UnpkMapSS7AddrssType  */
/*                                                                   */
/* Notes:                                                            */
/* <Any file specific text>                                          */
/*                                                                   */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
#ifndef __ET96MAP_UTILS_H__
#define __ET96MAP_UTILS_H__
/*********************************************************************/
/*                                                                   */
/*                    EXTERNAL DEFINITIONS                           */
/*                                                                   */
/*********************************************************************/

/* MAP tracing status */
extern BOOLEAN_T et96map_traceEnabled;

/* need to include this here as it contains the SysLog fn prototype */
#include "ss7log.h"
#include "et96map_consts.h"


/* operating system function for copying memory */
#define ET96MAP_MEMCPY(dest_p, orig_p, origLen) \
         memcpy(dest_p, orig_p, origLen)

/* operating system function for comparing memory */
#define ET96MAP_MEMCMP(dest_p, orig_p, origLen) \
         memcmp(dest_p, orig_p, origLen)

/* Used to log errors */
#define ET96MAP_ERROR(state, event, p1, p2, errorCode) \
        SysLog(ETSIMAPMODULEID, (CHAR_T *)__FILE__, __LINE__, state, event, p1, p2, errorCode)


/* Used for normal tracing */
#ifdef MAP_TRACE_ENABLED
#define ET96MAP_TRACE(state, event, p1, p2, p3) \
        Et96MapTrace((CHAR_T *)__FILE__, __LINE__, state, event, p1, p2, p3)
#else /* do nothing */
#define ET96MAP_TRACE(state, event, p1, p2, p3) ;
#endif /* MAP_TRACE_ENABLED */

USHORT_T Et96MapJoin(UCHAR_T byteHi, UCHAR_T byteLo);
ULONG_T Et96MapJoinLong(UCHAR_T byteHi,UCHAR_T byteMi1,UCHAR_T byteMi2,UCHAR_T byteLo);
void Et96MapSplitLong(ULONG_T thisLong,UCHAR_T *byteHi_p,UCHAR_T 
	       *byteMi1_p,UCHAR_T *byteMi2_p,UCHAR_T *byteLo_p);
void Et96MapSplit(USHORT_T dblByte, UCHAR_T *byteHi_p, UCHAR_T *byteLo_p);


UCHAR_T Et96MapBcdToOctetLen(UCHAR_T *buffer, UCHAR_T bcdLen);
UCHAR_T Et96MapOctetToBcdLen(UCHAR_T *buffer, UCHAR_T octetLen);

void Et96MapUnpkAddressType(UCHAR_T **buf_pp, ET96MAP_ADDRESS_T *struct_p);

UCHAR_T Et96MapPackAddressType(UCHAR_T *buf_sp, ET96MAP_ADDRESS_T *struct_p);

UCHAR_T Et96MapUnpkApiAddressType(UCHAR_T **buf_pp, ET96MAP_ADDRESS_T *struct_p,UCHAR_T len);

UCHAR_T Et96MapUnpkApiLocationInfoType(UCHAR_T **buf_pp, ET96MAP_LOCATION_INFO_T *struct_p,UCHAR_T len);
UCHAR_T Et96MapUnpkApiSCAddressType(UCHAR_T **buf_pp, ET96MAP_ADDRESS_T *struct_p);
UCHAR_T Et96MapPackApiAddressType(UCHAR_T *buf_sp, ET96MAP_ADDRESS_T *struct_p);

UCHAR_T Et96MapUnpkSS7AddressType(UCHAR_T **buf_pp, ET96MAP_SS7_ADDR_T *struct_p);



#endif /* __ET96MAP_UTILS_H__ */
