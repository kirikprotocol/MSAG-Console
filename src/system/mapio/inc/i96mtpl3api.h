/**********************************************************************/
/* GASK Required Information:                                         */
/* -------------------------                                          */
/* Document Number: %Container% Revision: %Revision%                  */
/*                                                                    */
/* Comment: Don't remove the above text, container depends on it!     */
/**********************************************************************/

/*--------------------------------------------------------------------*/
/*                                                                    */
/* COPYRIGHT Ericsson Infotech AB                                     */
/*                                                                    */
/* The copyright to the computer program herein is the property of    */
/* Ericsson Infotech AB. The program may be used and/or copied only   */
/* with the written permission from Ericsson Infotech AB or in the    */
/* accordance with the terms and conditions stipulated in the         */
/* agreement/contract under which the program has been supplied.      */
/*                                                                    */
/*--------------------------------------------------------------------*/

/**********************************************************************/
/* File information:          		                              */
/* -----------------           		                              */
/**********************************************************************/
/* 
   DOCUMENT NAME: i96mtpl3api.h,v
   
   DOCUMENT NUMBER: 4/190 55-CAA 901 735 Ux  

*/ 
/*--------------------------------------------------------------------*/
/* Template:24/1013-FEA 202 705 Uen, Rev B, 1999-10-06                */
/*--------------------------------------------------------------------*/
/*                                                                    */
/* History:                                                           */
/*  ----------------------------------------------------------------  */
/* | REV |    DATE    | PREP.   | DESCRIPTION                       | */
/*  ----------------------------------------------------------------  */
/* |  00 | 1994-09-08 | Tomas   | Initial issue                     | */
/* |     |            | Ekstedt |                                   | */
/*  ----------------------------------------------------------------  */
/* |  01 | 1997-12-02 | John    | API Update                        | */
/* |     |            | Lynas   |                                   | */
/*  ----------------------------------------------------------------  */
/* |  02 | 1998-12-05 |EHanratty| Updated to include provision of   | */ 
/* |     |            |         | Callback Registration. Added      | */
/* |     |            |         | EINSS7_ prefix to all external    | */      
/* |     |            |         | visible constants, global         | */
/* |     |            |         | variables, function names         | */ 
/* |     |            |         | and identifiers. Macros included  | */
/* |     |            |         | for reading/writing  SPC data.    | */
/*  ----------------------------------------------------------------  */
/* |  03 | 1999-08-20 | QINXAMC | Corrected TR1667                  | */
/* |     |            |         | MTP_L3 API rejects near max size  | */
/* |     |            |         | MSU's                             | */
/*  ----------------------------------------------------------------  */
/* |  05 | 2001-02-07 | ECEIAS  | Updated to include the DPD        | */
/* |     |            |         | Dispatcher functions.             | */
/*  ----------------------------------------------------------------  */
/* |  06 | 2001-03-30 | ECEIAS  | Updated header                    | */
/* |     |            |         |                                   | */
/*  ----------------------------------------------------------------  */
/* |  07 | 2001-04-20 | ECEIAS  | Changes related to PR 21/1751-FCP | */
/* |     |            |         | 1010976                           | */
/* |     |            |         |                                   | */
/*  ----------------------------------------------------------------  */


/**********************************************************************/

/*********************************************************************/
/****                                                             ****/
/****            R E C U R S I O N  S E M A P H O R E             ****/
/****                                                             ****/
/*********************************************************************

#ifndef  __MTPL3API__

#define __MTPL3API__



/*********************************************************************/
/*                                                                   */
/*            P A R A M E T E R   C O N S T A N T S                  */
/*                                                                   */
/*********************************************************************/

