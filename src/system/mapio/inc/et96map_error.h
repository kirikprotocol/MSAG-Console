/*********************************************************************/
/* Don't remove these two lines, container depends on them!          */
/* Document Number: %Container% Revision: %Revision%                 */
/*                                                                   */
/* et96map_error.h,v                                                         */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/* Et96map_error.h                                                   */
/*                                                                   */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* COPYRIGHT Ericsson Infocom Consultants AB 1996		     */
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
/* 38/190 55-CAA 201 45 Ux                                           */
/*                                                                   */
/* Revision:                                                         */
/* @EINVER: $RCfile$ 1.5 2000/04/07 09:04:08 Exp */
/*                                                                   */
/* Programmer:                                                       */
/* Paul Concannon                                                    */
/* Euristix Ltd.                                                     */
/*                                                                   */
/* Purpose:                                                          */
/* Provide error defines for the ET96 MAP Module                     */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Revision record:                                                  */
/* <date>       <Author>         <Revision text>                     */
/*                                                                   */
/*                                                                   */
/* Notes:                                                            */
/* <Any file specific text>                                          */
/*                                                                   */
/*                                                                   */
/*                                                                   */
/*********************************************************************/

#ifndef __ET96MAP_ERROR_H__
#define __ET96MAP_ERROR_H_

/* Error codes used with the Common Parts SysLog function */

#define ET96MAP_UNSUPPORTED_PRIM     (1 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_ALLOC_FAILED         (2 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_NO_MACHINE           (3 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_NULL_POINTER         (4 + ETSIMAP_ERROR_OFFSET)  
#define ET96MAP_LIST_CORRUPT         (5 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_OUT_OF_BOUNDS        (6 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_DUP_DIALOGUE         (7 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_INVLD_EVENT          (8 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_INVLD_TRANS          (9 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_INVLD_ADDR           (10 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_VERSION_VIO          (11 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_INVLD_OP_TAG         (12 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_DECODE_FAILED        (13 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_UNREG_OP             (14 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_INVLD_SM_ID          (15 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_INVLD_DATA           (16 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_INVLD_FORMAT         (17 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_DECODE_ERR           (18 + ETSIMAP_ERROR_OFFSET) 
#define ET96MAP_ENCODE_ERR           (19 + ETSIMAP_ERROR_OFFSET) 
#define ET96MAP_INVLD_INVOKEID       (20 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_INVLD_OP_CODE        (21 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_PARM_NOTEXPECTED     (22 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_INVLD_DIALOGUEID     (23 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_INVLD_ERR_TAG        (24 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_INVLD_ERROR          (25 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_INVLD_ERR_PAR        (26 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_UNEX_TOUT            (27 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_PROTOCOL_VIO         (28 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_MSGSND_FAIL          (29 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_TIMER_OUTSTAND       (30 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_INVALID_MSG_PRIM     (31 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_INVLD_DCS            (32 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_BIND_STATE           (33 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_INIT_FAILED          (34 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_MINIT_FAILED         (35 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_MSGRECV_FAILED       (36 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_MSGOPEN_FAILED       (37 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_MSGREL_FAILED        (38 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_MSGCLOSE_FAILED      (39 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_MSGCONN_FAILED       (40 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_BAD_TCAP_BIND        (41 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_TCAP_API_ERR_IND     (42 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_TCAP_REQ_ERROR       (43 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_TIMER_REQ_FAIL       (44 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_XLATE_ERR            (45 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_INVLD_PROBCODE       (46 + ETSIMAP_ERROR_OFFSET) 
#define ET96MAP_ENCODE_FAILED        (47 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_USSD_BUSY            (48 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_UNEXPECT_VAL         (50 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_UNKNOWN_ALPHA        (51 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_BAD_CONFIG           (52 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_FILE_COULD_NOT_OPEN  (53 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_NO_COMPONENT         (54 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_USER_NOT_KNOWN_ON_SSN  (55 + ETSIMAP_ERROR_OFFSET)
#define ET96MAP_USER_NOT_OWNER_OF_THIS_DIALOGUE  (56 + ETSIMAP_ERROR_OFFSET)
#endif /* __ET96MAP_ERROR_H__ */
