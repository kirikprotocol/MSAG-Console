/*********************************************************************/
/* GASK Required Information:                                        */
/* -------------------------                                         */
/* Document Number: %Container% Revision: %Revision%  */
/*                                                                   */
/* Comment: Don't remove the above text, container depends on it!    */
/*********************************************************************/
/*-------------------------------------------------------------------*/
/*                                                                   */
/* COPYRIGHT Ericsson Infotech AB                                    */
/*                                                                   */
/* The copyright to the computer program herein is the property of   */
/* Ericsson Infotech AB. The program may be used and/or copied only  */
/* with the written permission from Ericsson Infotech AB or in the   */
/* accordance with the terms and conditions stipulated in the        */
/* agreement/contract under which the program has been supplied.     */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*********************************************************************/
/* File information:                                                 */
/* -----------------                                                 */
/* 
   $RCSfile$
   $Revision$  
   $Date$ 
   $State$       
*/
/* Comment:  The above values are retrieved from CVS.                */
/*********************************************************************/
/* 
   Document Name:
   Document Number: 1/190 55-CAA 901 610 Ux 
*/ 
/* Template: 24/1013-FEA 202 705 Uen, Rev B, 1999-10-06              */
/*                                                                   */
/* History:                                                          */
/*                                                                   */
/* 00-20001009  Anders Peltomäki                                     */
/*              First version.                                       */
/*                                                                   */
/* 01-20001109  Mattias Claesson                                     */
/*              INC 2. Added BssmapLeCOInfo, BsslapMsPos and         */
/*              BsslapTaLayer3.                                      */
/*                                                                   */
/* 02-20001116  Anders Peltomäki                                     */
/*              INC 3. Added BsslapTaRequest, BsslapTaResponse,      */
/*              BsslapAbort (ind+req), BsslapReject, BsslapReset,    */
/*              BssmapLeReset (ind+req) and BssmapLeResetAck         */
/*              (ind+req).                                           */
/*                                                                   */
/* 03-20001201  Anders Peltomäki                                     */
/*              Changed BsslapTaLayer3 from "Ind"-function to        */
/*              unpacking function to be called by the user.         */
/*                                                                   */
/* 04-20010118  Per-Olof Lindgren                                    */
/*              Added BssmapLeUnpackCLInfo, and                      */
/*              EINSS7_BssmapLeCLInfoReq. Added defines for ITU93,   */
/*              ANSI, TTC and Chinese SCCP.                          */
/*                                                                   */
/*********************************************************************/

/*********************************************************************/
/****                                                             ****/
/****            R E C U R S I O N  S E M A P H O R E             ****/
/****                                                             ****/
/*********************************************************************/
#ifndef __BSSAPLEAPI_H__
#define __BSSAPLEAPI_H__

/*********************************************************************/
/****                                                             ****/
/****                      C + +  S E M A P H O R E               ****/
/****                                                             ****/
/**** Comment: The other part of this definition is included at   ****/
/****          the end of this file.                              ****/
/*********************************************************************/

