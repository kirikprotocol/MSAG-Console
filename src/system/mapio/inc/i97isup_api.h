/*********************************************************************
Don't remove these two lines, contain depends on them!                    
Document Number: %Container% Revision: %Revision%                         
*/

/************************************************************************/
/*                                                                      */
/* i97isup_api.h,v */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/* COPYRIGHT Ericsson Infocom AB 1997                                   */
/*                                                                      */
/* All rights reserved. The copyright of the computer program herein    */
/* is the property of Ericsson Infocom AB, Sweden. The program may be   */
/* used and/or copied only with the permission of Ericsson Infocom AB   */
/* or in accordance with the terms and conditions stipulated in the     */
/* agreement/contract under which the program has been supplied.        */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/* Document Number:                                                     */
/* 22/190 55-CAA 201 80 Ux                                              */
/*                                                                      */
/* Programmer:                                                          */
/*      KS/EIN/N/P Per Wessmark                                         */
/*                                                                      */
/* Purpose:                                                             */
/*      This header file contains declarations and definitions          */
/*      used by the ISUP-API module,                                    */
/*      i97isup_api.c, 21/190 55-CAA 201 80 Ux.                         */
/*                                                                      */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/* Revision record:                                                     */
/*                                                                      */
/*	970319 Per Wessmark	First version				*/
/*                                                                   	*/
/* 	970922 EIN/N/B Mikael Blom Ericsson Infotech AB.               	*/
/*	Added compile switch EINSS7_FUNC_POINTER for use of local    	*/
/*	naming of the functions(primitives) in the application.      	*/
/*                                                                      */
/*      980902 EIN/N/P Anders Karlsson Ericsson Infotech AB.            */
/*      Added parameter(reserved) in  I97_FORWARD_T struct in order to  */
/*      relay reserved bits to the application (CR455).                 */
/*                                                                      */
/* 	980915 Kevin Finnegan				                */
/*      Prefixed EINSS7_ to all constants, global variables, function   */
/*	names and identifiers. Updated to support 24-bit point codes    */
/*                                                                      */
/*      990507 Seamus Burns (qinxsbu)                                   */
/*      CR0447 Added define for EINSS7_I97_MAX_SENDBUFSIZE, struct      */
/*             EINSS7_I97_SENDBUFFER_T and prototype for function       */
/*             *EINSS7_I97IsupGetSendBuffer                             */
/*                                                                      */
/*      000202 Martin Caldwell (qinxcam) ISUP LMF merge                 */
/*      Added EINSS7_I97IsupCircuitCancellationConf and                 */
/*      EINSS7_I97IsupReservationConf to EINSS7_I97ISUPINIT_T           */
/*      also adding types and prototypes                                */
/*                                                                      */
/*      010307 Martine O'Doherty (qinxmph)                              */
/*      SS7 SRS S2000  Mod Req-101                                      */
/*      Removed all instances of the compilation directive              */
/*      EINSS7_TWENTYFOURBITPC. This directive is not needed because the*/
/*      API will support both ITU Chinese 24 bit SPC & ITU 14 bit SPC.  */
/*      Removed EINSS7_I97_MAX_SIGN_POINT_CODE. This will not be used   */
/*      because max SPC can be for either ITU Chinese 24 bit SPC or     */
/*      ITU 14 bit SPC.                                                 */
/*                                                                      */
/*      010116 Anibal Caceres (eceacah) CS7_ONG2 Req 254 HSN/SPN        */  
/*      Updated value of constants EINSS7_I97_MAX_HSN, and              */  
/*      EINSS7_I97_MAX_SPAN.                                            */
/*                                                                      */
/*      010202 Luis Calvo Lopez (emelcl) ISUP ITU R8 (CS7-ONG2.0)       */
/*      Resource group parameter is added to primitives                 */
/*      ISUP_CONG_ind (Req-ISUP-7) and ISUP_SETUP_ind (Req-ISUP-9).     */
/*                                                                      */
/*      011130   Luis Calvo Lopez (ecelcl)	    	                */
/*      TR2599 Support for new ISUP_CIRCUIT_CANCELLATION_IND            */
/*      Added support for primitive ISUP_CIRCUIT_CANCELLATION_IND.      */
/*                                                                      */
/* Notes:                                                               */
/*                                                                      */
/************************************************************************/
#ifndef __ISUP_API_H__
#define __ISUP_API_H__
#if defined (__cplusplus) || (c_plusplus)
extern "C" {
#endif

/************************************************************************/
/*                                                                      */
/*                 C O N S T A N T S                                    */
/*                                                                      */
/************************************************************************/

#define EINSS7_I97_NO_IND		0x00
#define EINSS7_I97_NO_CHARGE		0x01
#define EINSS7_I97_CHARGE		0x02
#define EINSS7_I97_SUBSCRIBER_FREE 	0x01
#define EINSS7_I97_CONN_WHEN_FREE 	0x02
#define EINSS7_I97_ORD_B_SUBSCRIBER	0x01
#define EINSS7_I97_B_PAYPHONE		0x02
#define EINSS7_I97_NO_ETE_METH		0x00
#define EINSS7_I97_PASS_ALONG		0x01
#define EINSS7_I97_NO_INTERW		0x00
#define EINSS7_I97_INTERW   		0x01
#define EINSS7_I97_NO_END_END_INFO	0x00
#define EINSS7_I97_END_END_INFO	0x01
#define EINSS7_I97_NOT_ISUP_USED	0x00
#define EINSS7_I97_ISUP_USED		0x01
#define EINSS7_I97_NO_HOLD_REQ		0x00
#define EINSS7_I97_HOLD_REQ		0x01
#define EINSS7_I97_ACC_NON_ISDN	0x00
#define EINSS7_I97_ACC_ISDN	        0x01
#define EINSS7_I97_NO_ECHO_CONTR	0x00
#define EINSS7_I97_ECHO_CONTR		0x01
#define EINSS7_I97_NO_RESERVED_USED    	0x00 /* einkarl 980902 */

#define EINSS7_I97_SUBSCRIBER_NO	0x01
#define EINSS7_I97_NA_UNKNOWN	 	0x02
#define EINSS7_I97_NATIONAL_NO 	0x03
#define EINSS7_I97_INTERNATIONAL_NO	0x04
#define EINSS7_I97_ISDN_PLAN		0x01
#define EINSS7_I97_DATA_PLAN		0x03
#define EINSS7_I97_TELEX_PLAN		0x04
#define EINSS7_I97_INTERNAL_ROUT	0x00
#define EINSS7_I97_NO_INTERNAL_ROUT	0x01

#define EINSS7_I97_UNKNOWN_CAT		0x00
#define EINSS7_I97_OPERATOR_FRENCH	0x01
#define EINSS7_I97_OPERATOR_ENGLISH	0x02
#define EINSS7_I97_OPERATOR_GERMAN	0x03
#define EINSS7_I97_OPERATOR_RUSSIAN	0x04
#define EINSS7_I97_OPERATOR_SPANISH	0x05
#define EINSS7_I97_CPC_AVAIL_1		0x06
#define EINSS7_I97_CPC_AVAIL_2		0x07
#define EINSS7_I97_CPC_AVAIL_3		0x08
#define EINSS7_I97_ORD_A_SUBSCRIBER	0x0A
#define EINSS7_I97_PRIO_A_SUBSCRIBER	0x0B
#define EINSS7_I97_DATA_CALL		0x0C
#define EINSS7_I97_TEST_CALL		0x0D
#define EINSS7_I97_PAYPHONE 		0x0F

#define EINSS7_I97_USER_PROV		0x01
#define EINSS7_I97_NETW_PROV		0x03
#define EINSS7_I97_PRES_ALLOWED	0x00
#define EINSS7_I97_PRES_RESTR		0x01
#define EINSS7_I97_ADDR_NOT_AVAIL	0x02
#define EINSS7_I97_NUMB_COMPL		0x00
#define EINSS7_I97_NUMB_INCOMPL	0x01

#define EINSS7_I97_ISUP_USER		0x00
#define EINSS7_I97_PRIV_NETW_LOC	0x01
#define EINSS7_I97_PUBL_NETW_LOC	0x02
#define EINSS7_I97_TRANS_NETW		0x03
#define EINSS7_I97_PUBL_NETW_REM	0x04
#define EINSS7_I97_PRIV_NETW_REM	0x05
#define EINSS7_I97_INTERNAT_NETW	0x07
#define EINSS7_I97_BEYOND_INTERW	0x0A
#define EINSS7_I97_ITU_STAND		0x00
#define EINSS7_I97_REC_Q_763		0x00

#define EINSS7_I97_LC_HARD_BLOCK	0x01
#define EINSS7_I97_LC_DUAL_SEIZ	0x02
#define EINSS7_I97_LC_INV_RG		0x03
#define EINSS7_I97_LC_INV_HSN		0x04
#define EINSS7_I97_LC_INV_SPAN		0x05
#define EINSS7_I97_LC_INV_TS		0x06
#define EINSS7_I97_LC_USR_ERR		0x07
#define EINSS7_I97_LC_NTW_ERR		0x08
#define EINSS7_I97_LC_NO_MEM		0x09
#define EINSS7_I97_LC_NO_CIRC		0x0A
#define EINSS7_I97_LC_TS_UNAVAIL	0x0B
#define EINSS7_I97_LC_T6_REL		0x0C
#define EINSS7_I97_LC_T7_REL		0x0D
#define EINSS7_I97_LC_T9_REL		0x0E
#define EINSS7_I97_LC_RESET 		0x0F
#define EINSS7_I97_LC_UCIC_TWICE	0x10
#define EINSS7_I97_LC_CONG  		0x11
#define EINSS7_I97_LC_LOC_RESET 	0x12

#define EINSS7_I97_SENDING_TONE	0x00
#define EINSS7_I97_BACK_TONE_DETECTED	0x01
#define EINSS7_I97_NO_BACK_TONE_DETECTED 0x02

#define EINSS7_I97_TEST_COMPLETED	0x00
#define EINSS7_I97_LPA_REC		0x01
#define EINSS7_I97_CIRCUIT_SELECTED	0x02

#define EINSS7_I97_INSTALL_EQUIP   	0x01
#define EINSS7_I97_REMOVE_EQUIP	0x02
#define EINSS7_I97_IDLE		0x00
#define EINSS7_I97_BUSY		0x01

#define EINSS7_I97_NO_CONG  		0x00
#define EINSS7_I97_CONG_1     		0x01
#define EINSS7_I97_CONG_2     		0x02
#define EINSS7_I97_CONG_3     		0x03
#define EINSS7_I97_UNAVAIL_PAUSE  	0x04
#define EINSS7_I97_UNAVAIL_INACC  	0x05
#define EINSS7_I97_UNAVAIL_UNEQUIP	0x06

#define EINSS7_I97_ACC_CONG_1 		0x01
#define EINSS7_I97_ACC_CONG_2 		0x02

#define EINSS7_I97_ALERT		0x01
#define EINSS7_I97_PROGRESS		0x02
#define EINSS7_I97_INBAND_INFO		0x03
#define EINSS7_I97_FORWARD_BUSY	0x04
#define EINSS7_I97_FORWARD_NO_REPLY	0x05
#define EINSS7_I97_FORWARD_UNCON	0x06

#define EINSS7_I97_NAT_CALL		0x00
#define EINSS7_I97_INTERNAT_CALL	0x01
#define EINSS7_I97_NOT_ISUP_USED	0x00
#define EINSS7_I97_ISUP_PREF		0x00
#define EINSS7_I97_ISUP_NOT_REQ	0x01
#define EINSS7_I97_ISUP_REQ		0x02

#define EINSS7_I97_NO_SAT		0x00
#define EINSS7_I97_ONE_SAT		0x01
#define EINSS7_I97_TWO_SAT		0x02
#define EINSS7_I97_NO_CONT_CHECK	0x00
#define EINSS7_I97_CONT_CHECK_REQ	0x01
#define EINSS7_I97_CONT_CHECK_PRE	0x02

#define EINSS7_I97_IBI_AVAIL		0x01
#define EINSS7_I97_B_SEGMENTED		0x01
#define EINSS7_I97_MLPP_USER		0x01

#define EINSS7_I97_NOT_CUG_CALL	0x00
#define EINSS7_I97_CUG_ACC_ALL		0x02
#define EINSS7_I97_CUG_ACC_NO_ALL	0x03
#define EINSS7_I97_NO_SEG		0x00
#define EINSS7_I97_F_SEGMENTED		0x01
#define EINSS7_I97_CLI_NOT_REQ		0x00
#define EINSS7_I97_CLI_REQ		0x01

#define EINSS7_I97_NO_PAM		0x00
#define EINSS7_I97_PAM			0x01

#define EINSS7_I97_NO_RED		0x00
#define EINSS7_I97_CALL_RER		0x01
#define EINSS7_I97_CALL_RER_RESTR	0x02
#define EINSS7_I97_CALL_FORW		0x03
#define EINSS7_I97_CALL_FORW_RESTR	0x04
#define EINSS7_I97_CALL_RER_RED_RES	0x05
#define EINSS7_I97_CALL_FORW_RED_RES	0x06
#define EINSS7_I97_REASON_UNKNOWN	0x00
#define EINSS7_I97_USER_BUSY		0x01
#define EINSS7_I97_NO_REPLY		0x02
#define EINSS7_I97_UNCOND		0x03
#define EINSS7_I97_DEF_ALERT		0x04
#define EINSS7_I97_DEF_RESP		0x05
#define EINSS7_I97_MOB_NOT_REACHED	0x06

#define EINSS7_I97_REP_CONG		0x01
#define EINSS7_I97_UNKNOWN_MSG		0x02
#define EINSS7_I97_WRONG_STATE		0x03
#define EINSS7_I97_INV_CIRCUIT		0x04
#define EINSS7_I97_FORMAT_ERR		0x05
#define EINSS7_I97_MSG_TRUNCATED	0x06

#define EINSS7_I97_CON_TEST_SUCCESS	0x00
#define EINSS7_I97_CON_TEST_FAIL	0x01

#define EINSS7_I97_BIND_OK 		0x00
#define EINSS7_I97_ANOTHER_CC_BOUND	0x01
#define EINSS7_I97_SAME_CC_BOUND	0x02

#define EINSS7_I97_SOURCE_NET		0x00
#define EINSS7_I97_SOURCE_LOC		0x01

#define EINSS7_I97_RG_UNAVAIL		0x00
#define EINSS7_I97_RG_AVAIL		0x01

#define EINSS7_I97_ISDN_USER_INIT	0x00
#define EINSS7_I97_NETWORK_INIT	0x01

#define EINSS7_I97_SPEECH		0x00
#define EINSS7_I97_UNRESTR_64K		0x02
#define EINSS7_I97_AUDIO_3_1KHZ	0x03
#define EINSS7_I97_64K_PREFERRED	0x06
#define EINSS7_I97_2_64K_UNRESTR	0x07
#define EINSS7_I97_384K_UNRESTR	0x08
#define EINSS7_I97_1536K_UNRESTR	0x09
#define EINSS7_I97_1920K_UNRESTR	0x0A
	
/* Added for TR962 */
/* User service information */		/* See also 'Transmission medium req..' */
#define EINSS7_I97_UNRESTR_DIGITAL_INFO	0x08	/* and 'Cause indicators' */
#define EINSS7_I97_RESTR_DIGITAL_INFO	0x09
#define EINSS7_I97_AUDIO_3KHZ		0x10
#define EINSS7_I97_AUDIO_7KHZ		0x11
#define EINSS7_I97_VIDEO			0x18
#define EINSS7_I97_CCITT_STANDARD	0x00
#define EINSS7_I97_INT_STANDARDS	0x01
#define EINSS7_I97_NATIONAL_STANDARD	0x02
#define EINSS7_I97_NETWORK_STANDARD	0x03
#define EINSS7_I97_PACKET_MODE		0x00
#define EINSS7_I97_TRANS_RATE_64_KBITS	0x10
#define EINSS7_I97_TRANS_RATE_2_X_64_KBITS	0x11
#define EINSS7_I97_TRANS_RATE_384_KBITS		0x13
#define EINSS7_I97_TRANS_RATE_1536_KBITS	0x15
#define EINSS7_I97_TRANS_RATE_1920_KBITS	0x17
#define EINSS7_I97_TRANSFER_MODE_CIRCUIT	0x00
#define EINSS7_I97_TRANSFER_MODE_PACKET	0x01
#define EINSS7_I97_DEMAND		0x00
#define EINSS7_I97_POINT_TO_POINT	0x00
#define EINSS7_I97_STRUCTURE_DEFAULT	0x00
#define EINSS7_I97_STRUCTURE_8KHZ		0x01
#define EINSS7_I97_STRUCTURE_SERVICE_DATA	0x04
#define EINSS7_I97_UNSTRUCTURED		0x07
#define EINSS7_I97_BIDIRECT_SYMMETRIC	0x00
#define EINSS7_I97_PART_NOT_INCLUDED	0x00
#define EINSS7_I97_PART_INCLUDED	0x01
#define EINSS7_I97_CCITT_V110_X30	0x01
#define EINSS7_I97_G711_MICRO		0x02
#define EINSS7_I97_G711_A		0x03
#define EINSS7_I97_G721_I460		0x04
#define EINSS7_I97_G722_G724		0x05
#define EINSS7_I97_G735			0x06
#define EINSS7_I97_NON_CCITT		0x07
#define EINSS7_I97_CCITT_V120		0x08
#define EINSS7_I97_CCITT_X31		0x09
#define EINSS7_I97_INFO_LAYER_1		0x01
#define EINSS7_I97_INFO_LAYER_2		0x02
#define EINSS7_I97_INFO_LAYER_3		0x03
#define EINSS7_I97_E_BITS		0x00
#define EINSS7_I97_RATE_0_6		0x01
#define EINSS7_I97_RATE_1_2		0x02
#define EINSS7_I97_RATE_2_4		0x03
#define EINSS7_I97_RATE_3_6		0x04
#define EINSS7_I97_RATE_4_8		0x05
#define EINSS7_I97_RATE_7_2		0x06
#define EINSS7_I97_RATE_8_0		0x07
#define EINSS7_I97_RATE_9_6		0x08
#define EINSS7_I97_RATE_14_4		0x09
#define EINSS7_I97_RATE_16_0		0x0A
#define EINSS7_I97_RATE_19_2		0x0B
#define EINSS7_I97_RATE_32_0		0x0C
#define EINSS7_I97_RATE_48_0		0x0E
#define EINSS7_I97_RATE_56_0		0x0F
#define EINSS7_I97_RATE_0_1345		0x15
#define EINSS7_I97_RATE_0_1000		0x16
#define EINSS7_I97_RATE_0_075FORW_1_2BACKW	0x17
#define EINSS7_I97_RATE_1_2FORW_0_075BACKW	0x18
#define EINSS7_I97_RATE_0_050		0x19
#define EINSS7_I97_RATE_0_075		0x1A
#define EINSS7_I97_RATE_0_110		0x1B
#define EINSS7_I97_RATE_0_150		0x1C
#define EINSS7_I97_RATE_0_200		0x1D
#define EINSS7_I97_RATE_0_300		0x1E
#define EINSS7_I97_RATE_12 		0x1F
#define EINSS7_I97_NEGOTIATION_NOT_POSSIBLE	0x00
#define EINSS7_I97_NEGOTIATION_POSSIBLE	0x01
#define EINSS7_I97_SYNCHRONOUS		0x00
#define EINSS7_I97_ASYNCHRONOUS		0x01
#define EINSS7_I97_NO_FLOWCONTR_RECEPTION	0x00
#define EINSS7_I97_FLOWCONTR_RECEPTION	0x01
#define EINSS7_I97_NO_FLOWCONTR_TRANSM	0x00
#define EINSS7_I97_FLOWCONTR_TRANSM	0x01
#define EINSS7_I97_NO_NIC_RECEPTION	0x00
#define EINSS7_I97_NIC_RECEPTION	0x01
#define EINSS7_I97_NO_NIC_TRANSMISSION	0x00
#define EINSS7_I97_NIC_TRANSMISSION	0x01
#define EINSS7_I97_NOT_USED		0x00
#define EINSS7_I97_INTERM_RATE_8KBITS	0x01
#define EINSS7_I97_INTERM_RATE_16KBITS	0x02
#define EINSS7_I97_INTERM_RATE_32KBITS	0x03
#define EINSS7_I97_NOT_IN_BAND		0x00
#define EINSS7_I97_IN_BAND		0x01
#define EINSS7_I97_DEFAULT_ASSIGNEE	0x00
#define EINSS7_I97_ASSIGNOR_ONLY	0x01
#define EINSS7_I97_LLI_256 		0x00
#define EINSS7_I97_FULL_PROTOCOL	0x01
#define EINSS7_I97_BIT_TRANSPARENT 	0x00
#define EINSS7_I97_PROTOCOL_SENSITIVE	0x01
#define EINSS7_I97_NO_MULTIPLE_FRAME	0x00
#define EINSS7_I97_MULTIPLE_FRAME	0x01
#define EINSS7_I97_NO_RATE_ADAP_HEADER	0x00
#define EINSS7_I97_RATE_ADAP_HEADER	0x01
#define EINSS7_I97_PARITY_ODD		0x00
#define EINSS7_I97_PARITY_EVEN		0x02
#define EINSS7_I97_PARITY_NONE		0x03
#define EINSS7_I97_PARITY_FORCED_TO_0	0x04
#define EINSS7_I97_PARITY_FORCED_TO_1	0x05
#define EINSS7_I97_DATA_BITS_5		0x01
#define EINSS7_I97_DATA_BITS_7		0x02
#define EINSS7_I97_DATA_BITS_8		0x03
#define EINSS7_I97_STOP_BITS_1		0x01
#define EINSS7_I97_STOP_BITS_1_5	0x02
#define EINSS7_I97_STOP_BITS_2		0x03
#define EINSS7_I97_HALF_DUPLEX		0x00
#define EINSS7_I97_FULL_DUPLEX		0x01
#define EINSS7_I97_Q921_I441		0x02
#define EINSS7_I97_X25			0x06
#define EINSS7_I97_Q931_I451		0x02


/* Odds and ends */

#define EINSS7_I97_MIN_CALLED_DIGITS	0x01
#define EINSS7_I97_MAX_DIGITS		0x10

#define EINSS7_I97_SIZE_OF_MSG_HEAD	0x08

/* 010116 - eceacah - CS7_ONG2 Req 254 HSN/SPN. Updated constants. */
#define EINSS7_I97_MAX_HSN		0xFE
#define EINSS7_I97_MAX_SPAN 		0xFE
/* 010116 - eceacah - CS7_ONG2 Req 254 HSN/SPN. END */
#define EINSS7_I97_MAX_TIMESLOT	0x1F
#define EINSS7_I97_UNDEFINED		0xFF

#define EINSS7_I97_LAST_OCTET		0x80
#define EINSS7_I97_EVEN		0x00
#define EINSS7_I97_ODD			0x80

#define EINSS7_I97_MAX_PARAMS		0x20

#define EINSS7_I97_ZERO			0x00
#define EINSS7_I97_ONE			0x01
#define EINSS7_I97_TWO			0x02
#define EINSS7_I97_THREE		0x03
#define EINSS7_I97_FOUR			0x04
#define EINSS7_I97_FIVE			0x05
#define EINSS7_I97_SIX			0x06
#define EINSS7_I97_SEVEN		0x07
#define EINSS7_I97_EIGHT		0x08
#define EINSS7_I97_NINE			0x09

#define EINSS7_I97_1_BIT		0x01
#define EINSS7_I97_2_BITS		0x03
#define EINSS7_I97_3_BITS		0x07
#define EINSS7_I97_4_BITS  		0x0F
#define EINSS7_I97_5_BITS  		0x1F
#define EINSS7_I97_6_BITS  		0x3F
#define EINSS7_I97_7_BITS  		0x7F

#define EINSS7_I97_BIT_0		0x01
#define EINSS7_I97_BIT_01		0x03
#define EINSS7_I97_BIT_012		0x07
#define EINSS7_I97_BIT_0123		0x0F
#define EINSS7_I97_BIT_01234		0x1F
#define EINSS7_I97_BIT_012345		0x3F
#define EINSS7_I97_BIT_0123456		0x7F
#define EINSS7_I97_BIT_1		0x02
#define EINSS7_I97_BIT_12		0x06
#define EINSS7_I97_BIT_2		0x04
#define EINSS7_I97_BIT_23		0x0C
#define EINSS7_I97_BIT_3		0x08
#define EINSS7_I97_BIT_34		0x18
#define EINSS7_I97_BIT_34567		0xF8
#define EINSS7_I97_BIT_4		0x10
#define EINSS7_I97_BIT_45		0x30
#define EINSS7_I97_BIT_456 		0x70
#define EINSS7_I97_BIT_4567		0xF0
#define EINSS7_I97_BIT_5		0x20
#define EINSS7_I97_BIT_56		0x60
#define EINSS7_I97_BIT_6		0x40
#define EINSS7_I97_BIT_67		0xC0
#define EINSS7_I97_BIT_7		0x80

#define EINSS7_I97_MAX_USR_SIZE	0x81   /* 129 */


/* ISUP Parameter codes */

#define EINSS7_CALL_REFERENCE		0x01
#define EINSS7_TRANS_MEDIUM_REQ		0x02
#define EINSS7_ACCESS_TRANSPORT		0x03
#define EINSS7_NATURE_OF_CONN		0x06
#define EINSS7_FORWARD_IND		0x07
#define EINSS7_OPT_FORWARD_IND		0x08
#define EINSS7_CALLING_PARTY_CAT	0x09
#define EINSS7_CALLING_NUMB		0x0A
#define EINSS7_REDIRECTING_NUMB		0x0B
#define EINSS7_REDIRECTION_NUMB		0x0C
#define EINSS7_CONNECTION_REQUEST	0x0D
#define EINSS7_BACKWARD_IND		0x11
#define EINSS7_CAUSE_INDICATORS		0x12
#define EINSS7_REDIRECTION_INFO		0x13
#define EINSS7_CUG_INTERLOCK		0x1A
#define EINSS7_USER_SERVICE		0x1D
#define EINSS7_SIGN_POINT_CODE		0x1E
#define EINSS7_USER_INFORMATION		0x20
#define EINSS7_CONNECTED_NUMB		0x21
#define EINSS7_TRANSIT_NETWORK		0x23
#define EINSS7_EVENT_INFO		0x24
#define EINSS7_AUTO_CONGESTION		0x27
#define EINSS7_ORIGINAL_NUMB		0x28
#define EINSS7_OPT_BACKWARD_IND		0x29
#define EINSS7_USER_INDICATORS		0x2A


/***** Parameter defaults *****/

#define EINSS7_I97_DEF_NAT_OF_CONN      0x00
      /* No satellite           */
      /* Cont. check not req.   */
      /* No echo control device */

#define EINSS7_I97_DEF_FORW_IND_1       0x20
      /* National call          */
      /* No ETE method available */
      /* No interworking        */
      /* No end-to-end info     */
      /* ISUP used all the way  */
      /* ISUP preferred all the way   */

#define EINSS7_I97_DEF_FORW_IND_2       0x01
      /* Orig. access ISDN  */
      /* No SCCP method avail.  */

#define EINSS7_I97_DEF_CALLING_PARTY_CAT   0x0A
      /* Ordinary calling subs. */

#define EINSS7_I97_DEF_TRANSMISSION_REQ    0x00
      /* Speech          */

#define EINSS7_I97_DEF_BACKWARD_IND_1      0x10
      /* No ind on charge       */
      /* No ind on Called party Status */
      /* Ordinary subscriber */
      /* No end-to-end method   */

#define EINSS7_I97_DEF_BACKWARD_IND_2      0x04
      /* No interworking        */
      /* No end-to-end info     */
      /* ISUP used all the way  */
      /* Holding not required   */
      /* Term. access non-ISDN  */
      /* No echo control device */
      /* No SCCP method avail.  */

#define EINSS7_I97_DEF_SUS_RES		    0x00
      /* Subscriber initiated */

/*********************************************************************/
/*                                                                   */
/*                    O T H E R   C O N S T A N T S                  */
/*                                                                   */
/*********************************************************************/

/* 990507 - qinxsbu - CR0447 */
#define EINSS7_I97_MAX_SENDBUFSIZE       1000


/************************************************************************/
/*                                                                      */
/*			   S T R U C T U R E S				*/
/*                                                                      */
/************************************************************************/

/* 010307 - qinxmph - SS7-SRS S2000 Req-101 */
	typedef	ULONG_T		EINSS7_I97_SPC_T;

typedef struct EINSS7_I97_ACCESS_TAG
{
	UCHAR_T lengthOfInfoElements;
	UCHAR_T *infoElements_p;
} EINSS7_I97_ACCESS_T;

typedef struct EINSS7_I97_BACKWARD_TAG
{
	UCHAR_T charge;
	UCHAR_T calledPartyStatus;
	UCHAR_T calledPartyCategory;
	UCHAR_T endToEndMethod;
	BOOLEAN_T interworking;
	BOOLEAN_T endToEndInfo;
	BOOLEAN_T usedISUP;
	BOOLEAN_T holding;
	BOOLEAN_T accessISDN;
	BOOLEAN_T echoControlDevice;
	UCHAR_T methodSCCP;
} EINSS7_I97_BACKWARD_T;

typedef struct EINSS7_I97_CALLEDNUMB_TAG
{
	UCHAR_T natureOfAddr;
	UCHAR_T numberPlan;
	BOOLEAN_T internalNetwNumb;
	UCHAR_T noOfAddrSign;
	UCHAR_T *addrSign_p;
} EINSS7_I97_CALLEDNUMB_T;

typedef struct EINSS7_I97_CALLINGNUMB_TAG
{
	UCHAR_T natureOfAddr;
	UCHAR_T screening;
	UCHAR_T presentationRestr;
	UCHAR_T numberPlan;
	BOOLEAN_T numbIncompl;
	UCHAR_T noOfAddrSign;
	UCHAR_T *addrSign_p;
} EINSS7_I97_CALLINGNUMB_T;

typedef struct EINSS7_I97_CAUSE_TAG
{
	UCHAR_T location;
	UCHAR_T codingStd;
	UCHAR_T rec;
	UCHAR_T causeValue;
	UCHAR_T lengthOfDiagnostics;
	UCHAR_T *diagnostics_p;
} EINSS7_I97_CAUSE_T;

typedef struct EINSS7_I97_CONNECTEDNUMB_TAG
{
	UCHAR_T natureOfAddr;
	UCHAR_T screening;
	UCHAR_T presentationRestr;
	UCHAR_T numberPlan;
	UCHAR_T noOfAddrSign;
	UCHAR_T *addrSign_p;
} EINSS7_I97_CONNECTEDNUMB_T;

/* Added for TR995 */
typedef struct EINSS7_I97_CONNECTIONREQ_TAG
{
	ULONG_T localRef;
	EINSS7_I97_SPC_T signPointCode;
	UCHAR_T protocolClass;
	UCHAR_T credit;
} EINSS7_I97_CONNECTIONREQ_T;

typedef struct EINSS7_I97_CONREQSTATIND_TAG
{
	UCHAR_T requestInd;
	UCHAR_T circuitStatInd;
} EINSS7_I97_CONREQSTATIND_T;

typedef struct EINSS7_I97_EVENT_TAG
{
	UCHAR_T event;
	UCHAR_T presentation;
} EINSS7_I97_EVENT_T;

typedef struct EINSS7_I97_FORWARD_TAG
{
	BOOLEAN_T nationalInternational;
	UCHAR_T endToEndMethod;
	BOOLEAN_T interworking;
	BOOLEAN_T endToEndInfo;
	BOOLEAN_T usedISUP;
	UCHAR_T preferenceISUP;
	BOOLEAN_T accessISDN;
	UCHAR_T methodSCCP;
        UCHAR_T reserved; /* einkarl 980902 */
} EINSS7_I97_FORWARD_T;

typedef struct EINSS7_I97_OPTFORWARD_TAG
{
	UCHAR_T cugCallInd;
	BOOLEAN_T segmented;
	BOOLEAN_T cliReqInd;
} EINSS7_I97_OPTFORWARD_T;

typedef struct EINSS7_I97_ISUPHEAD_TAG
{
	ULONG_T connectionId;
	UCHAR_T hsn;
	UCHAR_T span;
	UCHAR_T timeslot;
} EINSS7_I97_ISUPHEAD_T;

typedef struct EINSS7_I97_NATUREOFCONN_TAG
{
	UCHAR_T satellite;
	UCHAR_T continuityCheck;
	BOOLEAN_T echoControlDevice;
} EINSS7_I97_NATUREOFCONN_T;

typedef struct EINSS7_I97_OPTBACKWARD_TAG
{
	BOOLEAN_T inBandInfo;
	BOOLEAN_T callForwarding;
	BOOLEAN_T segmented;
	BOOLEAN_T mlppUserInd;
} EINSS7_I97_OPTBACKWARD_T;

typedef struct EINSS7_I97_OPTPARAMS_TAG
{
	UCHAR_T  noOfParams;
	UCHAR_T	 *params[EINSS7_I97_MAX_PARAMS];
} EINSS7_I97_OPTPARAMS_T;

typedef struct EINSS7_I97_ORIGINALNUMB_TAG
{
	UCHAR_T natureOfAddr;
	UCHAR_T presentationRestr;
	UCHAR_T numberPlan;
	UCHAR_T noOfAddrSign;
	UCHAR_T *addrSign_p;
} EINSS7_I97_ORIGINALNUMB_T;

typedef struct EINSS7_I97_REDIRECTINGNUMB_TAG
{
	UCHAR_T natureOfAddr;
	UCHAR_T presentationRestr;
	UCHAR_T numberPlan;
	UCHAR_T noOfAddrSign;
	UCHAR_T *addrSign_p;
} EINSS7_I97_REDIRECTINGNUMB_T;

typedef struct EINSS7_I97_REDIRECTIONINFO_TAG
{
	UCHAR_T redirecting;
	UCHAR_T origReason;
	UCHAR_T counter;
	UCHAR_T lastReason;
} EINSS7_I97_REDIRECTIONINFO_T;

typedef struct EINSS7_I97_REDIRECTIONNUMB_TAG
{
	UCHAR_T natureOfAddr;
	UCHAR_T numberPlan;
	BOOLEAN_T internalNetwNumb;
	UCHAR_T noOfAddrSign;
	UCHAR_T *addrSign_p;
} EINSS7_I97_REDIRECTIONNUMB_T;

typedef struct EINSS7_I97_REPORTINFO_TAG
{
	UCHAR_T failedPrim;
	UCHAR_T failureCause;
	UCHAR_T lengthOfInfo;
	UCHAR_T *repInfo_p;
} EINSS7_I97_REPORTINFO_T;

typedef struct EINSS7_I97_USERINFORMATION_TAG
{
	UCHAR_T lengthOfUserInfo;
	UCHAR_T *userInfo_p;
} EINSS7_I97_USERINFORMATION_T;

/* Added for TR962 */
typedef struct EINSS7_I97_USERSERVICE_TAG
{
	UCHAR_T infoTransferCapability;
	UCHAR_T codingStd;
	UCHAR_T transRateOrigToDest;
	UCHAR_T transferMode;
	UCHAR_T establishment;
	UCHAR_T config;
	UCHAR_T structure;
	UCHAR_T transRateDestToOrig;
	UCHAR_T symmetry;
	BOOLEAN_T part2Included;
	UCHAR_T userInfoLayer1;
	UCHAR_T idLayer1;
	UCHAR_T userRate;
	BOOLEAN_T negotiation;
	BOOLEAN_T synchAsynch;
	BOOLEAN_T part2bIIncluded;
	BOOLEAN_T flowContrOnRx;
	BOOLEAN_T flowContrOnTx;
	BOOLEAN_T NICOnRx;
	BOOLEAN_T NICOnTx;
	UCHAR_T intermRate;
	BOOLEAN_T inBandOutBand;
	BOOLEAN_T assigner;
	BOOLEAN_T LLI;
	BOOLEAN_T modeOfOperation;
	BOOLEAN_T multipleFrame;
	BOOLEAN_T rateAdapHeader;
	BOOLEAN_T part2cIncluded;
	UCHAR_T parity;
	UCHAR_T noOfDataBits;
	UCHAR_T noOfStopBits;
	BOOLEAN_T part2dIncluded;
	UCHAR_T modemType;
	BOOLEAN_T duplex;
	BOOLEAN_T part3Included;
	UCHAR_T userInfoLayer2;
	UCHAR_T idLayer2;
	BOOLEAN_T part4Included;
	UCHAR_T userInfoLayer3;
	UCHAR_T idLayer3;
} EINSS7_I97_USERSERVICE_T;

/* 990507 - qinxsbu - CR0447 */
typedef struct EINSS7_I97_SENDBUFFER_TAG
{
       USHORT_T size;
       UCHAR_T *sendBuffer_p;
} EINSS7_I97_SENDBUFFER_T;

/************************************************************************/
/*                                                                      */
/*               R E Q U E S T   R E T U R N   C O D E S                */
/*                                                                      */
/************************************************************************/

#define EINSS7_I97_REQUEST_OK			0x00
#define EINSS7_I97_INV_CONN_ID			0x01
#define EINSS7_I97_INV_HSN		        0x02
#define EINSS7_I97_INV_SPAN		        0x03
#define EINSS7_I97_INV_TIMESLOT	        	0x04
#define EINSS7_I97_INV_BACKWARD_CALL_IND	0x05
#define EINSS7_I97_INV_CALLED_NUMBER		0x06
#define EINSS7_I97_INV_CALLING_NUMBER	    	0x07
#define EINSS7_I97_INV_CAUSE_IND    		0x08
#define EINSS7_I97_INV_END_INFO			0x09
#define EINSS7_I97_INV_EVENT_INFO		0x0A
#define EINSS7_I97_INV_EXTRA_OPTS_LEN		0x0B
#define EINSS7_I97_INV_FORWARD_CALL_IND		0x0C
#define EINSS7_I97_INV_NAT_OF_CONN_IND		0x0D
#define EINSS7_I97_INV_USER_TO_USER_INFO_LEN	0x0E
#define EINSS7_I97_INV_CONNECTED_NUMBER		0x0F
#define EINSS7_I97_INV_ORIGINAL_NUMBER		0x10
#define EINSS7_I97_INV_REDIRECTING_NUMBER	0x11
#define EINSS7_I97_INV_REDIRECTION_NUMBER	0x12
#define EINSS7_I97_INV_REDIRECTION_INFO		0x13
#define EINSS7_I97_INV_OPT_FORWARD_IND		0x14
#define EINSS7_I97_INV_CHECK_TONE_IND		0x15
#define EINSS7_I97_INV_CONT_REQ_STAT_IND	0x16
#define EINSS7_I97_INV_BIND_REQ			0x17

/* Added for TR962 */
#define EINSS7_I97_INV_TRANSFER_CAPABILITY 	0x18
#define EINSS7_I97_INV_TRANSFER_MODE		0x19
#define EINSS7_I97_INV_TRANSFER_RATE		0x1A
#define EINSS7_I97_INV_USER_INFO_LAYER1		0x1B
#define EINSS7_I97_INV_USER_INFO_LAYER2		0x1C
#define EINSS7_I97_INV_USER_INFO_LAYER3		0x1D
#define EINSS7_I97_INV_USER_RATE		0x1E
#define EINSS7_I97_INV_INTERMEDIATE_RATE	0x1F
#define EINSS7_I97_INV_CODING_STANDARD		0x20
#define EINSS7_I97_INV_ESTABLISHMENT		0x21
#define EINSS7_I97_INV_CONFIGURATION		0x22
#define EINSS7_I97_INV_STRUCTURE		0x23
#define EINSS7_I97_INV_SYMMETRY			0x24
#define EINSS7_I97_INV_PARITY			0x25
#define EINSS7_I97_INV_DATA_BITS		0x26
#define EINSS7_I97_INV_STOP_BITS		0x27
/* End of TR962 */

#define	EINSS7_I97_INV_CONGESTION_LEVEL		0x28	/* Added for TR993 */
#define EINSS7_I97_INV_SIGN_POINT_CODE		0x29	/* Added for TR994/TR995 */
#define EINSS7_I97_ISUP_CALLBACK_FUNC_ALREADY_SET		0x2A
#define EINSS7_I97_ISUP_CALLBACK_FUNC_NOT_SET		0x2B
/* 990507 - qinxsbu - CR0447 */ 
#define EINSS7_I97_INV_MESSAGE_SIZE             0x2C
#define EINSS7_I97_INV_RET_CONFIRMATION		0x2D /* qinxcam */

/************************************************************************/
/*                                                                      */
/*           I N D I C A T I O N   R E T U R N   C O D E S              */
/*                                                                      */
/************************************************************************/

#define EINSS7_I97_IND_UNKNOWN_CODE         0xFE
#define EINSS7_I97_IND_LENGTH_ERROR         0xFF

/************************************************************************/
/*                                                                      */
/*                  R E Q U E S T   P R O T O T Y P E S                 */
/*                                                                      */
/************************************************************************/
USHORT_T EINSS7_I97IsupBindReq(USHORT_T);

USHORT_T EINSS7_I97IsupUnBindReq(void);

USHORT_T EINSS7_I97IsupSetupReq(EINSS7_I97_ISUPHEAD_T *, 
				USHORT_T, 
				EINSS7_I97_CALLEDNUMB_T *,
                         	EINSS7_I97_NATUREOFCONN_T *, 
				EINSS7_I97_FORWARD_T *, 
				UCHAR_T *, 
				UCHAR_T *,
		         	EINSS7_I97_CALLINGNUMB_T *, 
				EINSS7_I97_OPTFORWARD_T *, 
				EINSS7_I97_ACCESS_T *, 
		         	EINSS7_I97_REDIRECTINGNUMB_T *, 
				EINSS7_I97_REDIRECTIONINFO_T *,  
		         	EINSS7_I97_ORIGINALNUMB_T *, 
				EINSS7_I97_USERINFORMATION_T *, 
		         	EINSS7_I97_USERSERVICE_T *, 		/* TR962 */
				/*	EINSS7_I97_CONNECTIONREQ_T *, */
				EINSS7_I97_OPTPARAMS_T *);

USHORT_T EINSS7_I97IsupSetupResp(EINSS7_I97_ISUPHEAD_T *, 
				EINSS7_I97_BACKWARD_T *, 
				EINSS7_I97_OPTBACKWARD_T *, 
			  	EINSS7_I97_USERINFORMATION_T *, 
				EINSS7_I97_CONNECTEDNUMB_T *, 
			  	EINSS7_I97_ACCESS_T *, 
				EINSS7_I97_OPTPARAMS_T *);

USHORT_T EINSS7_I97IsupReleaseReq(EINSS7_I97_ISUPHEAD_T *, 
				EINSS7_I97_CAUSE_T *, 
				UCHAR_T *, 
			   	EINSS7_I97_REDIRECTIONNUMB_T *, 
				EINSS7_I97_REDIRECTIONINFO_T *,
				EINSS7_I97_OPTPARAMS_T *);

USHORT_T EINSS7_I97IsupReleaseResp(EINSS7_I97_ISUPHEAD_T *, 
				EINSS7_I97_OPTPARAMS_T *);

USHORT_T EINSS7_I97IsupProceedReq(EINSS7_I97_ISUPHEAD_T *, 
				EINSS7_I97_BACKWARD_T *, 
				EINSS7_I97_OPTBACKWARD_T *,
                              	EINSS7_I97_OPTPARAMS_T *);

USHORT_T EINSS7_I97IsupProgressReq(EINSS7_I97_ISUPHEAD_T *, 
				EINSS7_I97_EVENT_T *, 
				EINSS7_I97_BACKWARD_T *, 
			    	EINSS7_I97_OPTBACKWARD_T *, 
				EINSS7_I97_OPTPARAMS_T *);

USHORT_T EINSS7_I97IsupContinuityReq(EINSS7_I97_ISUPHEAD_T *, 
				UCHAR_T);

USHORT_T EINSS7_I97IsupContinuityResp(EINSS7_I97_ISUPHEAD_T *, 
				EINSS7_I97_CONREQSTATIND_T *, 
				UCHAR_T);

USHORT_T EINSS7_I97IsupSuspendReq(EINSS7_I97_ISUPHEAD_T *, 
				UCHAR_T *, 
				EINSS7_I97_OPTPARAMS_T *);

USHORT_T EINSS7_I97IsupResumeReq(EINSS7_I97_ISUPHEAD_T *, 
				UCHAR_T *, 
				EINSS7_I97_OPTPARAMS_T *);

USHORT_T EINSS7_I97IsupMiscReq(EINSS7_I97_ISUPHEAD_T *, 
				UCHAR_T, 
				UCHAR_T, 
				EINSS7_I97_OPTPARAMS_T *);
/* qinxcam */
USHORT_T EINSS7_I97IsupCircuitReservationReq(EINSS7_I97_ISUPHEAD_T *, 
				USHORT_T); 

USHORT_T EINSS7_I97IsupCircuitCancellationReq(EINSS7_I97_ISUPHEAD_T *, 
				UCHAR_T); 

/************************************************************************/
/*                                                                      */
/*               I N D I C A T I O N   P R O T O T Y P E S              */
/*                                                                      */
/************************************************************************/

USHORT_T EINSS7_I97IsupHandleInd(MSG_T *mqp_sp);

#ifdef EINSS7_FUNC_POINTER

/* creates the datatypes with "pointer to function" */

typedef USHORT_T (*EINSS7_I97ISUPINDERROR_T)(USHORT_T, MSG_T *);

typedef USHORT_T (*EINSS7_I97ISUPBINDCONF_T)(USHORT_T, UCHAR_T);

typedef USHORT_T (*EINSS7_I97ISUPSETUPIND_T)
			(EINSS7_I97_ISUPHEAD_T *, 
/* 010312 - emelcl CS7-ONG2.0 Req-ISUP-9 */
                        USHORT_T,
/* END 010312 - emelcl CS7-ONG2.0 Req-ISUP-9 */
			EINSS7_I97_NATUREOFCONN_T *,
		   	EINSS7_I97_FORWARD_T *, 
			UCHAR_T *, UCHAR_T *, 
			EINSS7_I97_CALLEDNUMB_T *, 
		   	EINSS7_I97_CALLINGNUMB_T *, 
			EINSS7_I97_OPTFORWARD_T *, 
			EINSS7_I97_ACCESS_T *, 
		   	EINSS7_I97_REDIRECTINGNUMB_T *, 
			EINSS7_I97_REDIRECTIONINFO_T *,  
		   	EINSS7_I97_ORIGINALNUMB_T *, 
			EINSS7_I97_USERINFORMATION_T *, 
		   	EINSS7_I97_USERSERVICE_T *, 		/* TR962 */
		      /*EINSS7_I97_CONNECTIONREQ_T *, */
			EINSS7_I97_OPTPARAMS_T *);

typedef USHORT_T (*EINSS7_I97ISUPSETUPCONF_T)
			(EINSS7_I97_ISUPHEAD_T *, 
			EINSS7_I97_BACKWARD_T *, 
		   	EINSS7_I97_OPTBACKWARD_T *, 
			EINSS7_I97_USERINFORMATION_T *,
		   	EINSS7_I97_CONNECTEDNUMB_T *, 
			EINSS7_I97_ACCESS_T *, 
			EINSS7_I97_OPTPARAMS_T *);

typedef USHORT_T (*EINSS7_I97ISUPRELEASEIND_T)
			(EINSS7_I97_ISUPHEAD_T *, 
			UCHAR_T, 
			UCHAR_T , 
		   	EINSS7_I97_CAUSE_T *, 
			UCHAR_T *, 
			EINSS7_I97_REDIRECTIONNUMB_T *, 
		   	EINSS7_I97_REDIRECTIONINFO_T *, 
			/*EINSS7_I97_SPC_T *,*/
			EINSS7_I97_OPTPARAMS_T *);

typedef USHORT_T (*EINSS7_I97ISUPRELEASECONF_T)
			(EINSS7_I97_ISUPHEAD_T *, 
			EINSS7_I97_OPTPARAMS_T *);

typedef USHORT_T (*EINSS7_I97ISUPPROCEEDIND_T)
			(EINSS7_I97_ISUPHEAD_T *, 
			EINSS7_I97_BACKWARD_T *, 
		   	EINSS7_I97_OPTBACKWARD_T *, 
			EINSS7_I97_OPTPARAMS_T *);

typedef USHORT_T (*EINSS7_I97ISUPPROGRESSIND_T)
			(EINSS7_I97_ISUPHEAD_T *, 
			EINSS7_I97_EVENT_T *, 
		   	EINSS7_I97_BACKWARD_T *, 
			EINSS7_I97_OPTBACKWARD_T *, 
			EINSS7_I97_OPTPARAMS_T *);

typedef USHORT_T (*EINSS7_I97ISUPCONTINUITYCONF_T)
			(EINSS7_I97_ISUPHEAD_T *, 
			UCHAR_T);

typedef USHORT_T (*EINSS7_I97ISUPCONTINUITYIND_T)
			(EINSS7_I97_ISUPHEAD_T *, 
			UCHAR_T, 
		   	EINSS7_I97_CONREQSTATIND_T *);

typedef USHORT_T (*EINSS7_I97ISUPSUSPENDIND_T)
			(EINSS7_I97_ISUPHEAD_T *, 
			UCHAR_T *, 
		   	EINSS7_I97_OPTPARAMS_T *);

typedef USHORT_T (*EINSS7_I97ISUPRESUMEIND_T)
			(EINSS7_I97_ISUPHEAD_T *, 
			UCHAR_T *, 
		   	EINSS7_I97_OPTPARAMS_T *);

typedef USHORT_T (*EINSS7_I97ISUPMISCIND_T)
			(EINSS7_I97_ISUPHEAD_T *, 
			UCHAR_T, 
			UCHAR_T, 
		   	EINSS7_I97_OPTPARAMS_T *);

typedef USHORT_T (*EINSS7_I97ISUPRESOURCEIND_T)
			(USHORT_T, 
			UCHAR_T);

typedef USHORT_T (*EINSS7_I97ISUPREPORTIND_T)
			(EINSS7_I97_ISUPHEAD_T *, 
			EINSS7_I97_REPORTINFO_T *);

typedef USHORT_T (*EINSS7_I97ISUPCONGIND_T)
/* 010312 - emelcl CS7-ONG2.0 Req-ISUP-7 */
			(USHORT_T ,
/* END 010312 - emelcl CS7-ONG2.0 Req-ISUP-7 */
			EINSS7_I97_SPC_T, 
			UCHAR_T);

typedef USHORT_T (*EINSS7_I97ISUPCANCELLATIONCONF_T)
			(EINSS7_I97_ISUPHEAD_T *, 
			UCHAR_T);

typedef USHORT_T (*EINSS7_I97ISUPRESERVATIONCONF_T)
			(EINSS7_I97_ISUPHEAD_T *, 
			UCHAR_T);

/* 011130 - ecelcl - TR2599 */
typedef USHORT_T (*EINSS7_I97ISUPCANCELLATIONIND_T)
			(EINSS7_I97_ISUPHEAD_T *, 
			UCHAR_T);
#else

USHORT_T EINSS7_I97IsupIndError(USHORT_T, 
				MSG_T *);

USHORT_T EINSS7_I97IsupBindConf(USHORT_T, 
				UCHAR_T);

USHORT_T EINSS7_I97IsupSetupInd(EINSS7_I97_ISUPHEAD_T *, 
/* 010312 - emelcl CS7-ONG2.0 Req-ISUP-9 */
				USHORT_T ,
/* END 010312 - emelcl CS7-ONG2.0 Req-ISUP-9 */
				EINSS7_I97_NATUREOFCONN_T *, 
				EINSS7_I97_FORWARD_T *,
                          	UCHAR_T *, 
				UCHAR_T *, 
				EINSS7_I97_CALLEDNUMB_T *, 
			  	EINSS7_I97_CALLINGNUMB_T *, 
				EINSS7_I97_OPTFORWARD_T *, 
				EINSS7_I97_ACCESS_T *, 
			  	EINSS7_I97_REDIRECTINGNUMB_T *, 
				EINSS7_I97_REDIRECTIONINFO_T *,  
			  	EINSS7_I97_ORIGINALNUMB_T *, 
				EINSS7_I97_USERINFORMATION_T *, 
			  	EINSS7_I97_USERSERVICE_T *, 		/* TR962 */
				/*EINSS7_I97_CONNECTIONREQ_T *, */
				EINSS7_I97_OPTPARAMS_T *);

USHORT_T EINSS7_I97IsupSetupConf(EINSS7_I97_ISUPHEAD_T *, 
				EINSS7_I97_BACKWARD_T *, 
				EINSS7_I97_OPTBACKWARD_T *, 
			  	EINSS7_I97_USERINFORMATION_T *, 
				EINSS7_I97_CONNECTEDNUMB_T *, 
			  	EINSS7_I97_ACCESS_T *, 
				EINSS7_I97_OPTPARAMS_T *);

USHORT_T EINSS7_I97IsupReleaseInd(EINSS7_I97_ISUPHEAD_T *, 
				UCHAR_T, 
				UCHAR_T , 
				EINSS7_I97_CAUSE_T *,
                           	UCHAR_T *, 
				EINSS7_I97_REDIRECTIONNUMB_T *, 
			   	EINSS7_I97_REDIRECTIONINFO_T *,
				EINSS7_I97_OPTPARAMS_T *);

USHORT_T EINSS7_I97IsupReleaseConf(EINSS7_I97_ISUPHEAD_T *, 
				EINSS7_I97_OPTPARAMS_T *);

USHORT_T EINSS7_I97IsupProceedInd(EINSS7_I97_ISUPHEAD_T *, 
				EINSS7_I97_BACKWARD_T *, 
				EINSS7_I97_OPTBACKWARD_T *, 
			   	EINSS7_I97_OPTPARAMS_T *);

USHORT_T EINSS7_I97IsupProgressInd(EINSS7_I97_ISUPHEAD_T *, 
				EINSS7_I97_EVENT_T *, 
				EINSS7_I97_BACKWARD_T *, 
			    	EINSS7_I97_OPTBACKWARD_T *, 
				EINSS7_I97_OPTPARAMS_T *);

USHORT_T EINSS7_I97IsupContinuityConf(EINSS7_I97_ISUPHEAD_T *, 
				UCHAR_T);

USHORT_T EINSS7_I97IsupContinuityInd(EINSS7_I97_ISUPHEAD_T *, 
				UCHAR_T, 
				EINSS7_I97_CONREQSTATIND_T *);

USHORT_T EINSS7_I97IsupSuspendInd(EINSS7_I97_ISUPHEAD_T *, 
				UCHAR_T *, 
				EINSS7_I97_OPTPARAMS_T *);

USHORT_T EINSS7_I97IsupResumeInd(EINSS7_I97_ISUPHEAD_T *, 
				UCHAR_T *, 
				EINSS7_I97_OPTPARAMS_T *);

USHORT_T EINSS7_I97IsupMiscInd(EINSS7_I97_ISUPHEAD_T *, 
				UCHAR_T, 
				UCHAR_T, 
				EINSS7_I97_OPTPARAMS_T *);

USHORT_T EINSS7_I97IsupResourceInd(USHORT_T, 
				UCHAR_T);

USHORT_T EINSS7_I97IsupReportInd(EINSS7_I97_ISUPHEAD_T *, 
				EINSS7_I97_REPORTINFO_T *);

/* 010312 - emelcl CS7-ONG2.0 Req-ISUP-7 */
USHORT_T EINSS7_I97IsupCongInd(USHORT_T ,
/* END 010312 - emelcl CS7-ONG2.0 Req-ISUP-7 */
                                EINSS7_I97_SPC_T, 
				UCHAR_T);

/* qinxcam */
USHORT_T EINSS7_I97IsupCircuitCancellationConf(EINSS7_I97_ISUPHEAD_T *, 
				UCHAR_T);

USHORT_T EINSS7_I97IsupCircuitReservationConf(EINSS7_I97_ISUPHEAD_T *, 
				UCHAR_T);

/* 011130 - ecelcl - TR2599 */
USHORT_T EINSS7_I97IsupCircuitCancellationInd(EINSS7_I97_ISUPHEAD_T *, 
				UCHAR_T);

#endif /*EINSS7_FUNC_POINTER */

/************************************************************************/
/*                                                                      */
/*                    O T H E R    P R O T O T Y P E S                  */
/*                                                                      */
/************************************************************************/
#ifdef EINSS7_NO_APISEND
/* 990507 - qinxsbu - CR0447 */
EINSS7_I97_SENDBUFFER_T *EINSS7_I97IsupGetSendBuffer(void);
#endif /* EINSS7_NO_APISEND */

#ifdef EINSS7_FUNC_POINTER
  typedef struct EINSS7_I97ISUPINIT_TAG
  {
	EINSS7_I97ISUPINDERROR_T	EINSS7_I97IsupIndError;	/* Pointer to a function*/
	EINSS7_I97ISUPBINDCONF_T	EINSS7_I97IsupBindConf;
	EINSS7_I97ISUPSETUPIND_T	EINSS7_I97IsupSetupInd;
	EINSS7_I97ISUPSETUPCONF_T	EINSS7_I97IsupSetupConf;
	EINSS7_I97ISUPRELEASEIND_T	EINSS7_I97IsupReleaseInd;
	EINSS7_I97ISUPRELEASECONF_T	EINSS7_I97IsupReleaseConf;
	EINSS7_I97ISUPPROCEEDIND_T	EINSS7_I97IsupProceedInd;
	EINSS7_I97ISUPPROGRESSIND_T	EINSS7_I97IsupProgressInd;
	EINSS7_I97ISUPCONTINUITYCONF_T	EINSS7_I97IsupContinuityConf;
	EINSS7_I97ISUPCONTINUITYIND_T	EINSS7_I97IsupContinuityInd;
	EINSS7_I97ISUPSUSPENDIND_T	EINSS7_I97IsupSuspendInd;
	EINSS7_I97ISUPRESUMEIND_T	EINSS7_I97IsupResumeInd;
	EINSS7_I97ISUPMISCIND_T	EINSS7_I97IsupMiscInd;
	EINSS7_I97ISUPRESOURCEIND_T	EINSS7_I97IsupResourceInd;
	EINSS7_I97ISUPREPORTIND_T	EINSS7_I97IsupReportInd;
	EINSS7_I97ISUPCONGIND_T	EINSS7_I97IsupCongInd;
	EINSS7_I97ISUPCANCELLATIONCONF_T EINSS7_I97IsupCircuitCancellationConf;/* qinxcam */
	EINSS7_I97ISUPRESERVATIONCONF_T	EINSS7_I97IsupCircuitReservationConf;
        /* 011130 - ecelcl - TR2599 */
	EINSS7_I97ISUPCANCELLATIONIND_T EINSS7_I97IsupCircuitCancellationInd;
  }EINSS7_I97ISUPINIT_T;

/* The application must call this function to register the call back funtions. */ 
	USHORT_T EINSS7_I97IsupInit(EINSS7_I97ISUPINIT_T *);
#endif /* EINSS7_FUNC_POINTER */

#if defined (__cplusplus) || (c_plusplus)
}
#endif
#endif /* __ISUP_API_H__ */