#if defined (__cplusplus) || (c_plusplus)
extern "C" {
#endif

/* Various Constants */

#define EINSS7_I96MTPL3_TUP_LABEL             4
#define EINSS7_I96MTPL3_ISUP_LABEL            5

#ifdef EINSS7_MTPL3_TWENTYFOURBITPC /* changed from CHINA_TUP */

#define EINSS7_I96MTPL3_LENGTH_OF_STANDARD        7
#define EINSS7_I96MTPL3_LENGTH_OF_TUP             8
#define EINSS7_I96MTPL3_LENGTH_OF_ISUP            8
#define EINSS7_I96MTPL3_MAX_DPC_VALUE      16777215
#define EINSS7_I96MTPL3_MAX_OPC_VALUE      16777215

#else

#define EINSS7_I96MTPL3_LENGTH_OF_STANDARD     4
#define EINSS7_I96MTPL3_LENGTH_OF_TUP          5
#define EINSS7_I96MTPL3_LENGTH_OF_ISUP         6
#define EINSS7_I96MTPL3_MAX_DPC_VALUE         16383
#define EINSS7_I96MTPL3_MAX_OPC_VALUE         16383

#endif


#define EINSS7_I96MTPL3_MAX_SLS_VALUE         15
#define EINSS7_I96MTPL3_MAX_CIC_VALUE       4095
#define EINSS7_I96MTPL3_MAX_DATA_LENGTH      272

#define EINSS7_I96MTPL3_BIND_OK                0
#define EINSS7_I96MTPL3_SAP_ALREADY_IN_USE     1

#define EINSS7_I96MTPL3_REM_USER_NOT_AVAIL_UNKNOWN   00   /* inclusion of cause codes  00 - 39 */
#define EINSS7_I96MTPL3_REM_USER_NOT_AVAIL_UNEQUIP   01
#define EINSS7_I96MTPL3_REM_USER_NOT_AVAIL_INACCESS  02
#define EINSS7_I96MTPL3_NETW_CONG_LEVEL0             36
#define EINSS7_I96MTPL3_NETW_CONG_LEVEL1             37
#define EINSS7_I96MTPL3_NETW_CONG_LEVEL2             38
#define EINSS7_I96MTPL3_NETW_CONG_LEVEL3             39

#define EINSS7_I96MTPL3_REM_USER_NOT_AVAIL 26
#define EINSS7_I96MTPL3_NETW_RESTRICTED    30
#define EINSS7_I96MTPL3_NETW_CONG          35

#define EINSS7_I96MTPL3_PHYSICAL_CIRCUIT   0
#define EINSS7_I96MTPL3_VIRTUAL_CIRCUIT    1

#define EINSS7_T96MTPL3_RS_UNAVAILABLE     0
#define EINSS7_I96MTPL3_RS_PROHIBITED      1
#define EINSS7_I96MTPL3_RS_CONTROLLED      2
#define EINSS7_I96MTPL3_RS_RESTRICTED      3
#define EINSS7_I96MTPL3_RS_AVAILABLE       4
#define EINSS7_I96MTPL3_RS_UNKNOWN_DPC     255

#define EINSS7_I96MTPL3_NIF_NOT_SUPPORTED  2

/*********************************************************************/
/*                                                                   */
/*           R E Q U E S T   R E T U R N   C O D E S                 */
/*                                                                   */
/*********************************************************************/

#define EINSS7_I96MTPL3_REQUEST_OK               0

#define EINSS7_I96MTPL3_INV_DEST_POINT_CODE      1
#define EINSS7_I96MTPL3_INV_ORIG_POINT_CODE      2
#define EINSS7_I96MTPL3_INV_CIRCUIT_ID_CODE      3
#define EINSS7_I96MTPL3_INV_SIGNAL_LINK_SEL      4
#define EINSS7_I96MTPL3_INV_MBIT                 5
#define EINSS7_I96MTPL3_INV_USERDATA_LENGTH      6
#define EINSS7_I96MTPL3_REQUEST_NOT_COMPLETED    7

/* return codes for callback functions  */
#ifdef EINSS7_FUNC_POINTER
/*********************************************************************/
/*								     */
/*           EINSS7-FUNC-POINTER   R E T U R N  C O D E S	     */
/*								     */
/*********************************************************************/

#define EINSS7_I96MTPL3_CALLBACK_FUNC_NOT_SET       250
#define EINSS7_I96MTPL3_CALLBACK_FUNC_ALREADY_SET   251
#endif


/*********************************************************************/
/*                                                                   */
/*           I N D I C A T I O N   R E T U R N   C O D E S           */
/*                                                                   */
/*********************************************************************/

#define EINSS7_I96MTPL3_IND_UNKNOWN_CODE      254
#define EINSS7_I96MTPL3_IND_LENGTH_ERROR      255

/*********************************************************************/
/*								     */
/*           R E A D   /   W R I T E   M A C R O S                   */
/*								     */
/*********************************************************************/

#ifdef EINSS7_MTPL3_TWENTYFOURBITPC

static const UCHAR_T MTP_RLABEL = 7;

#else   /* then FOURTEEN BIT... */

static const UCHAR_T MTP_RLABEL = 4; 

#endif



#ifdef EINSS7_MTPL3_TWENTYFOURBITPC

static const UCHAR_T SPC_SIZE = 3;

#else   /* then FOURTEEN BIT... */

static const UCHAR_T SPC_SIZE = 2;   

#endif


#ifdef EINSS7_TWENTYFOURBITPC 

/*  create routing label - write SPC data to buffers */ 

#define CreateLabel_m(outBuff_p, offset, destPointCode, origPointCode, signalLinkSel) \
(outBuff_p)[(offset)]   = TriLoByte_m((destPointCode));  \
(outBuff_p)[(offset)+1] = TriMiByte_m((destPointCode));  \
(outBuff_p)[(offset)+2] = TriHiByte_m((destPointCode));  \
(outBuff_p)[(offset)+3] = TriLoByte_m((origPointCode));  \
(outBuff_p)[(offset)+4] = TriMiByte_m((origPointCode));  \
(outBuff_p)[(offset)+5] = TriHiByte_m((origPointCode));  \
(outBuff_p)[(offset)+6] = (signalLinkSel)

#else /* 14 bit pc */

/*  create routing label - write SPC data to buffers */

#define CreateLabel_m(outBuff_p, offset, destPointCode, origPointCode, signalLinkSel) \
(outBuff_p)[(offset)]   = LoByte_m((destPointCode));  \
(outBuff_p)[(offset)+1] = ((HiByte_m((destPointCode)) & 0x3F) | (UCHAR_T)(((origPointCode) & 0x03) << 6)); \
(outBuff_p)[(offset)+2] = LoByte_m((origPointCode) >> 2); \
(outBuff_p)[(offset)+3] = (UCHAR_T)((((origPointCode) >> 10) & 0x0F) | (((signalLinkSel) << 4) & 0xF0))
        
#endif



/* read SPC data */

/* Extract first SPC from an array */

#ifdef EINSS7_MTPL3_TWENTYFOURBITPC
#define ExtractSPC1FromArray_m(inBuff_p, offset) \
((ULONG_T)((inBuff_p)[(offset)] + 256*((inBuff_p)[(offset)+1]) + 65536*((inBuff_p)[(offset)+2])))
#else   /* then FOURTEEN BIT... */   
#define ExtractSPC1FromArray_m(inBuff_p,offset) \
((ULONG_T)((inBuff_p)[(offset)] + 256*((inBuff_p)[(offset)+1] & 0x3F)))
#endif

/* Extract second SPC from array */

#ifdef EINSS7_MTPL3_TWENTYFOURBITPC
#define ExtractSPC2FromArray_m(inBuff_p, offset) \
(ExtractSPC1FromArray_m((inBuff_p),((offset)+SPC_SIZE)))
#else   /* then FOURTEEN BIT... */   
#define ExtractSPC2FromArray_m(inbuff_p, offset) \
((ULONG_T)((((inBuff_p)[(offset)+1] >> 6) & 0x03) + \
           4*((inBuff_p)[(offset)+2]) + \
           1024*((inBuff_p)[(offset)+3] & 0x0F)))
#endif

/*********************************************************************/
/*								     */
/*           M E S S A G E  S E N D  P R O T O T Y P E S             */
/*								     */
/*********************************************************************/

/**************    Programmed by qinxial.  BEGIN   **************/
USHORT_T EINSS7_I96MtpL3ApiSend(MSG_T *mqp_sp);
/**************    Programmed by qinxial.  END   **************/

/*********************************************************************/
/*                                                                   */
/*               R E Q U E S T    P R O T O T Y P E S                */
/*                                                                   */
/*********************************************************************/
 

USHORT_T EINSS7_I96MtpL3BindReq(UCHAR_T,
				USHORT_T);

USHORT_T EINSS7_I96MtpL3UnBindReq(UCHAR_T);

USHORT_T EINSS7_I96MtpL3TransferReq(ULONG_T,
				    ULONG_T,
				    UCHAR_T,
				    USHORT_T,
				    UCHAR_T,
                                    UCHAR_T,
                                    USHORT_T,
                                    UCHAR_T *);
                                    
/**************    Programmed by qinxial.  BEGIN   **************/
#ifdef EINSS7_NIF_INTERFACE 

USHORT_T EINSS7_I96DDBindReq(USHORT_T);

USHORT_T EINSS7_I96DDRSStatusReq(ULONG_T);

USHORT_T EINSS7_I96DDUnBindReq(); 

USHORT_T EINSS7_I96DDTransferReq(ULONG_T,
				    ULONG_T,
				    UCHAR_T,
				    USHORT_T,
				    UCHAR_T,
                                    UCHAR_T,
                                    USHORT_T,
                                    UCHAR_T *); 
                                    
#endif                                  
/**************    Programmed by qinxial.  END   **************/




/*********************************************************************/
/*                                                                   */
/*        C O N F I R M A T I O N   A N D  I N D I C A T I O N       */
/*                     P R O T O T Y P E S                           */
/*                                                                   */
/*********************************************************************/

USHORT_T EINSS7_I96MtpL3HandleInd(MSG_T*);

#ifdef EINSS7_FUNC_POINTER

/* procedure to register callback functions  */
/*creates the datatypes with pointer to function.  */

typedef USHORT_T (*EINSS7_M3BINDCONF_T)(UCHAR_T);

typedef USHORT_T (*EINSS7_M3TRANSFERIND_T)(ULONG_T,
				    ULONG_T,
				    UCHAR_T,
				    USHORT_T,
				    UCHAR_T,
                                    UCHAR_T,
                                    USHORT_T,
                                    UCHAR_T *);

typedef USHORT_T (*EINSS7_M3PAUSEIND_T)(ULONG_T
#ifdef EINSS7_MTPL3_MULTLSPC
                     ,ULONG_T
#endif
);

typedef USHORT_T (*EINSS7_M3RESUMEIND_T)(ULONG_T
#ifdef EINSS7_MTPL3_MULTLSPC
                     ,ULONG_T
#endif
);

typedef USHORT_T (*EINSS7_M3STATUSIND_T)(ULONG_T,
 			        UCHAR_T);



typedef USHORT_T (*EINSS7_M3INDERROR_T)(USHORT_T, 
				 MSG_T *);
				 


/**************    Programmed by qinxial.  BEGIN   **************/
#ifdef EINSS7_NIF_INTERFACE 

typedef USHORT_T (*EINSS7_DDBINDCONF_T)(UCHAR_T);

typedef USHORT_T (*EINSS7_DDTRANSFERIND_T)(ULONG_T,
				    ULONG_T,
				    UCHAR_T,
				    USHORT_T,
				    UCHAR_T,
                                    UCHAR_T,
                                    USHORT_T,
                                    UCHAR_T *);

typedef USHORT_T (*EINSS7_DDPAUSEIND_T)(ULONG_T
#ifdef EINSS7_MTPL3_MULTLSPC
                     ,ULONG_T
#endif
);

typedef USHORT_T (*EINSS7_DDRESUMEIND_T)(ULONG_T
#ifdef EINSS7_MTPL3_MULTLSPC
                     ,ULONG_T
#endif
);

typedef USHORT_T (*EINSS7_DDSTATUSIND_T)(ULONG_T,
 			        UCHAR_T,USHORT_T);
 			        
typedef USHORT_T (*EINSS7_DDRSSTATUSCONF_T)(ULONG_T,
 			        UCHAR_T);


#endif
/**************    Programmed by qinxial.  END   **************/				 

typedef struct EINSS7_I96MTPL3INIT
{
   EINSS7_M3BINDCONF_T       EINSS7_M3BindConf;
   EINSS7_M3TRANSFERIND_T    EINSS7_M3TransferInd;
   EINSS7_M3PAUSEIND_T       EINSS7_M3PauseInd;
   EINSS7_M3RESUMEIND_T      EINSS7_M3ResumeInd;
   EINSS7_M3STATUSIND_T      EINSS7_M3StatusInd;
   EINSS7_M3INDERROR_T       EINSS7_M3IndError;
   
/**************    Programmed by qinxial.  BEGIN   **************/
#ifdef EINSS7_NIF_INTERFACE 
   EINSS7_DDBINDCONF_T       EINSS7_DDBindConf;
   EINSS7_DDTRANSFERIND_T    EINSS7_DDTransferInd;
   EINSS7_DDPAUSEIND_T       EINSS7_DDPauseInd;
   EINSS7_DDRESUMEIND_T      EINSS7_DDResumeInd;
   EINSS7_DDSTATUSIND_T      EINSS7_DDStatusInd;
   EINSS7_DDRSSTATUSCONF_T   EINSS7_DDRSStatusConf;
   
#endif
/**************    Programmed by qinxial.  END   **************/

  
}EINSS7_I96MTPL3INIT_T;

#else

USHORT_T EINSS7_I96MtpL3BindConf(UCHAR_T);

USHORT_T EINSS7_I96MtpL3TransferInd(ULONG_T,
				    ULONG_T,
				    UCHAR_T,
				    USHORT_T,
				    UCHAR_T,
                                    UCHAR_T,
                                    USHORT_T,
                                    UCHAR_T *);

USHORT_T EINSS7_I96MtpL3PauseInd(ULONG_T
#ifdef EINSS7_MTPL3_MULTLSPC
                     ,ULONG_T
#endif
);

USHORT_T EINSS7_I96MtpL3ResumeInd(ULONG_T
#ifdef EINSS7_MTPL3_MULTLSPC
                     ,ULONG_T
#endif
);

USHORT_T EINSS7_I96MtpL3StatusInd(ULONG_T,
 			          UCHAR_T);



USHORT_T EINSS7_I96MtpL3IndError(USHORT_T, 
		                 MSG_T *);


/**************    Programmed by qinxial.  BEGIN   **************/
#ifdef EINSS7_NIF_INTERFACE 

USHORT_T EINSS7_I96DDBindConf(UCHAR_T);

USHORT_T EINSS7_I96DDTransferInd(ULONG_T,
				    ULONG_T,
				    UCHAR_T,
				    USHORT_T,
				    UCHAR_T,
                                    UCHAR_T,
                                    USHORT_T,
                                    UCHAR_T *);

USHORT_T EINSS7_I96DDPauseInd(ULONG_T
#ifdef EINSS7_MTPL3_MULTLSPC
                     ,ULONG_T
#endif
);

USHORT_T EINSS7_I96DDResumeInd(ULONG_T
#ifdef EINSS7_MTPL3_MULTLSPC
                     ,ULONG_T
#endif
);

USHORT_T EINSS7_I96DDStatusInd(ULONG_T,
 			          UCHAR_T);

USHORT_T EINSS7_I96DDRSStatusConf(ULONG_T,
 			          UCHAR_T);

#endif  /*  NIF_INTERFACE  */

#endif

/**************    Programmed by qinxial.  END   **************/


#ifdef EINSS7_FUNC_POINTER
/* to register callback functions  */
USHORT_T EINSS7_I96MtpL3RegFunc(EINSS7_I96MTPL3INIT_T*);
#endif


#if defined (__cplusplus) || (c_plusplus)
}
#endif