#if defined (__cplusplus) || (c_plusplus)
extern "C" {
#endif

/*********************************************************************/
/****                                                             ****/
/****                   I  N  C  L  U  D  E  S                    ****/
/****                                                             ****/
/*********************************************************************/

#include        "ss7osdpn.h"
#include        "portss7.h"
#include        "ss7tmc.h"
#include        "ss7msg.h"

/*********************************************************************/
/****                                                             ****/
/****                  V  A  R  I  A  B  L  E  S                  ****/
/****                                                             ****/
/*********************************************************************/

/*********************************************************************/
/****                                                             ****/
/****                  C  O  N  S  T  A  N  T  S                  ****/
/****                                                             ****/
/*********************************************************************/

/* if we are running basic test we need the following SCCP constants*/
#ifdef BSSAPLE_BASIC_TEST
#ifdef EINSS7_BSSAPLE_ITU_SCCP_R6
  /* ITU SCCP R6 */
#define EINSS7_I93SCCP_INV_CALLED_ADR_LENGTH    1
#define EINSS7_I93SCCP_INV_CALLED_ADR_IND       2
#define EINSS7_I93SCCP_INV_CALLING_ADR_LENGTH  11
#define EINSS7_I93SCCP_INV_CALLING_ADR_IND     12
#define EINSS7_I93SCCP_INV_RESPOND_ADR_LENGTH  21
#define EINSS7_I93SCCP_INV_RESPOND_ADR_IND     22
#define EINSS7_I93SCCP_INV_USER_DATA_LENGTH    30
#define EINSS7_I93SCCP_INV_SEQ_CONTROL         31
#define EINSS7_I93SCCP_INV_RET_OPTION          32
#define EINSS7_I93SCCP_INV_CONNECT_ID          33
#define EINSS7_I93SCCP_INV_REASON              34
#define EINSS7_I93SCCP_INV_MESS_PRIO           35
#define EINSS7_I93SCCP_INV_HOP_COUNT_USED      36
#elif defined EINSS7_BSSAPLE_ANSI_SCCP
  /* ANSI SCCP */
#define EINSS7_A96SCCP_INV_CALLED_ADR_LENGTH    1
#define EINSS7_A96SCCP_INV_CALLED_ADR_IND       2
#define EINSS7_A96SCCP_INV_CALLING_ADR_LENGTH  11
#define EINSS7_A96SCCP_INV_CALLING_ADR_IND     12
#define EINSS7_A96SCCP_INV_RESPOND_ADR_LENGTH  21
#define EINSS7_A96SCCP_INV_RESPOND_ADR_IND     22
#define EINSS7_A96SCCP_INV_USER_DATA_LENGTH    30
#define EINSS7_A96SCCP_INV_SEQ_CONTROL         31
#define EINSS7_A96SCCP_INV_RET_OPTION          32
#define EINSS7_A96SCCP_INV_CONNECT_ID          33
#define EINSS7_A96SCCP_INV_REASON              34
#define EINSS7_A96SCCP_INV_MESS_PRIO           35
#define EINSS7_A96SCCP_INV_HOP_COUNT_USED      36
#define EINSS7_A96SCCP_USER_ALREADY_BOUND      37
#elif defined EINSS7_BSSAPLE_TTC_SCCP
  /* TTC SCCP */
#define EINSS7_T97SCCP_INV_CALLED_ADR_LENGTH    1
#define EINSS7_T97SCCP_INV_CALLED_ADR_IND       2
#define EINSS7_T97SCCP_INV_CALLING_ADR_LENGTH  11
#define EINSS7_T97SCCP_INV_CALLING_ADR_IND     12
#define EINSS7_T97SCCP_INV_RESPOND_ADR_LENGTH  21
#define EINSS7_T97SCCP_INV_RESPOND_ADR_IND     22
#define EINSS7_T97SCCP_INV_USER_DATA_LENGTH    30
#define EINSS7_T97SCCP_INV_SEQ_CONTROL         31
#define EINSS7_T97SCCP_INV_RET_OPTION          32
#define EINSS7_T97SCCP_INV_CONNECT_ID          33
#define EINSS7_T97SCCP_INV_REASON              34
#define EINSS7_T97SCCP_INV_MESS_PRIO           35
#define EINSS7_T97SCCP_INV_HOP_COUNT_USED      36
#define EINSS7_T97SCCP_USER_ALREADY_BOUND      37
#define EINSS7_T97SCCP_INV_MESS_PRI_IMP        38
#else
  /* ITU SCCP R7 */
#define EINSS7_I96SCCP_INV_CALLED_ADR_LENGTH    1
#define EINSS7_I96SCCP_INV_CALLED_ADR_IND       2
#define EINSS7_I96SCCP_INV_CALLING_ADR_LENGTH  11
#define EINSS7_I96SCCP_INV_CALLING_ADR_IND     12
#define EINSS7_I96SCCP_INV_RESPOND_ADR_LENGTH  21
#define EINSS7_I96SCCP_INV_RESPOND_ADR_IND     22
#define EINSS7_I96SCCP_INV_USER_DATA_LENGTH    30
#define EINSS7_I96SCCP_INV_SEQ_CONTROL         31
#define EINSS7_I96SCCP_INV_RET_OPTION          32
#define EINSS7_I96SCCP_INV_CONNECT_ID          33
#define EINSS7_I96SCCP_INV_REASON              34
#define EINSS7_I96SCCP_INV_MESS_PRIO           35
#define EINSS7_I96SCCP_INV_HOP_COUNT_USED      36
#define EINSS7_I96SCCP_USER_ALREADY_BOUND      37
#define EINSS7_I96SCCP_INV_MESS_PRI_IMP        38
#endif
#endif

/*********************************************************************/
/*    S C C P - C A R R I E R   C O N S T A N T S                    */
/*********************************************************************/
#define EINSS7_BSSAPLE_SCCPCARR_CONNREQ      0
#define EINSS7_BSSAPLE_SCCPCARR_CONNIND      1
#define EINSS7_BSSAPLE_SCCPCARR_CONNRESP     2
#define EINSS7_BSSAPLE_SCCPCARR_CONNCONF     3
#define EINSS7_BSSAPLE_SCCPCARR_DISCREQ      4
#define EINSS7_BSSAPLE_SCCPCARR_DISCIND      5
#define EINSS7_BSSAPLE_SCCPCARR_DATAREQ      6
#define EINSS7_BSSAPLE_SCCPCARR_DATAIND      7
#define EINSS7_BSSAPLE_SCCPCARR_UNITDATAREQ  8
#define EINSS7_BSSAPLE_SCCPCARR_UNITDATAIND  9
#define EINSS7_BSSAPLE_SCCPCARR_INVALID     10

/*********************************************************************/
/* M E S S A G E   T Y P E   I D E N T I F I E R   C O N S T A N T S */
/*********************************************************************/
#define EINSS7_BSSAPLE_BSSAP_DISCRIMINATION_INDICATOR 0x00
#define EINSS7_BSSAPLE_DTAP_DISCRIMINATION_INDICATOR 0x01

#define EINSS7_BSSLAP_MESTYPE_TAREQUEST    0x01
#define EINSS7_BSSLAP_MESTYPE_TARESPONSE   0x02
#define EINSS7_BSSLAP_MESTYPE_REJECT       0x0A
#define EINSS7_BSSLAP_MESTYPE_RESET        0x0B
#define EINSS7_BSSLAP_MESTYPE_ABORT        0x0C
#define EINSS7_BSSLAP_MESTYPE_TALAYER3     0x0D
#define EINSS7_BSSLAP_MESTYPE_MSPOSCOMMAND 0x0F
#define EINSS7_BSSLAP_MESTYPE_MSPOSRESP    0x10
#define EINSS7_BSSMAP_MESTYPE_PERFLOCRESP  0x2D
#define EINSS7_BSSMAP_MESTYPE_PERFLOCREQ   0x2B
#define EINSS7_BSSMAP_MESTYPE_PERFLOCABORT 0x2E
#define EINSS7_BSSMAP_MESTYPE_COINFO       0x2A
#define EINSS7_BSSMAP_MESTYPE_RESET        0x30
#define EINSS7_BSSMAP_MESTYPE_RESETACK     0x31
#define EINSS7_BSSMAP_MESTYPE_CLINFO       0x3A
    
/*********************************************************************/
/*    P A R A M E T E R   I E I   C O N S T A N T S                  */
/*********************************************************************/
#define EINSS7_BSSAPLE_IEI_TA                    0x01
#define EINSS7_BSSAPLE_IEI_CELL_ID               0x05
#define EINSS7_BSSLAP_IEI_CELL_ID                0x09
#define EINSS7_BSSLAP_IEI_CHANNEL_DESCRIPTION    0x10
#define EINSS7_BSSAPLE_IEI_CLASSMARK_INFO_TYPE_3 0x13
#define EINSS7_BSSAPLE_IEI_MEASUREMENT_REPORT    0x14
#define EINSS7_BSSAPLE_IEI_CAUSE                 0x18
#define EINSS7_BSSAPLE_IEI_RRLP_FLAG             0x19
#define EINSS7_BSSAPLE_IEI_RRLP                  0x1B
#define EINSS7_BSSAPLE_IEI_CHOSEN_CHANNEL        0x21
#define EINSS7_BSSAPLE_IEI_LCS_QOS               0x3E
#define EINSS7_BSSAPLE_IEI_LCS_PRIORITY          0x43
#define EINSS7_BSSAPLE_IEI_LOC_TYPE              0x44
#define EINSS7_BSSAPLE_IEI_LOC_ESTIM             0x45
#define EINSS7_BSSAPLE_IEI_POS_DATA              0x46
#define EINSS7_BSSAPLE_IEI_LCS_CAUSE             0x47
#define EINSS7_BSSAPLE_IEI_LCS_CLIENT_TYPE       0x48
#define EINSS7_BSSAPLE_IEI_BSSLAP_APDU           0x49
#define EINSS7_BSSMAPLE_IEI_NETWORK_ELEMENT_ID   0x4A
#define EINSS7_BSSAPLE_IEI_GPS_ASSIST_DATA       0x4B
#define EINSS7_BSSAPLE_IEI_DECIPH_KEYS           0x4C
#define EINSS7_BSSMAPLE_IEI_RETURN_ERROR_REQ     0x4D
#define EINSS7_BSSMAPLE_IEI_RETURN_ERROR_CAUSE   0x4E
#define EINSS7_BSSAPLE_IEI_SEGMENTATION          0x4F
#define EINSS7_BSSMAPLE_IEI_CAUSE                0x04
    
/*********************************************************************/
/*    P A R A M E T E R   A P D U   P R O T O C O L   I  D           */
/*********************************************************************/
#define EINSS7_BSSAPLE_APDU_PROTOCOL_ID_BSSLAP 0x01

/*********************************************************************/
/*    P A R A M E T E R   C O N S T A N T S                          */
/*********************************************************************/
#define EINSS7_BSSAPLE_DECKEY_LEN                          7

#define EINSS7_BSSAPLE_LCSQOS_LEN                          4

#define EINSS7_BSSAPLE_MAX_LOCESTIM_LEN                   20

#define EINSS7_BSSAPLE_MAX_CELLID_LEN                      8

#define EINSS7_BSSAPLE_LCS_CLIENT_TYPE_LEN                 1

#define EINSS7_BSSAPLE_LCS_PRIORITY_LEN                    1

#define EINSS7_BSSAPLE_LCS_QOS_LEN                         4

#define EINSS7_BSSAPLE_MIN_LOC_TYPE_LEN                    1
#define EINSS7_BSSAPLE_MAX_LOC_TYPE_LEN                    2

#define EINSS7_BSSAPLE_MIN_NETWORK_ELEMENT_ID_LEN          2
#define EINSS7_BSSAPLE_MAX_NETWORK_ELEMENT_ID_LEN          7

#define EINSS7_BSSAPLE_MIN_LCS_CAUSE_LEN                   1
#define EINSS7_BSSAPLE_MAX_LCS_CAUSE_LEN                   2

#define EINSS7_BSSMAPLE_MAX_CAUSE_LEN                      2

#define EINSS7_BSSAPLE_MIN_APDU_LEN                        3
#define EINSS7_BSSAPLE_MIN_SEGMENTATION_LEN                1
#define EINSS7_BSSAPLE_MAX_SEGMENTATION_LEN                3
    
static const UCHAR_T EINSS7_BSSAPLE_MIN_SSN = 2;
static const UCHAR_T EINSS7_BSSAPLE_MAX_SSN = 254;

#define EINSS7_BSSAPLE_MAX_OPTPARAM_LEN                  100

#define EINSS7_BSSAPLE_MAX_MEASURMENT_REPORT_LEN          20

#define EINSS7_BSSAPLE_NETWORK_ID_MAX_LEN                 10

/*********************************************************************/
/*    R E Q U E S T  /  R E S P O N S E   R E T U R N   C O D E S    */
/*********************************************************************/
static const USHORT_T EINSS7_BSSAPLE_RETURN_OK = 0;
static const USHORT_T EINSS7_BSSAPLE_INVALID_STRUCTURE_POINTER = 1;
static const USHORT_T EINSS7_BSSAPLE_INVALID_SUBSYSTEM_NUMBER = 2;
static const USHORT_T EINSS7_BSSAPLE_INVALID_SCCP_CARRIER  = 3;
static const USHORT_T EINSS7_BSSAPLE_INVALID_LOC_ESTIM_LEN = 4;
static const USHORT_T EINSS7_BSSAPLE_INVALID_LCS_CAUSE_LEN = 5;
static const USHORT_T EINSS7_BSSAPLE_INVALID_OPTPARAM_LEN  = 6;
static const USHORT_T EINSS7_BSSAPLE_INVALID_CALLED_ADDR   = 7;
static const USHORT_T EINSS7_BSSAPLE_INVALID_CALLING_ADDR  = 8;
static const USHORT_T EINSS7_BSSAPLE_INVALID_RESPOND_ADDR  = 9;
static const USHORT_T EINSS7_BSSAPLE_INVALID_POS_DATA  = 10;
static const USHORT_T EINSS7_BSSAPLE_INVALID_DEC_KEY_CIPH_FLAG  = 11;
static const USHORT_T EINSS7_BSSAPLE_INVALID_COINFO_LEN = 12;
static const USHORT_T EINSS7_BSSAPLE_INVALID_MSPOS_FLAG = 13;
static const USHORT_T EINSS7_BSSAPLE_INVALID_PROTOCOL_ID = 14;
static const USHORT_T EINSS7_BSSAPLE_INVALID_SEGMENTATION = 15;
static const USHORT_T EINSS7_BSSAPLE_MISSING_COINFO_MAND_PARAM = 16;
static const USHORT_T EINSS7_BSSAPLE_MISSING_MSPOS_MAND_PARAM = 17;
static const USHORT_T EINSS7_BSSAPLE_MISSING_ABORT_MAND_PARAM = 18;
static const USHORT_T EINSS7_BSSAPLE_INVALID_CAUSE_LEN = 19;
static const USHORT_T EINSS7_BSSAPLE_MISSING_RESET_MAND_PARAM = 20;
static const USHORT_T EINSS7_BSSAPLE_SCCP_INV_CALLED_ADR_LENGTH = 21;
static const USHORT_T EINSS7_BSSAPLE_SCCP_INV_CALLED_ADR_IND = 22;
static const USHORT_T EINSS7_BSSAPLE_SCCP_INV_CALLING_ADR_LENGTH = 23;
static const USHORT_T EINSS7_BSSAPLE_SCCP_INV_CALLING_ADR_IND = 24;
static const USHORT_T EINSS7_BSSAPLE_SCCP_INV_RESPOND_ADR_LENGTH = 25;
static const USHORT_T EINSS7_BSSAPLE_SCCP_INV_RESPOND_ADR_IND = 26;
static const USHORT_T EINSS7_BSSAPLE_SCCP_INV_USER_DATA_LENGTH = 27;
static const USHORT_T EINSS7_BSSAPLE_SCCP_INV_SEQ_CONTROL = 28;
static const USHORT_T EINSS7_BSSAPLE_SCCP_INV_RET_OPTION = 29;
static const USHORT_T EINSS7_BSSAPLE_SCCP_INV_CONNECT_ID = 30;
static const USHORT_T EINSS7_BSSAPLE_SCCP_INV_REASON = 31;
static const USHORT_T EINSS7_BSSAPLE_SCCP_INV_MESS_PRIO = 32;
static const USHORT_T EINSS7_BSSAPLE_SCCP_INV_HOP_COUNT_USED = 33;
static const USHORT_T EINSS7_BSSAPLE_TOO_LONG_DATA = 34;
static const USHORT_T EINSS7_BSSAPLE_NOT_TALAYER3 = 35;
static const USHORT_T EINSS7_BSSAPLE_MISSING_TALAYER3_MAND_PARAM = 36;
static const USHORT_T EINSS7_BSSAPLE_MISSING_APDU = 37;
static const USHORT_T EINSS7_BSSAPLE_MISSING_TA_LAYER3_PARAM_STRUCT = 38;
static const USHORT_T EINSS7_BSSAPLE_MISSING_CLINFO_MAND_PARAM = 39;
static const USHORT_T EINSS7_BSSAPLE_INVALID_CLINFO_LEN = 40;
static const USHORT_T EINSS7_BSSAPLE_SUCCESS_SEGMENTED_SEND = 41;
static const USHORT_T EINSS7_BSSAPLE_INVALID_CLINFO_MAND_PARAM = 42;
static const USHORT_T EINSS7_BSSAPLE_CALLBACK_FUNC_NOT_SET = 43;

/*********************************************************************/
/*    I N D I C A T I O N   R E T U R N   C O D E S                  */
/*********************************************************************/
static const USHORT_T EINSS7_BSSAPLE_IND_LENGTH_ERROR = 1;
static const USHORT_T EINSS7_BSSAPLE_IND_LOC_TYPE_LEN_ERR = 2;
static const USHORT_T EINSS7_BSSAPLE_IND_CELL_ID_LEN_ERR = 3;
static const USHORT_T EINSS7_BSSAPLE_IND_EXTRA_PARAM_BUF_FULL = 4;
static const USHORT_T EINSS7_BSSAPLE_IND_LCS_CAUSE_LEN_ERR = 5;
static const USHORT_T EINSS7_BSSAPLE_PERFLOCREQ_MISSING_MAND_PARAM = 6;
static const USHORT_T EINSS7_BSSAPLE_PERFLOCABORT_MISSING_MAND_PARAM = 7;
static const USHORT_T EINSS7_BSSAPLE_SCCP_ERROR = 8;
static const USHORT_T EINSS7_BSSAPLE_COINFO_MISSING_MAND_PARAM = 9;
static const USHORT_T EINSS7_BSSAPLE_IND_APDU_LEN_ERR = 10;
static const USHORT_T EINSS7_BSSAPLE_IND_SEGMENTATION_LEN_ERR = 11;
static const USHORT_T EINSS7_BSSAPLE_TARESPONSE_MISSING_MAND_PARAM = 12;
static const USHORT_T EINSS7_BSSAPLE_ABORT_MISSING_MAND_PARAM = 13;
static const USHORT_T EINSS7_BSSAPLE_REJECT_MISSING_MAND_PARAM = 14;
static const USHORT_T EINSS7_BSSLAP_RESET_MISSING_MAND_PARAM = 15;
static const USHORT_T EINSS7_BSSMAPLE_IND_CAUSE_LEN_ERR = 16;
static const USHORT_T EINSS7_BSSMAPLE_RESET_MISSING_MAND_PARAM = 17;
static const USHORT_T EINSS7_BSSAPLE_MSPOSRESPONSE_MISSING_MAND_PARAM = 18;
static const USHORT_T EINSS7_BSSAPLE_IND_RRLP_LEN_ERR = 19;
static const USHORT_T EINSS7_BSSMAPLE_NETWORK_ELEMENT_LEN_ERR = 20;
static const USHORT_T EINSS7_BSSMAPLE_RETURN_ERROR_REQ_LEN_ERR = 21;
static const USHORT_T EINSS7_BSSMAPLE_RETURN_ERROR_CAUSE_LEN_ERR = 22;
static const USHORT_T EINSS7_BSSAPLE_CLINFO_MISSING_MAND_PARAM = 23;

/*********************************************************************/
/*    M I S C   I N T E R N A L   C O N S T A N T S                  */
/*********************************************************************/
#define EINSS7_BSSAPLE_TEMP_DATA_SIZE       260

static const USHORT_T EINSS7_BSSAPLE_IEI_AND_LENGHT_LEN = 2;
static const USHORT_T EINSS7_BSSAPLE_IEI_LEN = 1;
static const USHORT_T EINSS7_BSSAPLE_MES_TYPE_LEN = 1;
static const USHORT_T EINSS7_BSSAPLE_CELLID_LEN = 2;
static const USHORT_T EINSS7_BSSAPLE_CHANNEL_DESC_LEN = 3;
static const USHORT_T EINSS7_BSSAPLE_APDU_HEADER_LEN = 3;

static const USHORT_T EINSS7_BSSLAP_APDU_DATA_HEADER_LEN = 4;
    
static const USHORT_T EINSS7_BSSLAP_NETWORK_ELEMENT_ID_HEADER_LEN = 3;
    
#define EINSS7_BSSAPLE_SEGM_MAX_APDUDATA_LEN      247
#define EINSS7_BSSAPLE_SEGM_MAX_CL_APDUDATA_LEN   239
#define EINSS7_BSSAPLE_MAX_BSSMAPLE_LEN           257 /* in one segment */
    
static const UCHAR_T EINSS7_BSSAPLE_MAX_NUMBER_OF_SEGMENTS = 16;

static const UCHAR_T EINSS7_BSSAPLE_SCCPDATA_OVERHEAD = 2;

static const UCHAR_T EINSS7_BSSAPLE_ONE_BIT_MAX_VALUE = 1;
static const UCHAR_T EINSS7_BSSAPLE_TWO_BIT_MAX_VALUE = 3;
static const UCHAR_T EINSS7_BSSAPLE_THREE_BIT_MAX_VALUE = 7;
static const UCHAR_T EINSS7_BSSAPLE_FOUR_BIT_MAX_VALUE = 15;
static const UCHAR_T EINSS7_BSSAPLE_FIVE_BIT_MAX_VALUE = 31;
static const UCHAR_T EINSS7_BSSAPLE_SIX_BIT_MAX_VALUE = 63;
static const UCHAR_T EINSS7_BSSAPLE_SEVEN_BIT_MAX_VALUE = 127;

static const UCHAR_T EINSS7_BSSAPLE_HI_4_BITS = 0xF0;
static const UCHAR_T EINSS7_BSSAPLE_LO_4_BITS = 0x0F;

#define EINSS7_BSSAPLE_RETURN_ERROR_PARAM_LEN 3

#define EINSS7_BSSAPLE_OPER_DISCR_POS      0
#define EINSS7_BSSAPLE_OPER_LENGTH_POS     1
#define EINSS7_BSSAPLE_OPER_MSGTYPE_POS    2

/*********************************************************************/
/****                                                             ****/
/****                  S  T  R  U  C  T  U  R  E  S               ****/
/****                                                             ****/
/*********************************************************************/

/*********************************************************************/
/*                                                                   */
/*             R E G   F U N C   I N I T                             */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/
/*********************************************************************/
/*                                                                   */
/*          E X T R A   O P T I O N A L   P A R A M E T E R S        */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_BSSAPLE_EXTRAOPTPARAM_TAG
{
    UCHAR_T     optParamLen;    /* extra optional parameters length  */
    UCHAR_T     *optParam_p;    /* extra optional parameters pointer */
} EINSS7_BSSAPLE_EXTRAOPTPARAM_T;

/*********************************************************************/
/*                                                                   */
/* T R A N S A C T I O N   P A R A M E T E R S                       */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_BSSAPLE_TRANSACT_TAG
{
    UCHAR_T  subSystemNumber;
    ULONG_T  connectionId;
    UCHAR_T  messPriImportance;
    UCHAR_T  hopCounterUsed;
    UCHAR_T  calledAddressLength;
    UCHAR_T  *calledAddress_p;
    UCHAR_T  callingAddressLength;
    UCHAR_T  *callingAddress_p;
    UCHAR_T  respondAddressLength;
    UCHAR_T  *respondAddress_p;   
    UCHAR_T  discReason;
    UCHAR_T  originator;
    UCHAR_T  referenceParam;
    UCHAR_T  sequenceControl;
}EINSS7_BSSAPLE_TRANSACT_T;

/*********************************************************************/
/*                                                                   */
/*  O P E R A T I O N   P A R A M E T E R S                          */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/

typedef struct EINSS7_BSSAPLE_GENERAL_DATA_TAG
{
    UCHAR_T  dataLen; /* Data length */
    UCHAR_T  *data_p; /* Data pointer */
} EINSS7_BSSAPLE_GENERAL_DATA_T;

typedef struct EINSS7_BSSAPLE_LOCATION_TYPE_TAG
{
    UCHAR_T   locationInfo;
    UCHAR_T   positioningMethod;
} EINSS7_BSSAPLE_LOCATION_TYPE_T;

typedef struct EINSS7_BSSAPLE_LCS_CLIENT_TYPE_TAG
{
    UCHAR_T   clientCategory;
    UCHAR_T   clientSubtype;
} EINSS7_BSSAPLE_LCS_CLIENT_TYPE_T;

typedef struct EINSS7_BSSAPLE_LCS_QOS_TAG
{
    UCHAR_T   vert;      /* one bit */
    UCHAR_T   vertSpare; /* 7 bits */
    UCHAR_T   horizontalAccuracyInd; /* one bit */
    UCHAR_T   horizontalAccuracy;    /* 7 bits */
    UCHAR_T   verticalAccuracyInd; /* one bit */
    UCHAR_T   verticalAccuracy;    /* 7 bits */
    UCHAR_T   responseTimerCategory; /* 2 bits */
    UCHAR_T   rtSpare;               /* 6 bits */
} EINSS7_BSSAPLE_LCS_QOS_T;

typedef struct EINSS7_BSSAPLE_POSITIONING_DATA_TAG
{
    UCHAR_T   posDataDiscriminator; /* Pos. Data Discriminator */
    UCHAR_T   numberOfPosMethods;    /* Number of Pos. Methods */
    UCHAR_T   *posMethods_p;
} EINSS7_BSSAPLE_POSITIONING_DATA_T;

typedef struct EINSS7_BSSAPLE_DECIPHERING_KEYS_TAG
{
    UCHAR_T   cipheringKeyFlag;      /* 1 bit  */
    UCHAR_T   cipheringKeyFlagSpare; /* 7 bits */
    UCHAR_T   currentDecKeyVal[EINSS7_BSSAPLE_DECKEY_LEN];
    UCHAR_T   nextDecKeyVal[EINSS7_BSSAPLE_DECKEY_LEN];
} EINSS7_BSSAPLE_DECIPHERING_KEYS_T;

typedef struct EINSS7_BSSAPLE_LCS_CAUSE_TAG
{
    UCHAR_T   causeValue;
    BOOLEAN_T diagnosticValueUsed;
    UCHAR_T   diagnosticValue;
} EINSS7_BSSAPLE_LCS_CAUSE_T;

typedef struct EINSS7_BSSAPLE_SEGMENTATION_TAG
{
  UCHAR_T segmentNumber;     /* 4 bits */
  UCHAR_T finalSegmentFlag;  /* 1 bit  */
  UCHAR_T segmentationSpare; /* 3 bits */
  BOOLEAN_T messageIdUsed;
  USHORT_T messageId;
} EINSS7_BSSAPLE_SEGMENTATION_T;

typedef struct EINSS7_BSSAPLE_APDU_DATA_TAG
{
    USHORT_T  dataLen; /* Data length */
    UCHAR_T  *data_p; /* Data pointer */
} EINSS7_BSSAPLE_APDU_DATA_T;

typedef struct EINSS7_BSSAPLE_APDU_TAG
{
    UCHAR_T protocolId;      /* 7 bits */
    UCHAR_T protocolIdSpare; /* 1 bit  */
    EINSS7_BSSAPLE_APDU_DATA_T *message_sp;
} EINSS7_BSSAPLE_APDU_T;

/*********************************************************************/
/*                                                                   */
/* P E R F L O C R E Q U E S T   I N D   P A R A M E T E R S         */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_BSSAPLE_PERFLOC_IND_TAG
{
    EINSS7_BSSAPLE_LOCATION_TYPE_T   *locationType_sp;
    EINSS7_BSSAPLE_GENERAL_DATA_T    *cellIdentifier_sp;
    EINSS7_BSSAPLE_GENERAL_DATA_T    *classmarkInfoType3_sp;
    EINSS7_BSSAPLE_LCS_CLIENT_TYPE_T *lcsClientType_sp;
    EINSS7_BSSAPLE_GENERAL_DATA_T    *chosenChannel_sp;
    UCHAR_T                          *lcsPriority_p;
    EINSS7_BSSAPLE_LCS_QOS_T         *lcsQos_sp;
    EINSS7_BSSAPLE_GENERAL_DATA_T    *gpsAssistanceData_sp;
    EINSS7_BSSAPLE_APDU_T            *bsslapApdu_sp;
    /* Extra Optional Parameters */
    EINSS7_BSSAPLE_EXTRAOPTPARAM_T   *extraOptParam_sp;
} EINSS7_BSSAPLE_PERFLOC_IND_T;

/*********************************************************************/
/*                                                                   */
/* P E R F L O C R E Q U E S T   R E S P   P A R A M E T E R S       */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_BSSAPLE_PERFLOC_RESP_TAG
{
    EINSS7_BSSAPLE_GENERAL_DATA_T     *locationEstimate_sp;
    EINSS7_BSSAPLE_POSITIONING_DATA_T *posData_sp;
    EINSS7_BSSAPLE_DECIPHERING_KEYS_T *decKey_sp;
    EINSS7_BSSAPLE_LCS_CAUSE_T        *lcsCause_sp;
    /* Extra Optional Parameters */
    EINSS7_BSSAPLE_EXTRAOPTPARAM_T *extraOptParam_sp;
} EINSS7_BSSAPLE_PERFLOC_RESP_T;

/*********************************************************************/
/*                                                                   */
/* P E R F L O C A B O R T   I N D   P A R A M E T E R S             */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_BSSAPLE_PERFLOCABORT_IND_TAG
{
    EINSS7_BSSAPLE_LCS_CAUSE_T *lcsCause_sp;
    /* Extra Optional Parameters */
    EINSS7_BSSAPLE_EXTRAOPTPARAM_T *extraOptParam_sp;
} EINSS7_BSSAPLE_PERFLOCABORT_IND_T;

/*********************************************************************/
/*                                                                   */
/*             R E S E T   P A R A M E T E R S                       */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_BSSMAPLE_RESET_TAG
{
    EINSS7_BSSAPLE_GENERAL_DATA_T *cause_sp;
    /* Extra Optional Parameters */
    EINSS7_BSSAPLE_EXTRAOPTPARAM_T *extraOptParam_sp;
} EINSS7_BSSMAPLE_RESET_T;

/*********************************************************************/
/*                                                                   */
/*             R E S E T   A C K   P A R A M E T E R S               */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_BSSMAPLE_RESET_ACK_TAG
{
    /* Extra Optional Parameters */
    EINSS7_BSSAPLE_EXTRAOPTPARAM_T *extraOptParam_sp;
} EINSS7_BSSMAPLE_RESET_ACK_T;

/*********************************************************************/
/*                                                                   */
/*            C O I N F O   R E Q   P A R A M E T E R S              */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_BSSAPLE_COINFO_REQ_TAG
{
    EINSS7_BSSAPLE_APDU_T          *bsslapApdu_sp;
    /* Extra Optional Parameters */
    EINSS7_BSSAPLE_EXTRAOPTPARAM_T *extraOptParam_sp;
} EINSS7_BSSAPLE_COINFO_REQ_T;

/*********************************************************************/
/*                                                                   */
/*            C O I N F O   I N D   P A R A M E T E R S              */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_BSSAPLE_COINFO_IND_TAG
{
    EINSS7_BSSAPLE_APDU_T          *bsslapApdu_sp;
    EINSS7_BSSAPLE_SEGMENTATION_T  *segmentation_sp;
    /* Extra Optional Parameters */
    EINSS7_BSSAPLE_EXTRAOPTPARAM_T *extraOptParam_sp;
} EINSS7_BSSAPLE_COINFO_IND_T;

/*********************************************************************/
/*                                                                   */
/*     N E T W O R K   E L E M E N T   I D   P A R A M E T E R S     */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_BSSAPLE_NETWORK_ELEMENT_ID_TAG
{
  UCHAR_T   idDiscriminator; /* 4 bits */
  UCHAR_T   idDiscrimSpare; /* 4 bits */
  UCHAR_T   networkElementIdLen;
  UCHAR_T   networkElementId[EINSS7_BSSAPLE_NETWORK_ID_MAX_LEN];
} EINSS7_BSSAPLE_NETWORK_ELEMENT_ID_T;

/*********************************************************************/
/*                                                                   */
/*            C L I N F O   R E Q   P A R A M E T E R S              */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_BSSAPLE_CLINFO_REQ_TAG
{
    EINSS7_BSSAPLE_NETWORK_ELEMENT_ID_T *sourceIdentity_sp;
    EINSS7_BSSAPLE_NETWORK_ELEMENT_ID_T *destIdentity_sp;
    EINSS7_BSSAPLE_APDU_T          *bsslapApdu_sp;
    EINSS7_BSSAPLE_SEGMENTATION_T  *segmentation_sp;
    UCHAR_T *returnErrorReq_p;
    UCHAR_T *returnErrorCause_p;
    /* Extra Optional Parameters */
    EINSS7_BSSAPLE_EXTRAOPTPARAM_T *extraOptParam_sp;
} EINSS7_BSSAPLE_CLINFO_REQ_T;

/*********************************************************************/
/*                                                                   */
/*            C L I N F O   I N D   P A R A M E T E R S              */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_BSSAPLE_CLINFO_IND_TAG
{
    EINSS7_BSSAPLE_NETWORK_ELEMENT_ID_T *sourceIdentity_sp;
    EINSS7_BSSAPLE_NETWORK_ELEMENT_ID_T *destIdentity_sp;
    EINSS7_BSSAPLE_APDU_T          *bsslapApdu_sp;
    EINSS7_BSSAPLE_SEGMENTATION_T  *segmentation_sp;
    /* Extra Optional Parameters */
    UCHAR_T *returnErrorReq_p;
    UCHAR_T *returnErrorCause_p;
    EINSS7_BSSAPLE_EXTRAOPTPARAM_T *extraOptParam_sp;
} EINSS7_BSSAPLE_CLINFO_IND_T;

/*********************************************************************/
/*                                                                   */
/*           M S   P O S   R E Q   P A R A M E T E R S               */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_BSSAPLE_MSPOS_COMMAND_REQ_TAG
{
    UCHAR_T flag;  /* 1 bit  */
    UCHAR_T flagSpare; /* 7 bits */
    EINSS7_BSSAPLE_APDU_DATA_T *infoRRLP_sp;
    /* Extra Optional Parameters */
    EINSS7_BSSAPLE_EXTRAOPTPARAM_T *extraOptParam_sp;
} EINSS7_BSSAPLE_MSPOS_COMMAND_REQ_T;

/*********************************************************************/
/*                                                                   */
/*          C O   M S   P O S   I N D   P A R A M E T E R S          */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_BSSAPLE_MSPOS_RESPONSE_IND_TAG
{
    UCHAR_T flag;  /* 1 bit  */
    UCHAR_T flagSpare; /* 7 bits */
    EINSS7_BSSAPLE_APDU_DATA_T     *infoRRLP_sp;
    UCHAR_T                        *timingAdvance_p;
    EINSS7_BSSAPLE_GENERAL_DATA_T  *measurementReport_sp;
    EINSS7_BSSAPLE_SEGMENTATION_T  *segmentation_sp;
    /* Extra Optional Parameters */
    EINSS7_BSSAPLE_EXTRAOPTPARAM_T *extraOptParam_sp;
} EINSS7_BSSAPLE_MSPOS_RESPONSE_IND_T;

/*********************************************************************/
/*                                                                   */
/*         T A   L A Y E R   3      P A R A M E T E R S              */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_BSSAPLE_TA_LAYER3_TAG
{
    UCHAR_T timingAdvance;
    UCHAR_T measurementReportLen;
    UCHAR_T measurementReport[EINSS7_BSSAPLE_MAX_MEASURMENT_REPORT_LEN];
    /* Extra Optional Parameters */
    UCHAR_T extraOptParamLen;
    UCHAR_T extraOptParam[EINSS7_BSSAPLE_MAX_OPTPARAM_LEN];
} EINSS7_BSSAPLE_TA_LAYER3_T;

/*********************************************************************/
/*                                                                   */
/*         T A   R E Q U E S T   R E Q   P A R A M E T E R S         */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_BSSAPLE_TA_REQUEST_REQ_TAG
{
    /* Extra Optional Parameters */
    EINSS7_BSSAPLE_EXTRAOPTPARAM_T *extraOptParam_sp;
} EINSS7_BSSAPLE_TA_REQUEST_REQ_T;

/*********************************************************************/
/*                                                                   */
/*       T A   R E S P O N S E   I N D   P A R A M E T E R S         */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_BSSAPLE_TA_RESPONSE_IND_TAG
{
    EINSS7_BSSAPLE_GENERAL_DATA_T *servingCellId_sp;
    UCHAR_T                       timingAdvance;
    EINSS7_BSSAPLE_GENERAL_DATA_T *measurementReport_sp;    
    /* Extra Optional Parameters */
    EINSS7_BSSAPLE_EXTRAOPTPARAM_T *extraOptParam_sp;
} EINSS7_BSSAPLE_TA_RESPONSE_IND_T;

/*********************************************************************/
/*                                                                   */
/*           R E J E C T   I N D   P A R A M E T E R S               */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_BSSAPLE_REJECT_IND_TAG
{
    UCHAR_T cause;
    /* Extra Optional Parameters */
    EINSS7_BSSAPLE_EXTRAOPTPARAM_T *extraOptParam_sp;
} EINSS7_BSSAPLE_REJECT_IND_T;

/*********************************************************************/
/*                                                                   */
/*          R E S E T   I  N D   P A R A M E T E R S                 */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_BSSAPLE_RESET_IND_TAG
{
    EINSS7_BSSAPLE_GENERAL_DATA_T *cellIdentity_sp;
    UCHAR_T                       timingAdvance;
    EINSS7_BSSAPLE_GENERAL_DATA_T *channelDescription_sp;
    UCHAR_T                       cause;
    EINSS7_BSSAPLE_GENERAL_DATA_T *measurementReport_sp;    
    /* Extra Optional Parameters */
    EINSS7_BSSAPLE_EXTRAOPTPARAM_T *extraOptParam_sp;
} EINSS7_BSSAPLE_RESET_IND_T;

/*********************************************************************/
/*                                                                   */
/*          A B O R T   R E Q / I N D   P A R A M E T E R S          */
/*                                                                   */
/*********************************************************************/
/*                                                                   */
/*             T Y P E   D E F I T I O N S                           */
/*                                                                   */
/*********************************************************************/
typedef struct EINSS7_BSSAPLE_ABORT_TAG
{
    UCHAR_T cause;
    /* Extra Optional Parameters */
    EINSS7_BSSAPLE_EXTRAOPTPARAM_T *extraOptParam_sp;
} EINSS7_BSSAPLE_ABORT_T;

/*********************************************************************/
/****                                                             ****/
/****                     T  Y  P  E  D  E  F  S                  ****/
/****                                                             ****/
/*********************************************************************/

/*********************************************************************/
/****                                                             ****/
/****                 P  R  O  T  O  T  Y  P  E  S                ****/
/****                                                             ****/
/*********************************************************************/

/*********************************************************************/
/* S U B S Y S T E M   I N F O R M A T I O N    P R O T O T Y P E S  */
/*********************************************************************/
extern USHORT_T EINSS7_BssapLeBindReq(UCHAR_T subSystemNumber, 
                                      USHORT_T userId,
                                      USHORT_T maxSegmSize);

extern USHORT_T EINSS7_BssapLeBindConf(UCHAR_T subSystemNumber, 
                                       UCHAR_T bindResult,
                                       USHORT_T maxSegmSize);

extern USHORT_T EINSS7_BssapLeUnBindReq(UCHAR_T subSystemNumber, 
                                        USHORT_T userId);

extern USHORT_T EINSS7_BssapLeStateInd(UCHAR_T subSystemNumber, 
                                       UCHAR_T userStatus,
                                       UCHAR_T affectedSSN,
                                       ULONG_T affectedSPC,
                                       ULONG_T localSPC,
                                       UCHAR_T subSystemMultiplicityInd); 

/*********************************************************************/
/*      T R A N S A C T I O N    P R O T O T Y P E S                 */
/*********************************************************************/
extern USHORT_T EINSS7_BssapLeConnectReq(UCHAR_T subSystemNumber,
                                         ULONG_T connectionId,
                                         UCHAR_T messagePriority,
                                         UCHAR_T hopCounterUsed,
                                         UCHAR_T calledAddressLength,
                                         UCHAR_T *calledAddress_p,
                                         UCHAR_T callingAddressLength,
                                         UCHAR_T *callingAddress_p,
                                         USHORT_T userDataLength,
                                         UCHAR_T *userData_p);

extern USHORT_T EINSS7_BssapLeConnectConf(UCHAR_T subSystemNumber,
                                          ULONG_T connectionId,
                                          UCHAR_T messagePriority,
                                          UCHAR_T respondAddressLength,
                                          UCHAR_T *respondAddress_p,
                                          USHORT_T userDataLength,
                                          UCHAR_T *userData_p);

extern USHORT_T EINSS7_BssapLeConnectInd(UCHAR_T subSystemNumber,
                                         ULONG_T connectionId,
                                         UCHAR_T messagePriority,
                                         UCHAR_T hopCounterUsed,
                                         UCHAR_T calledAddressLength,
                                         UCHAR_T *calledAddress_p,
                                         UCHAR_T callingAddressLength,
                                         UCHAR_T *callingAddress_p,
                                         USHORT_T userDataLength,
                                         UCHAR_T *userData_p);

extern USHORT_T EINSS7_BssapLeConnectResp(UCHAR_T subSystemNumber,
                                          ULONG_T connectionId,
                                          UCHAR_T messagePriority,
                                          UCHAR_T respondAddressLength,
                                          UCHAR_T *respondAddress_p,
                                          USHORT_T userDataLength,
                                          UCHAR_T *userData_p);

extern USHORT_T EINSS7_BssapLeDataReq(UCHAR_T subSystemNumber,
                                      ULONG_T connectionId,
                                      UCHAR_T messagePriority,
                                      USHORT_T  userDataLength,
                                      UCHAR_T   *userData_p);

extern USHORT_T EINSS7_BssapLeDataInd(UCHAR_T subSystemNumber,
                                      ULONG_T connectionId,
                                      UCHAR_T messagePriority,
                                      USHORT_T userDataLength,
                                      UCHAR_T *userData_p);

extern USHORT_T EINSS7_BssapLeDiscReq(UCHAR_T subSystemNumber,
                                      ULONG_T connectionId,
                                      UCHAR_T reason,
                                      UCHAR_T messagePriority,
                                      UCHAR_T respondAddressLength,
                                      UCHAR_T *respondAddress_p,
                                      USHORT_T userDataLength,
                                      UCHAR_T *userData_p);

extern USHORT_T EINSS7_BssapLeDiscInd(UCHAR_T subSystemNumber,
                                      ULONG_T connectionId,
                                      UCHAR_T reason,
                                      UCHAR_T originator,
                                      UCHAR_T respondAddressLength,
                                      UCHAR_T *respondAddress_p,
                                      USHORT_T userDataLength,
                                      UCHAR_T *userData_p);

extern USHORT_T EINSS7_BssapLeUnitdataReq(UCHAR_T subSystemNumber,
                                          UCHAR_T referenceParam,
                                          UCHAR_T sequenceControl,
                                          UCHAR_T messagePriority,
                                          UCHAR_T hopCounterUsed,
                                          UCHAR_T calledAddressLength,
                                          UCHAR_T *calledAddress_p,
                                          UCHAR_T callingAddressLength,
                                          UCHAR_T *callingAddress_p,
                                          USHORT_T userDataLength,
                                          UCHAR_T *userData_p);

extern USHORT_T EINSS7_BssapLeUnitdataInd(UCHAR_T subSystemNumber,
                                          UCHAR_T sequenceControl,
                                          UCHAR_T messagePriority,
                                          UCHAR_T calledAddressLength,
                                          UCHAR_T *calledAddress_p,
                                          UCHAR_T callingAddressLength,
                                          UCHAR_T *callingAddress_p,
                                          USHORT_T userDataLength,
                                          UCHAR_T *userData_p);

/*********************************************************************/
/*             F A I L U R E    P R O T O T Y P E S                  */
/*********************************************************************/
extern USHORT_T EINSS7_BssapLeIndError(UCHAR_T sccpCarrier,
                                       EINSS7_BSSAPLE_TRANSACT_T
                                       *transact_sp,
                                       USHORT_T error,
                                       USHORT_T userDataLength,
                                       UCHAR_T *userData_p);


/*********************************************************************/
/*           O P E R A T I O N    P R O T O T Y P E S                */
/*********************************************************************/
extern USHORT_T EINSS7_BssmapLePerfLocInd(UCHAR_T sccpCarrier,
                                          EINSS7_BSSAPLE_TRANSACT_T
                                          *transact_sp,
                                          EINSS7_BSSAPLE_PERFLOC_IND_T
                                          *perfLocInd_sp);

extern USHORT_T EINSS7_BssmapLePerfLocResp(UCHAR_T sccpCarrier,
                                           EINSS7_BSSAPLE_TRANSACT_T
                                           *transact_sp,
                                           EINSS7_BSSAPLE_PERFLOC_RESP_T
                                           *perfLocResp_sp);

extern USHORT_T EINSS7_BssmapLePerfLocAbortInd(UCHAR_T sccpCarrier,
                                               EINSS7_BSSAPLE_TRANSACT_T
                                               *transact_sp,
                                               EINSS7_BSSAPLE_PERFLOCABORT_IND_T
                                               *perfLocAbortInd_sp);

extern USHORT_T EINSS7_BssmapLeResetInd(UCHAR_T sccpCarrier,
                                        EINSS7_BSSAPLE_TRANSACT_T
                                        *transact_sp,
                                        EINSS7_BSSMAPLE_RESET_T
                                        *resetInd_sp);

extern USHORT_T EINSS7_BssmapLeResetReq(UCHAR_T sccpCarrier,
                                        EINSS7_BSSAPLE_TRANSACT_T
                                        *transact_sp,
                                        EINSS7_BSSMAPLE_RESET_T
                                        *resetReq_sp);

extern USHORT_T EINSS7_BssmapLeResetAckInd(UCHAR_T sccpCarrier,
                                           EINSS7_BSSAPLE_TRANSACT_T
                                           *transact_sp,
                                           EINSS7_BSSMAPLE_RESET_ACK_T
                                           *resetAckInd_sp);

extern USHORT_T EINSS7_BssmapLeResetAckReq(UCHAR_T sccpCarrier,
                                           EINSS7_BSSAPLE_TRANSACT_T
                                           *transact_sp,
                                           EINSS7_BSSMAPLE_RESET_ACK_T
                                           *resetAckReq_sp);

extern USHORT_T EINSS7_BssmapLeCOInfoInd(UCHAR_T sccpCarrier,
                                         EINSS7_BSSAPLE_TRANSACT_T
                                         *transact_sp,
                                         EINSS7_BSSAPLE_COINFO_IND_T
                                         *coInfoInd_sp);

extern USHORT_T EINSS7_BssmapLeCOInfoReq(UCHAR_T sccpCarrier,
                                         EINSS7_BSSAPLE_TRANSACT_T
                                         *transact_sp,
                                         EINSS7_BSSAPLE_COINFO_REQ_T
                                         *coInfoReq_sp);

extern USHORT_T EINSS7_BssmapLeCLInfoInd(UCHAR_T sccpCarrier,
                                         EINSS7_BSSAPLE_TRANSACT_T
                                         *transact_sp,
                                         EINSS7_BSSAPLE_CLINFO_IND_T
                                         *clInfoInd_sp);

extern USHORT_T EINSS7_BssmapLeCLInfoReq(UCHAR_T sccpCarrier,
                                         EINSS7_BSSAPLE_TRANSACT_T
                                         *transact_sp,
                                         EINSS7_BSSAPLE_CLINFO_REQ_T
                                         *clInfoReq_sp);

extern USHORT_T EINSS7_BsslapMsPosCommandReq(UCHAR_T sccpCarrier,
                                             EINSS7_BSSAPLE_TRANSACT_T
                                             *transact_sp,
                                             EINSS7_BSSAPLE_MSPOS_COMMAND_REQ_T
                                             *msPosReq_sp);

extern USHORT_T EINSS7_BsslapMsPosResponseInd(UCHAR_T sccpCarrier,
                                              EINSS7_BSSAPLE_TRANSACT_T
                                              *transact_sp,
                                              EINSS7_BSSAPLE_MSPOS_RESPONSE_IND_T
                                              *msPosInd_sp);

extern USHORT_T EINSS7_BsslapTaRequestReq(UCHAR_T sccpCarrier,
                                          EINSS7_BSSAPLE_TRANSACT_T
                                          *transact_sp,
                                          EINSS7_BSSAPLE_TA_REQUEST_REQ_T
                                          *taReq_sp);

extern USHORT_T EINSS7_BsslapTaResponseInd(UCHAR_T sccpCarrier,
                                           EINSS7_BSSAPLE_TRANSACT_T
                                           *transact_sp,
                                           EINSS7_BSSAPLE_TA_RESPONSE_IND_T
                                           *taInd_sp);

extern USHORT_T EINSS7_BsslapRejectInd(UCHAR_T sccpCarrier,
                                       EINSS7_BSSAPLE_TRANSACT_T
                                       *transact_sp,
                                       EINSS7_BSSAPLE_REJECT_IND_T
                                       *rejectInd_sp);

extern USHORT_T EINSS7_BsslapResetInd(UCHAR_T sccpCarrier,
                                      EINSS7_BSSAPLE_TRANSACT_T
                                      *transact_sp,
                                      EINSS7_BSSAPLE_RESET_IND_T
                                      *resetInd_sp);

extern USHORT_T EINSS7_BsslapAbortInd(UCHAR_T sccpCarrier,
                                      EINSS7_BSSAPLE_TRANSACT_T
                                      *transact_sp,
                                      EINSS7_BSSAPLE_ABORT_T
                                      *abortInd_sp);

extern USHORT_T EINSS7_BsslapAbortReq(UCHAR_T sccpCarrier,
                                      EINSS7_BSSAPLE_TRANSACT_T
                                      *transact_sp,
                                      EINSS7_BSSAPLE_ABORT_T
                                      *abortReq_sp);

extern USHORT_T EINSS7_BsslapUnpackTaLayer3(EINSS7_BSSAPLE_APDU_T 
                                            *apdu_sp,
                                            EINSS7_BSSAPLE_TA_LAYER3_T
                                            *taLayer3Param_sp);
#ifdef EINSS7_BSSAPLE_FUNC_POINTER
/* creates the datatypes with "pointer to function" */

typedef USHORT_T (*EINSS7_BSSAPLEBINDCONF_T) (UCHAR_T subSystemNumber, 
                                              UCHAR_T bindResult,
                                              USHORT_T maxSegmSize);

typedef USHORT_T (*EINSS7_BSSAPLECONNECT_IND_T) (UCHAR_T subSystemNumber,
                                                 ULONG_T connectionId,
                                                 UCHAR_T messagePriority,
                                                 UCHAR_T hopCounterUsed,
                                                 UCHAR_T calledAddressLength,
                                                 UCHAR_T *calledAddress_p,
                                                 UCHAR_T callingAddressLength,
                                                 UCHAR_T *callingAddress_p,
                                                 USHORT_T userDataLength,
                                                 UCHAR_T *userData_p);

typedef USHORT_T (*EINSS7_BSSAPLECONNECT_CONF_T) (UCHAR_T subSystemNumber,
                                                  ULONG_T connectionId,
                                                  UCHAR_T messagePriority,
                                                  UCHAR_T respondAddressLength,
                                                  UCHAR_T *respondAddress_p,
                                                  USHORT_T userDataLength,
                                                  UCHAR_T *userData_p);

typedef USHORT_T (*EINSS7_BSSAPLEDATA_IND_T) (UCHAR_T subSystemNumber,
                                              ULONG_T connectionId,
                                              UCHAR_T messagePriority,
                                              USHORT_T userDataLength,
                                              UCHAR_T *userData_p);

typedef USHORT_T (*EINSS7_BSSAPLEDISC_IND_T) (UCHAR_T subSystemNumber,
                                              ULONG_T connectionId,
                                              UCHAR_T reason,
                                              UCHAR_T originator,
                                              UCHAR_T respondAddressLength,
                                              UCHAR_T *respondAddress_p,
                                              USHORT_T userDataLength,
                                              UCHAR_T *userData_p);

typedef USHORT_T (*EINSS7_BSSAPLEUNITDATA_IND_T) (UCHAR_T subSystemNumber,
                                                  UCHAR_T sequenceControl,
                                                  UCHAR_T messagePriority,
                                                  UCHAR_T calledAddressLength,
                                                  UCHAR_T *calledAddress_p,
                                                  UCHAR_T callingAddressLength,
                                                  UCHAR_T *callingAddress_p,
                                                  USHORT_T userDataLength,
                                                  UCHAR_T *userData_p);

typedef USHORT_T (*EINSS7_BSSAPLEABORT_IND_T) (UCHAR_T sccpCarrier,
                                               EINSS7_BSSAPLE_TRANSACT_T 
                                               *transact_sp,
                                               EINSS7_BSSAPLE_ABORT_T
                                               *abortInd_sp);

typedef USHORT_T (*EINSS7_BSSAPLEMSPOSRESPONSE_IND_T) (UCHAR_T sccpCarrier,
                                                       EINSS7_BSSAPLE_TRANSACT_T
                                                       *transact_sp,
                                                       EINSS7_BSSAPLE_MSPOS_RESPONSE_IND_T
                                                       *msPosInd_sp);

typedef USHORT_T (*EINSS7_BSSAPLEREJECT_IND_T) (UCHAR_T sccpCarrier,
                                                EINSS7_BSSAPLE_TRANSACT_T
                                                *transact_sp,
                                                EINSS7_BSSAPLE_REJECT_IND_T
                                                *rejectInd_sp);

typedef USHORT_T (*EINSS7_BSSLAPRESET_IND_T) (UCHAR_T sccpCarrier,
                                              EINSS7_BSSAPLE_TRANSACT_T
                                              *transact_sp,
                                              EINSS7_BSSAPLE_RESET_IND_T
                                              *resetInd_sp);

typedef USHORT_T (*EINSS7_BSSAPLETARESPONSE_IND_T) (UCHAR_T sccpCarrier,
                                                    EINSS7_BSSAPLE_TRANSACT_T
                                                    *transact_sp,
                                                    EINSS7_BSSAPLE_TA_RESPONSE_IND_T
                                                    *taInd_sp);

typedef USHORT_T (*EINSS7_BSSAPLECLINFO_IND_T) (UCHAR_T sccpCarrier,
                                                EINSS7_BSSAPLE_TRANSACT_T
                                                *transact_sp,
                                                EINSS7_BSSAPLE_CLINFO_IND_T
                                                *clInfoInd_sp);

typedef USHORT_T (*EINSS7_BSSAPLECOINFO_IND_T) (UCHAR_T sccpCarrier,
                                                EINSS7_BSSAPLE_TRANSACT_T
                                                *transact_sp,
                                                EINSS7_BSSAPLE_COINFO_IND_T
                                                *coInfoInd_sp);

typedef USHORT_T (*EINSS7_BSSAPLEPERFLOCABORT_IND_T) (UCHAR_T sccpCarrier,
                                                      EINSS7_BSSAPLE_TRANSACT_T
                                                      *transact_sp,
                                                      EINSS7_BSSAPLE_PERFLOCABORT_IND_T
                                                      *perfLocAbortInd_sp);

typedef USHORT_T (*EINSS7_BSSAPLEPERFLOC_IND_T) (UCHAR_T sccpCarrier,
                                                 EINSS7_BSSAPLE_TRANSACT_T
                                                 *transact_sp,
                                                 EINSS7_BSSAPLE_PERFLOC_IND_T
                                                 *perfLocInd_sp);

typedef USHORT_T (*EINSS7_BSSAPLERESETACK_IND_T) (UCHAR_T sccpCarrier,
                                                  EINSS7_BSSAPLE_TRANSACT_T
                                                  *transact_sp,
                                                  EINSS7_BSSMAPLE_RESET_ACK_T
                                                  *resetAckInd_sp);

typedef USHORT_T (*EINSS7_BSSMAPLERESET_IND_T) (UCHAR_T sccpCarrier,
                                                EINSS7_BSSAPLE_TRANSACT_T
                                                *transact_sp,
                                                EINSS7_BSSMAPLE_RESET_T
                                                *resetInd_sp);

typedef USHORT_T (*EINSS7_BSSAPLESTATE_T) (UCHAR_T subSystemNumber, 
                                           UCHAR_T userStatus,
                                           UCHAR_T affectedSSN,
                                           ULONG_T affectedSPC,
                                           ULONG_T localSPC,
                                           UCHAR_T subSystemMultiplicityInd);

typedef USHORT_T (*EINSS7_BSSAPLEINDERROR_T) (UCHAR_T sccpCarrier,
                                              EINSS7_BSSAPLE_TRANSACT_T
                                              *transact_sp,
                                              USHORT_T error,
                                              USHORT_T userDataLength,
                                              UCHAR_T *userData_p); 

typedef struct EINSS7_BSSAPLE_INIT_TAG
{
    EINSS7_BSSAPLEBINDCONF_T          EINSS7_BssapLeBindConf;
    EINSS7_BSSAPLECONNECT_IND_T       EINSS7_BssapLeConnectInd;
    EINSS7_BSSAPLECONNECT_CONF_T      EINSS7_BssapLeConnectConf;
    EINSS7_BSSAPLEDATA_IND_T          EINSS7_BssapLeDataInd;
    EINSS7_BSSAPLEDISC_IND_T          EINSS7_BssapLeDiscInd;
    EINSS7_BSSAPLEUNITDATA_IND_T      EINSS7_BssapLeUnitdataInd;
    EINSS7_BSSAPLEABORT_IND_T         EINSS7_BssapLeAbortInd;
    EINSS7_BSSAPLEMSPOSRESPONSE_IND_T EINSS7_BssapLeMsPosResponseInd;
    EINSS7_BSSAPLEREJECT_IND_T        EINSS7_BssapLeRejectInd;
    EINSS7_BSSLAPRESET_IND_T          EINSS7_BsslapResetInd;
    EINSS7_BSSAPLETARESPONSE_IND_T    EINSS7_BssapLeTaResponseInd;
    EINSS7_BSSAPLECLINFO_IND_T        EINSS7_BssapLeCLInfoInd;
    EINSS7_BSSAPLECOINFO_IND_T        EINSS7_BssapLeCOInfoInd;
    EINSS7_BSSAPLEPERFLOCABORT_IND_T  EINSS7_BssapLePerfLocAbortInd;
    EINSS7_BSSAPLEPERFLOC_IND_T       EINSS7_BssapLePerfLocInd;
    EINSS7_BSSAPLERESETACK_IND_T      EINSS7_BssapLeResetAckInd;
    EINSS7_BSSMAPLERESET_IND_T        EINSS7_BssmapLeResetInd;
    EINSS7_BSSAPLESTATE_T             EINSS7_BssapLeStateInd;
    EINSS7_BSSAPLEINDERROR_T          EINSS7_BssapLeIndError;
} EINSS7_BSSAPLE_INIT_T;

#endif


/*********************************************************************/
/****         P  A  R  A  M  E  T  E  R     V  A  L  U  E         ****/
/****                  C  O  N  S  T  A  N  T  S                  ****/
/****                                                             ****/
/*********************************************************************/
/* APDU */

static const UCHAR_T EINSS7_BSSAPLE_PROT_ID_BSSLAP = 1;
static const UCHAR_T EINSS7_BSSAPLE_PROT_ID_LLP = 2;
static const UCHAR_T EINSS7_BSSAPLE_PROT_ID_SMLCPP = 3;

/* Bind Result */

static const UCHAR_T EINSS7_BSSAPLE_BINDRES_SCCP_NB_SUCCESS = 0;
static const UCHAR_T EINSS7_BSSAPLE_BINDRES_SCCP_SSN_ALREADY_IN_USE = 1;
static const UCHAR_T EINSS7_BSSAPLE_BINDRES_SCCP_PROTOCOL_ERROR = 2;
static const UCHAR_T EINSS7_BSSAPLE_BINDRES_SCCP_NB_RES_UNAVAIL = 3;
static const UCHAR_T EINSS7_BSSAPLE_BINDRES_SCCP_SSN_NOT_ALLOWED = 4;
static const UCHAR_T EINSS7_BSSAPLE_BINDRES_SCCP_SCCP_NOT_READY = 5;

/* Cause (BSSLAP) */

static const UCHAR_T EINSS7_BSSAPLE_CAUSE_CONGESTION = 0;
static const UCHAR_T EINSS7_BSSAPLE_CAUSE_CHANNEL_MODE_UNSUPPORTED = 1;
static const UCHAR_T EINSS7_BSSAPLE_CAUSE_PROCEDURE_UNSUPPORTED = 2;
static const UCHAR_T EINSS7_BSSAPLE_CAUSE_FAILURE_OTHER_RADIOREL_EVENTS = 3;
static const UCHAR_T EINSS7_BSSAPLE_CAUSE_INTRABSS_HANDOVER = 4;
static const UCHAR_T EINSS7_BSSAPLE_CAUSE_SUPERVISION_TIMER_EXPIRED = 5;
static const UCHAR_T EINSS7_BSSAPLE_CAUSE_INTERBSS_HANDOVER = 6;
static const UCHAR_T EINSS7_BSSAPLE_CAUSE_NO_SIGNALING_CONNECTION_TO_MS = 7;
static const UCHAR_T EINSS7_BSSAPLE_CAUSE_INCORRECT_SERVING_CELL_ID = 8;

/* Destination Identity */

static const UCHAR_T EINSS7_BSSAPLE_DEST_ID_MCC_MNC_LAC_CI = 0;
static const UCHAR_T EINSS7_BSSAPLE_DEST_ID_LAC_CI = 1;
static const UCHAR_T EINSS7_BSSAPLE_DEST_ID_MCC_MNC_LAC = 4;
static const UCHAR_T EINSS7_BSSAPLE_DEST_ID_LAC = 5;

/* LCS Cause */

static const UCHAR_T EINSS7_BSSAPLE_LCS_CAUSE_SYSTEM_FAILURE = 1;
static const UCHAR_T EINSS7_BSSAPLE_LCS_CAUSE_PROTOCOL_ERROR = 2;
static const UCHAR_T EINSS7_BSSAPLE_LCS_CAUSE_DATA_MISSING_IN_POS_REQ = 3;
static const UCHAR_T EINSS7_BSSAPLE_LCS_CAUSE_UNEXP_DATA_VALUE_IN_POS_REQ = 4;
static const UCHAR_T EINSS7_BSSAPLE_LCS_CAUSE_POS_METHOD_FAILURE = 5;
static const UCHAR_T EINSS7_BSSAPLE_LCS_CAUSE_TARGET_MS_UNREACHABLE = 6;
static const UCHAR_T EINSS7_BSSAPLE_LCS_CAUSE_LOCATION_REQ_ABORT = 7;
static const UCHAR_T EINSS7_BSSAPLE_LCS_CAUSE_FACILITY_NOT_SUPPORTED = 8;
static const UCHAR_T EINSS7_BSSAPLE_LCS_CAUSE_INTERBSC_HANDOVER_ONGOING = 9;
static const UCHAR_T EINSS7_BSSAPLE_LCS_CAUSE_INTRABSC_HANDOVER_COMPLETE = 10;
static const UCHAR_T EINSS7_BSSAPLE_LCS_CAUSE_CONGESTION = 11;

/* LCS QoS */

static const UCHAR_T EINSS7_BSSAPLE_LCS_QOS_LOW_DELAY = 1;
static const UCHAR_T EINSS7_BSSAPLE_LCS_QOS_DELAY_TOLERANT = 2;

/* Location Type */

static const UCHAR_T EINSS7_BSSAPLE_LOC_TYPE_CURRENT_GEOGRAPHIC_LOC = 0;
static const UCHAR_T EINSS7_BSSAPLE_LOC_TYPE_LOC_ASS_INFO_TARGET_MS = 1;
static const UCHAR_T EINSS7_BSSAPLE_LOC_TYPE_DECKEY_BRCAST_ASS_DATA_TARGET_MS = 2;
static const UCHAR_T EINSS7_BSSAPLE_LOC_TYPE_MOBILE_ASS = 1;
static const UCHAR_T EINSS7_BSSAPLE_LOC_TYPE_MOBILE_BASED_EOTD = 2;
static const UCHAR_T EINSS7_BSSAPLE_LOC_TYPE_ASSISTED_GPS = 3;

/* Message Priority / Importance Parameter */

static const UCHAR_T EINSS7_BSSAPLE_MESSPRIIMP_MESS_PRI_0 = 0;
static const UCHAR_T EINSS7_BSSAPLE_MESSPRIIMP_MESS_PRI_1 = 1;
static const UCHAR_T EINSS7_BSSAPLE_MESSPRIIMP_MESS_PRI_2 = 2;
static const UCHAR_T EINSS7_BSSAPLE_MESSPRIIMP_MESS_PRI_3 = 3;
static const UCHAR_T EINSS7_BSSAPLE_MESSPRIIMP_MESS_IMP_4 = 4;
static const UCHAR_T EINSS7_BSSAPLE_MESSPRIIMP_MESS_IMP_5 = 5;
static const UCHAR_T EINSS7_BSSAPLE_MESSPRIIMP_MESS_IMP_6 = 6;
static const UCHAR_T EINSS7_BSSAPLE_MESSPRIIMP_MESS_IMP_7 = 7;

/* Originator */

static const UCHAR_T EINSS7_BSSAPLE_ORGINATOR_SCCP_ORG_SCCP_USER = 1;
static const UCHAR_T EINSS7_BSSAPLE_ORGINATOR_SCCP_ORG_SCCP = 2;

/* Positioning Data */

static const UCHAR_T EINSS7_BSSAPLE_POS_DATA_TIMING_ADVANCE = 0;
static const UCHAR_T EINSS7_BSSAPLE_POS_DATA_TOA = 1;
static const UCHAR_T EINSS7_BSSAPLE_POS_DATA_AOA = 2;
static const UCHAR_T EINSS7_BSSAPLE_POS_DATA_MOBILE_ASSISTED_EOTD = 3;
static const UCHAR_T EINSS7_BSSAPLE_POS_DATA_MOBILE_BASED_EOTD = 4;
static const UCHAR_T EINSS7_BSSAPLE_POS_DATA_MOBILE_ASSISTED_GPS = 5;
static const UCHAR_T EINSS7_BSSAPLE_POS_DATA_MOBILE_BASED_GPS = 6;
static const UCHAR_T EINSS7_BSSAPLE_POS_DATA_CONVENTIONAL_GPS = 7;
static const UCHAR_T EINSS7_BSSAPLE_POS_DATA_UNSUCC_DUE_TO_FAIL_OR_INTERRUPT = 0;
static const UCHAR_T EINSS7_BSSAPLE_POS_DATA_SUCC_RES_NOT_USED_GEN_LOC = 1;
static const UCHAR_T EINSS7_BSSAPLE_POS_DATA_SUCC_RES_USED_VER_NOT_GEN_LOC = 2;
static const UCHAR_T EINSS7_BSSAPLE_POS_DATA_SUCC_MOB_POS_METH_NOT_DETERMINED = 3;

/* Reason */

static const UCHAR_T EINSS7_BSSAPLE_REASON_SCCP_END_USER_CONG = 0;
static const UCHAR_T EINSS7_BSSAPLE_REASON_SCCP_END_USER_FAILURE = 1;
static const UCHAR_T EINSS7_BSSAPLE_REASON_SCCP_SCCP_USER_ORIG = 2;
static const UCHAR_T EINSS7_BSSAPLE_REASON_SCCP_ACCESS_CONG = 3;
static const UCHAR_T EINSS7_BSSAPLE_REASON_SCCP_SS_CONG = 4;
static const UCHAR_T EINSS7_BSSAPLE_REASON_SCCP_ACCESS_FAILURE = 5;
static const UCHAR_T EINSS7_BSSAPLE_REASON_SCCP_ABN_COND = 6;
static const UCHAR_T EINSS7_BSSAPLE_REASON_SCCP_NORM_COND = 7;
static const UCHAR_T EINSS7_BSSAPLE_REASON_SCCP_NONTRANS_COND = 6;
static const UCHAR_T EINSS7_BSSAPLE_REASON_SCCP_TRANS_COND = 7;
static const UCHAR_T EINSS7_BSSAPLE_REASON_SCCP_INCOMP_INFO = 8;
static const UCHAR_T EINSS7_BSSAPLE_REASON_SCCP_END_USER_ORIG = 9;
static const UCHAR_T EINSS7_BSSAPLE_REASON_SCCP_ABN_COND_TRANS = 10;
static const UCHAR_T EINSS7_BSSAPLE_REASON_SCCP_ABN_COND_NONTRANS = 11;
static const UCHAR_T EINSS7_BSSAPLE_REASON_SCCP_SS_FAILURE = 12;
static const UCHAR_T EINSS7_BSSAPLE_REASON_SCCP_INVALID_STATE = 13;
static const UCHAR_T EINSS7_BSSAPLE_REASON_SCCP_DEST_ADDR_UNKNOWN = 14;
static const UCHAR_T EINSS7_BSSAPLE_REASON_SCCP_DEST_INACC_TRANS = 15;
static const UCHAR_T EINSS7_BSSAPLE_REASON_SCCP_DEST_INACC_NONTRANS = 16;
static const UCHAR_T EINSS7_BSSAPLE_REASON_SCCP_UNDEFINED = 17;
static const UCHAR_T EINSS7_BSSAPLE_REASON_SCCP_QOS_NOT_AVAIL_PERM = 18;
static const UCHAR_T EINSS7_BSSAPLE_REASON_SCCP_QOS_NOT_AVAIL_TRANS = 19;
static const UCHAR_T EINSS7_BSSAPLE_REASON_SCCP_REL_IN_PROG = 20;
static const UCHAR_T EINSS7_BSSAPLE_REASON_SCCP_UNSP_TRANS = 21;
static const UCHAR_T EINSS7_BSSAPLE_REASON_SCCP_UNSP_NON_TRANS = 22;
static const UCHAR_T EINSS7_BSSAPLE_REASON_SCCP_LOCAL_ERR = 23;
static const UCHAR_T EINSS7_BSSAPLE_REASON_SCCP_NO_TRANSLATION = 24;
static const UCHAR_T EINSS7_BSSAPLE_REASON_SCCP_UNKNOWN_LOCAL_SPC = 25;
static const UCHAR_T EINSS7_BSSAPLE_REASON_SCCP_SCCP_HOP_COUNT_ZERO = 26;

/* Return Error Cause */

static const UCHAR_T EINSS7_BSSAPLE_RETURN_ERR_CAUSE_SYSTEM_FAILURE = 1;
static const UCHAR_T EINSS7_BSSAPLE_RETURN_ERR_CAUSE_PROTOCOL_ERROR = 2;
static const UCHAR_T EINSS7_BSSAPLE_RETURN_ERR_CAUSE_DESTINATION_UNKNOWN = 3;
static const UCHAR_T EINSS7_BSSAPLE_RETURN_ERR_CAUSE_DESTINATION_UNREACHABLE = 4;
static const UCHAR_T EINSS7_BSSAPLE_RETURN_ERR_CAUSE_CONGESTION = 5;

/* Sequence Control */

static const UCHAR_T EINSS7_BSSAPLE_SEQ_CONTROL_SCCP_SEQ_CTRL_OFF = 0;
static const UCHAR_T EINSS7_BSSAPLE_SEQ_CONTROL_SCCP_SEQ_CTRL_ON = 1;

/* Source Identity */

static const UCHAR_T EINSS7_BSSAPLE_SOURCE_ID_MCC_MNC_LAC_CI = 0;
static const UCHAR_T EINSS7_BSSAPLE_SOURCE_ID_LAC_CI = 1;
static const UCHAR_T EINSS7_BSSAPLE_SOURCE_ID_MCC_MNC_LAC = 4;
static const UCHAR_T EINSS7_BSSAPLE_SOURCE_ID_LAC = 5;

/* User Status */

static const UCHAR_T EINSS7_BSSAPLE_USER_STAT_UIS = 0;
static const UCHAR_T EINSS7_BSSAPLE_USER_STAT_UOS = 1;
static const UCHAR_T EINSS7_BSSAPLE_USER_STAT_UIS_CONG_LEVEL_1 = 2;
static const UCHAR_T EINSS7_BSSAPLE_USER_STAT_UIS_CONG_LEVEL_2 = 3;
static const UCHAR_T EINSS7_BSSAPLE_USER_STAT_UIS_CONG_LEVEL_3 = 4;


#if defined (__cplusplus) || (c_plusplus)
}
#endif    /*        C + +  S E M A P H O R E                      ****/

#endif    /*        R E C U R S I O N  S E M A P H O R E          ****/
