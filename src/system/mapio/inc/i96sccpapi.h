/*********************************************************************
Don't remove these two lines, contain depends on them!                    
Document Number: %Container% Revision: %Revision%                         
*/

/*********************************************************************/
/*                                                                   */
/* i96sccpapi.h,v */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*								     */
/* COPYRIGHT Ericsson Infotech Consultants AB                        */
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
/* Document Number:           Rev:                                   */
/* 20/190 55-CAA 901 437 Ux    A                                     */
/*                                                                   */
/* Revision:                                                         */
/* @EINVER: i96sccpapi.h,v 1.6 1996/09/19 11:31:29 Exp */
/*                                                                   */
/* Programmer: Per Wessmark                                          */
/*                                                                   */
/* Purpose: Definitions for sccpapi.c                                */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Revision record:                                                  */
/*   00-930419  Per Wessmark	Initial issue			     */
/*   01-941028  J Lindvall	N_STATE_ind added                    */
/*   02-980806  Anders Wallden  Modification for ITU-T 03/93         */
/*   03-981102  Conrad Patton   Added EINSS7_ prefix to constants,   */
/*                              and function names, callback         */
/*                              functionality, 14-bit / 24bit SPC    */
/*                              constants.                           */
/*                                                                   */
/*   04-990809  Conrad Patton   Inserted different lengths for max.  */
/*                              value of primitive lengths (TR1555). */
/*                              TR1585: Increased value of           */
/*                              EINSS7_I96SCCP_MAX_REASON to 0x19.   */
/*                                                                   */
/*   05-000320  Gareth McKittrick                                    */
/*              Changes required resulting from new functionality in */
/*              S2000 including an increase in the size of the       */
/*              connectionId parameter from 2 bytes to 3.            */
/*              Change Tag: qinxgam                                  */
/*                                                                   */
/*   06-001220 Adele McKeown                                         */
/*             TR1585 MAX REASON is declared to be less than the     */
/*             reasons that are allowed.                             */
/*             Change tag: qinxaqu                                   */
/*                                                                   */
/*   07-001220 Adele McKeown                                         */
/*             TR1555 Invalid length errors for 24bit CONNECT_REQ/   */
/*             RESP and N_DISCONNECT_REQ messages.                   */
/*             Change tag: qinxaqu                                   */
/*                                                                   */
/*   08-010209 Adele McKeown                                         */
/*             CR38 Variables nor declared in header.                */
/*             Change tag: qinxaqu CR38                              */
/*                                                                   */
/*********************************************************************/
/* C++ compatibility definitions */
#ifndef __I96SCCPAPI_H__
#define __I96SCCPAPI_H__
#if defined (__cplusplus) || (c_plusplus)
extern "C" {
#endif

/*********************************************************************/
/*                                                                   */
/*            P A R A M E T E R   C O N S T A N T S                  */
/*                                                                   */
/*********************************************************************/

/* DaLi 960214: Compensate for possible SPC BEFORE address indicator */
#define EINSS7_I96SCCP_MIN_LENGTH_OF_GT         4
#define EINSS7_I96SCCP_MAX_LENGTH_OF_GT         12
#define EINSS7_I96SCCP_MIN_LENGTH_OF_ADR        1

/* xcpa: defined max address length and SPC length for 24-bit SPC.   */
#ifdef EINSS7_TWENTYFOURBITPC
#define EINSS7_I96SCCP_MAX_LENGTH_OF_ADR        20
#define EINSS7_I96SCCP_LENGTH_OF_SPC            3
#else
#define EINSS7_I96SCCP_MAX_LENGTH_OF_ADR        18
#define EINSS7_I96SCCP_LENGTH_OF_SPC            2
#endif

/* Various Fixed Part constants */
#define EINSS7_I96SCCP_LENGTH_OF_FIXED          10 /*qinxgam increased from 9 to 10 for 3 byte connectionId */ 
#define EINSS7_I96SCCP_MAX_SEQ_CONTROL          1
#define EINSS7_I96SCCP_MAX_RET_OPTION           1
#define EINSS7_I96SCCP_MAX_REASON               0x1A /* qinxaqu */
#define EINSS7_I96SCCP_MAX_MESS_PRIO            3
#define EINSS7_I96SCCP_MAX_MESS_PRI_IMP         11

/* qinxgam Added the new mapping of the Message Priority and Importance parameter constants */
#define EINSS7_I96SCCP_MSG_PRI_IMP_0             0  /*qinxgam */
#define EINSS7_I96SCCP_MSG_PRI_IMP_1             1  /*qinxgam */
#define EINSS7_I96SCCP_MSG_PRI_IMP_2             2  /*qinxgam */
#define EINSS7_I96SCCP_MSG_PRI_IMP_3             3  /*qinxgam */
#define EINSS7_I96SCCP_MSG_PRI_IMP_4             4  /*qinxgam */
#define EINSS7_I96SCCP_MSG_PRI_IMP_5             5  /*qinxgam */
#define EINSS7_I96SCCP_MSG_PRI_IMP_6             6  /*qinxgam */
#define EINSS7_I96SCCP_MSG_PRI_IMP_7             7  /*qinxgam */
#define EINSS7_I96SCCP_MSG_PRI_IMP_8             8  /*qinxgam */
#define EINSS7_I96SCCP_MSG_PRI_IMP_9             9  /*qinxgam */
#define EINSS7_I96SCCP_MSG_PRI_IMP_10            10 /*qinxgam */
#define EINSS7_I96SCCP_MSG_PRI_IMP_11            11 /*qinxgam */

/* Maximum lengths of requests */
/* xcpa: TR1555: inserted different lengths for different standards. */
#ifdef EINSS7_TWENTYFOURBITPC
#define EINSS7_I96SCCP_MAX_CONNECT_REQ_LENGTH   187
#define EINSS7_I96SCCP_MAX_CONNECT_RESP_LENGTH  167
#define EINSS7_I96SCCP_MAX_DISC_REQ_LENGTH      167
#define EINSS7_I96SCCP_MAX_UNITDATA_REQ_LENGTH  3928 /* qinxaqu */
#else
#define EINSS7_I96SCCP_MAX_CONNECT_REQ_LENGTH   183 /* qinxaqu */
#define EINSS7_I96SCCP_MAX_CONNECT_RESP_LENGTH  165 /* qinxaqu */
#define EINSS7_I96SCCP_MAX_DISC_REQ_LENGTH      165 /* qinxaqu */
#define EINSS7_I96SCCP_MAX_UNITDATA_REQ_LENGTH  3976 /* qinxaqu */
#endif

#define EINSS7_I96SCCP_MAX_DATA_REQ_LENGTH      3983 /* anwa */

/* Values for SCCP parameter names */
#define EINSS7_I96SCCP_PARAM_CALLED_ADDRESS	0x03
#define EINSS7_I96SCCP_PARAM_CALLING_ADDRESS	0x04
#define EINSS7_I96SCCP_PARAM_USER_DATA		0x0F
#define EINSS7_I96SCCP_PARAM_SEGMENTATION       0x10

/* qinxaqu - CR38 */
/* Values for SCCP parameter Originator */
#define EINSS7_I96SCCP_ORG_UNDEFINED		0x00
#define EINSS7_I96SCCP_ORG_SCCP_USER		0x01
#define EINSS7_I96SCCP_ORG_SCCP	         	0x02

/* Values for SCCP parameter Error Location, */
/* used in N_NOTICE_ind */
#define EINSS7_I96SCCP_LOCAL_ERROR              0
#define EINSS7_I96SCCP_REMOTE_ERROR             1

/* Values for 'result' in N_BIND_conf primitive */
#define EINSS7_I96SCCP_NB_SUCCESS               0x00
#define EINSS7_I96SCCP_SSN_ALREADY_IN_USE       0x01
#define EINSS7_I96SCCP_PROTOCOL_ERROR           0x02
#define EINSS7_I96SCCP_NB_RES_UNAVAIL           0x03
#define EINSS7_I96SCCP_SSN_NOT_ALLOWED          0x04
#define EINSS7_I96SCCP_SCCP_NOT_READY           0x05 /* qinxaqu CR38 */

/* Values for SCCP parameter Reason */
#define EINSS7_I96SCCP_END_USER_CONG	 	0x00
#define EINSS7_I96SCCP_END_USER_FAILURE	 	0x01
#define EINSS7_I96SCCP_SCCP_USER_ORIG	 	0x02
 
#define EINSS7_I96SCCP_ACCESS_CONG              0x03
#define EINSS7_I96SCCP_SS_CONG		 	0x04
#define EINSS7_I96SCCP_ACCESS_FAILURE	 	0x05

#define EINSS7_I96SCCP_ABN_COND			0x06
#define EINSS7_I96SCCP_NORM_COND                0x07
#define EINSS7_I96SCCP_NONTRANS_COND		0x06

#define EINSS7_I96SCCP_TRANS_COND               0x07
#define EINSS7_I96SCCP_INCOMP_INFO              0x08
#define EINSS7_I96SCCP_END_USER_ORIG		0x09

#define EINSS7_I96SCCP_ABN_COND_TRANS	 	0x0A
#define EINSS7_I96SCCP_ABN_COND_NONTRANS        0x0B
#define EINSS7_I96SCCP_SS_FAILURE               0x0C

#define EINSS7_I96SCCP_INVALID_STATE	 	0x0D
#define EINSS7_I96SCCP_DEST_ADDR_UNKNOWN        0x0E
#define EINSS7_I96SCCP_DEST_INACC_TRANS	 	0x0F

#define EINSS7_I96SCCP_DEST_INACC_NONTRANS      0x10
#define EINSS7_I96SCCP_UNDEFINED                0x11
#define EINSS7_I96SCCP_QOS_NOT_AVAIL_PERM       0x12

#define EINSS7_I96SCCP_QOS_NOT_AVAIL_TRANS      0x13
#define EINSS7_I96SCCP_REL_IN_PROG              0x14
#define EINSS7_I96SCCP_UNSP_TRANS               0x15

#define EINSS7_I96SCCP_UNSP_NONTRANS		0x16
#define EINSS7_I96SCCP_LOCAL_ERR                0x17
#define EINSS7_I96SCCP_NO_TRANSLATION		0x18

#define EINSS7_I96SCCP_UNKNOWN_LOCAL_SPC        0x19
#define EINSS7_I96SCCP_HOP_COUNT_ZERO           0x1A


/* Values for SCCP parameter Reason For Return */
#define EINSS7_I96SCCP_NO_TRANS_NATURE          0x00
#define EINSS7_I96SCCP_NO_TRANS_SPEC		0x01
#define EINSS7_I96SCCP_SUBSYS_CONG		0x02
#define EINSS7_I96SCCP_SUBSYS_FAILURE		0x03
#define EINSS7_I96SCCP_UNEQ_USER                0x04
#define EINSS7_I96SCCP_NETW_FAILURE		0x05
#define EINSS7_I96SCCP_NETW_CONGESTION		0x06
#define EINSS7_I96SCCP_UNQUALIFIED		0x07
#define EINSS7_I96SCCP_ERR_IN_MSG_TRANP		0x08
#define EINSS7_I96SCCP_ERR_IN_LOCAL_PROC        0x09
#define EINSS7_I96SCCP_DEST_CANNOT_REASS        0x0A
#define EINSS7_I96SCCP_FAILURE			0x0B
#define EINSS7_I96SCCP_HOP_COUNTER_VIOLATION    0x0C
#define EINSS7_I96SCCP_SEGMENT_NOT_SUPPORTED    0x0D
#define EINSS7_I96SCCP_SEGMENT_FAILURE          0x0E

/* Values for SCCP parameter Return Option */
#define EINSS7_I96SCCP_RET_OPT_OFF              0
#define EINSS7_I96SCCP_RET_OPT_ON               1

/* Values for SCCP parameter Sequence Control */
#define EINSS7_I96SCCP_SEQ_CTRL_OFF             0
#define EINSS7_I96SCCP_SEQ_CTRL_ON              1

/* Values for SCCP parameter User Status */
#define EINSS7_I96SCCP_USER_IN_SERVICE          0
#define EINSS7_I96SCCP_USER_OUT_OF_SERVICE      1


/* Other constants */
#define EINSS7_I96SCCP_MAX_SIZE                 256  /* Maximum size of array */
                                                     /* array sccpSender */

#define EINSS7_I96SCCP_INVALID_PRIMITIVE        99   

/*********************************************************************/
/*                                                                   */
/*           R E Q U E S T   R E T U R N   C O D E S                 */
/*                                                                   */
/*********************************************************************/

#define EINSS7_I96SCCP_REQUEST_OK               0

  /* Address errors are the sum of: */
#define EINSS7_I96SCCP_INV_CALLED               0
#define EINSS7_I96SCCP_INV_CALLING             10
#define EINSS7_I96SCCP_INV_RESPOND             20
  /* and: */
#define EINSS7_I96SCCP_INV_ADR_LENGTH           1
#define EINSS7_I96SCCP_INV_ADR_IND              2

  /* resulting in: */
#define EINSS7_I96SCCP_INV_CALLED_ADR_LENGTH    1
#define EINSS7_I96SCCP_INV_CALLED_ADR_IND       2
#define EINSS7_I96SCCP_INV_CALLING_ADR_LENGTH  11
#define EINSS7_I96SCCP_INV_CALLING_ADR_IND     12
#define EINSS7_I96SCCP_INV_RESPOND_ADR_LENGTH  21
#define EINSS7_I96SCCP_INV_RESPOND_ADR_IND     22

/* Other error constants */
#define EINSS7_I96SCCP_INV_USER_DATA_LENGTH    30
#define EINSS7_I96SCCP_INV_SEQ_CONTROL         31
#define EINSS7_I96SCCP_INV_RET_OPTION          32
#define EINSS7_I96SCCP_INV_CONNECT_ID          33
#define EINSS7_I96SCCP_INV_REASON              34
#define EINSS7_I96SCCP_INV_MESS_PRIO           35 /* anwa */
#define EINSS7_I96SCCP_INV_HOP_COUNT_USED      36 /* anwa */
#define EINSS7_I96SCCP_USER_ALREADY_BOUND      37 /* xcpa */    
#define EINSS7_I96SCCP_INV_MESS_PRI_IMP        38 /* xcer */

/*********************************************************************/
/*								     */
/*	     EINSS7_FUNC_POINTER   R E T U R N	 C O D E S           */
/*								     */
/*********************************************************************/
#ifdef EINSS7_FUNC_POINTER

#define EINSS7_I96SCCP_CALLBACK_FUNC_NOT_SET     250
#define EINSS7_I96SCCP_CALLBACK_FUNC_ALREADY_SET 251

#endif

/*********************************************************************/
/*                                                                   */
/*           I N D I C A T I O N   R E T U R N   C O D E S           */
/*                                                                   */
/*********************************************************************/

#define EINSS7_I96SCCP_IND_UNKNOWN_CODE        254
#define EINSS7_I96SCCP_IND_LENGTH_ERROR        255

/*********************************************************************/
/*                                                                   */
/*               R E Q U E S T    P R O T O T Y P E S                */
/*                                                                   */
/*********************************************************************/
 

USHORT_T EINSS7_I96SccpBindReq(UCHAR_T,USHORT_T,USHORT_T);

USHORT_T EINSS7_I96SccpUnBindReq(UCHAR_T);

 /*qinxgam Parameter 2 connectId  updated from USHORT_T to ULONG_T*/
USHORT_T EINSS7_I96SccpConnectReq(UCHAR_T, ULONG_T, UCHAR_T,
				  UCHAR_T, UCHAR_T *, UCHAR_T, 
				  UCHAR_T *, USHORT_T, UCHAR_T *);

 /*qinxgam Parameter 2 connectId  updated from USHORT_T to ULONG_T*/
USHORT_T EINSS7_I96SccpConnectResp(UCHAR_T, ULONG_T, UCHAR_T,
				   UCHAR_T, UCHAR_T *, USHORT_T,
				   UCHAR_T *);

 /*qinxgam Parameter 2 connectId  updated from USHORT_T to ULONG_T*/
USHORT_T EINSS7_I96SccpDataReq(UCHAR_T, ULONG_T, UCHAR_T,
			       USHORT_T, UCHAR_T *);

 /*qinxgam Parameter 2 connectId  updated from USHORT_T to ULONG_T*/
USHORT_T EINSS7_I96SccpDiscReq(UCHAR_T, ULONG_T, UCHAR_T,
			       UCHAR_T, UCHAR_T, UCHAR_T *,
			       USHORT_T, UCHAR_T *);

USHORT_T EINSS7_I96SccpUnitdataReq(UCHAR_T, UCHAR_T, UCHAR_T, 
				   UCHAR_T, UCHAR_T, BOOLEAN_T, 
				   UCHAR_T, UCHAR_T *, UCHAR_T, 
				   UCHAR_T *, USHORT_T, UCHAR_T *);

/*********************************************************************/
/*                                                                   */
/*                  H E L P   F U N C T I O N                        */
/*                     P R O T O T Y P E S                           */
/*                                                                   */
/*********************************************************************/


/*********************************************************************/
/*                                                                   */
/*        C O N F I R M A T I O N   A N D  I N D I C A T I O N       */
/*                     P R O T O T Y P E S                           */
/*                                                                   */
/*********************************************************************/

USHORT_T EINSS7_I96SccpHandleInd(MSG_T *);

#ifdef EINSS7_FUNC_POINTER

/* Creates the datatypes with "pointer to function".                 */

typedef USHORT_T (*EINSS7_I96SCCPNBINDCONF_T)(UCHAR_T, UCHAR_T, USHORT_T);

typedef USHORT_T (*EINSS7_I96SCCPNNOTICEIND_T)(UCHAR_T, UCHAR_T, UCHAR_T, 
					       UCHAR_T, UCHAR_T, UCHAR_T *,
					       UCHAR_T, UCHAR_T *, USHORT_T,
					       UCHAR_T *);

typedef USHORT_T (*EINSS7_I96SCCPNUNITDATAIND_T)(UCHAR_T, UCHAR_T, UCHAR_T, 
					         UCHAR_T, UCHAR_T, UCHAR_T *,
					         UCHAR_T, UCHAR_T *, USHORT_T,
					         UCHAR_T *);

/*qinxgam Parameter 2 connectId  updated from USHORT_T to ULONG_T*/
typedef USHORT_T (*EINSS7_I96SCCPNCONNECTIND_T)(UCHAR_T, ULONG_T, UCHAR_T,
					        UCHAR_T, UCHAR_T *, UCHAR_T, 
					        UCHAR_T *, USHORT_T,UCHAR_T *);

/*qinxgam Parameter 2 connectId  updated from USHORT_T to ULONG_T*/
typedef USHORT_T (*EINSS7_I96SCCPNCONNECTCONF_T)(UCHAR_T, ULONG_T, UCHAR_T,
					         UCHAR_T, UCHAR_T *, USHORT_T, 
					         UCHAR_T *);

/*qinxgam Parameter 2 connectId  updated from USHORT_T to ULONG_T*/
typedef USHORT_T (*EINSS7_I96SCCPNDATAIND_T)(UCHAR_T, ULONG_T, UCHAR_T,
					     USHORT_T, UCHAR_T *);

/*qinxgam Parameter 2 connectId  updated from USHORT_T to ULONG_T*/
typedef USHORT_T (*EINSS7_I96SCCPNDISCIND_T)(UCHAR_T, ULONG_T, UCHAR_T,
					     UCHAR_T, UCHAR_T, UCHAR_T *,
					     USHORT_T, UCHAR_T *);

typedef USHORT_T (*EINSS7_I96SCCPNSTATEIND_T)(UCHAR_T, UCHAR_T, UCHAR_T,
					      ULONG_T, ULONG_T);

typedef USHORT_T (*EINSS7_I96SCCPINDERROR_T)(USHORT_T, MSG_T *);

typedef struct EINSS7_I96SCCPINIT
{
  /* Pointer to user supplied indication / confirmation function.     */
  EINSS7_I96SCCPNBINDCONF_T     EINSS7_I96SccpBindConf;
  EINSS7_I96SCCPNNOTICEIND_T    EINSS7_I96SccpNoticeInd;
  EINSS7_I96SCCPNUNITDATAIND_T  EINSS7_I96SccpUnitdataInd;
  EINSS7_I96SCCPNCONNECTIND_T   EINSS7_I96SccpConnectInd;
  EINSS7_I96SCCPNCONNECTCONF_T  EINSS7_I96SccpConnectConf;
  EINSS7_I96SCCPNDATAIND_T      EINSS7_I96SccpDataInd;
  EINSS7_I96SCCPNDISCIND_T      EINSS7_I96SccpDiscInd;
  EINSS7_I96SCCPNSTATEIND_T     EINSS7_I96SccpStateInd;
  EINSS7_I96SCCPINDERROR_T      EINSS7_I96SccpIndError;
} EINSS7_I96SCCPINIT_T;

#else

/* Traditional indication / confirmation functions.                   */

USHORT_T EINSS7_I96SccpBindConf(UCHAR_T, UCHAR_T, USHORT_T);

USHORT_T EINSS7_I96SccpNoticeInd(UCHAR_T, UCHAR_T, UCHAR_T, 
				 UCHAR_T, UCHAR_T, UCHAR_T *,
				 UCHAR_T, UCHAR_T *, USHORT_T,
				 UCHAR_T *);

USHORT_T EINSS7_I96SccpUnitdataInd(UCHAR_T, UCHAR_T, UCHAR_T, 
				   UCHAR_T, UCHAR_T, UCHAR_T *,
				   UCHAR_T, UCHAR_T *, USHORT_T,
				   UCHAR_T *);

/*qinxgam Parameter 2 connectId  updated from USHORT_T to ULONG_T*/
USHORT_T EINSS7_I96SccpConnectInd(UCHAR_T, ULONG_T, UCHAR_T,
				  UCHAR_T, UCHAR_T *, UCHAR_T, 
				  UCHAR_T *, USHORT_T,UCHAR_T *);

/*qinxgam Parameter 2 connectId  updated from USHORT_T to ULONG_T*/
USHORT_T EINSS7_I96SccpConnectConf(UCHAR_T, ULONG_T, UCHAR_T,
				   UCHAR_T, UCHAR_T *, USHORT_T,
				   UCHAR_T *);

/*qinxgam Parameter 2 connectId  updated from USHORT_T to ULONG_T*/
USHORT_T EINSS7_I96SccpDataInd(UCHAR_T, ULONG_T, UCHAR_T,
			       USHORT_T, UCHAR_T *);

/*qinxgam Parameter 2 connectId  updated from USHORT_T to ULONG_T*/
USHORT_T EINSS7_I96SccpDiscInd(UCHAR_T, ULONG_T, UCHAR_T,
			       UCHAR_T, UCHAR_T, UCHAR_T *,
			       USHORT_T, UCHAR_T *);

USHORT_T EINSS7_I96SccpStateInd(UCHAR_T, UCHAR_T, UCHAR_T,
				ULONG_T, ULONG_T);

USHORT_T EINSS7_I96SccpIndError(USHORT_T, MSG_T *);

#endif

#ifdef EINSS7_FUNC_POINTER
/* Function used by application to register call back funtions.      */
USHORT_T EINSS7_I96SccpRegFunc(EINSS7_I96SCCPINIT_T *);
#endif

/* C++ compatibility definitions */
#if defined (__cplusplus) || (c_plusplus)
}
#endif
#endif /* __I96SCCPAPI_H__ */
