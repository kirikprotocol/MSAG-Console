#ifndef __SS7MSG_H__
#define __SS7MSG_H__
/*********************************************************************/
/*                                                                   */
/* ss7msg.h,v                                            */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* COPYRIGHT Ericsson Infotech 1998                                  */
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
/* 10/190 55-CAA 201 29 Ux                                           */
/*                                                                   */
/* Revision:                                                         */
/* @EINVER: ss7msg.h,v 1.65 2000/06/26 15:27:26 Exp
 */
/*                                                                   */
/* Purpose: Primitive codes for Portss7                              */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Revision record:                                                  */
/* 931220 M.Hammarlund  XM messages added. Test if UNKNOWN is        */
/*        defined before defining it.                                */
/*                                                                   */
/*                                                                   */
/* 940802 D.Eriksson New primitives for commonpart                   */
/*        RESERVED PRIMITIVES 50-55!!                                */
/*                                                                   */
/* 940923 CoLi Added MM_RESOURCES_UNAVAILABLE                        */
/*        in the errorcodes for Mgmt interface                       */
/*                                                                   */
/* 940928 DeEr New primitives for congestion handling                */
/*        from MTP-L2. DL_CONGESTED_IND and                          */
/*        DL_CONGCEASE_IND.                                          */
/*                                                                   */
/* 941014 JoLi New primitives for ANSI TCAP added.                   */
/*                                                                   */
/* 941116 XPWE New primitives for VAP added.                         */
/*                                                                   */
/* 950126 JoLi New primitives for IS41 added.                        */
/*                                                                   */
/* 950203 ToSt New primitive for CCITT TCAP, T_STATE_IND             */
/*                                                                   */
/* 950307 Dennis Eriksson                                            */
/*        Updated this header for CVS.                               */
/*                                                                   */
/* 950320 Bjerne Clavensjo                                           */
/*        New primitives for VAP/TMC added.                          */
/*                                                                   */
/* 951102 Martin Wessmark                                            */
/*        Updated primitives for ISUP.                               */
/*                                                                   */
/* 951117 Henrik Berglund                                            */
/*        New primitives for MTP-L2: DL_LPO_REQ, DL_LPOR_REQ,        */
/*        DL_RETRIEVEBSNT_REQ, DL_RETRIEVEFSNC_REQ, DL_BSNT_CONF     */
/*        and DL_FSNC_CONF.                                          */
/*                                                                   */
/* 951120 Mikael Lindh                                               */
/*        Updated primitives for TUP.                                */
/*                                                                   */
/* 951214 Dan Liljemark                                              */
/*        Added MM_XSTAT_REQ and MM_XSTAT_CONF.                      */
/*                                                                   */
/* 960213 Bjerne Clavensjo                                           */
/*        New primitives for ANSI ISUP added.                        */
/*                                                                   */
/* 960920 Ahmad Mahmoudi                                             */
/*        New primitives for BE-if and FE-if were added.             */
/*        Some MTPL2 primitive numbers were changed due to           */
/*        collision.                                                 */
/*                                                                   */
/* 961004 Ahmad Mahmoudi                                             */
/*        The MTPL2 primitive numbers which had been changed on      */
/*        960920 were changed back to their original values, for     */
/*        backward compatibility reasons.                            */
/*                                                                   */
/* 961120 Henrik Berglund                                            */
/*        Clean up for i93 integration                               */
/*        Changed names (&numbers)                                   */
/*           DL_BSNT_CONF          -> DL_RETRIEVEBSNT_CONF           */
/*           DL_FSNC_CONF          -> DL_RETRIEVEVALMSG_IND          */
/*           DL_RETRIEVEFSNC_REQ   -> DL_RETRIEVEVALSTART_REQ        */
/*        Changed primitive numbers                                  */
/*           DL_STATUS_REQ     46 -> 45                              */
/*           DL_STATUS_CONF    45 -> 43                              */
/*        New primitives                                             */
/*           DL_RETRIEVEVALEND_CONF                                  */
/*                                                                   */
/* 961126 Hans Andersson                                             */
/*        New primitives for IS-41 added.                            */
/*                                                                   */
/* 961126 Dan Liljemark                                              */
/*        New primitives for ETSI INAP added.                        */
/*                                                                   */
/* 961204 Dennis Eriksson                                            */
/*        Added XM_XSTAT_REQ and XM_XSTAT_CONF                       */
/*                                                                   */
/* 970127 Pär Larsson                                                */
/*        Added HB_PRIMITIVE                                         */
/*                                                                   */
/* 980420 Anders Peltomäki                                           */
/*        Added TUP_ACI_CONF for BTNR TUP. CR381                     */
/*                                                                   */
/* 990215 Richard Frostmark                                          */
/*        Added new primitives for ETSIMAP and IS41. CR 521 and 530  */
/*                                                                   */
/* 990225 Richard Frostmark                                          */
/*        Added new primitives for ISR FEIF. CR 492                  */
/*                                                                   */
/* 990322 Dan Liljemark                                              */
/*        Updated after review.                                      */
/*        Inspection master plan: 32/17017-CAA20129                  */
/*                                                                   */
/* 990701 Rustan Persson                                             */
/*	  Added new primitives for cello SAAL interface. CR 581      */
/*                                                                   */
/* 2000-02-23 Seamus Buns                                            */
/*            Added new primitives for ISUP for the LMF ONG (VoIpii) */
/*                                                                   */
/* 000410 Richard Frostmark                                          */
/*        Added new primitives for MAP ANSI-41. CR 669 and 663       */
/*                                                                   */
/* 000423 Richard Frostmark                                          */
/*        Added new primitives for stack 2000 (T_TIMER_RESET_REQ)    */
/*                                                                   */
/* 001215 Ulf Nilsson						     */
/*	  Added new primitives for DPC-dispatcher for MTPL3	     */
/*                                                                   */
/* 010118 Henrik Bergkvist                                           */
/*        Added new primitives for ANSI TCAP, ITU TCAP and ISUP      */
/*        Change Request 0749                                        */
/*                                                                   */
/* 010126 Henrik Bergkvist                                           */
/*        Added new primitive A41MAP_P_ABORT_IND                     */
/*        Change Request 0758                                        */
/*                                                                   */
/* 010130 Henrik Bergkvist                                           */
/*        Added new primitives for HD                                */
/*                                                                   */
/* 010206 Henrik Bergkvist                                           */
/*        Added new primitives for R7 ETSIMAP                        */
/*        Change Request 0765                                        */
/*                                                                   */
/* 010223 Henrik Bergkvist                                           */
/*        Added new primitives for TCAP and SCCP HD                  */
/*                                                                   */
/* 010226 Henrik Bergkvist                                           */
/*        Added new primitives NM_SSNBIND_REQ and NM_SSNUNBIND_REQ   */
/*        and changed names like NM_BEBIND_... to NM_RMBIND_...      */
/*        Added SCTP and M3UA primitives CR 0771.                    */
/*                                                                   */
/* 010306 Henrik Bergkvist                                           */
/*        Added new primitives SCTP_SIMULATED_IP_IND and             */
/*        SCTP_SIMULATED_IP_REQ                                      */
/*                                                                   */
/* 010306 QINXAGO                                                    */
/*        Added new primitives for FEIF HD.                          */
/*        Modify RM SCCP & NM SCCP primitive values.                 */
/*                                                                   */
/* 010329 Henrik Bergkvist                                           */
/*        Added new primitives for OAM                               */
/*                                                                   */
/* 010508 Henrik Bergkvist                                           */
/*        Added new primitive M3UA_DAUD_IND (CR 0798)                */
/*                                                                   */
/* 010918 Henrik Bergkvist                                           */
/*        Added primitives for ISR HD                                */
/*                                                                   */
/* 011212 Henrik Bergkvist                                           */
/*        Added primitives for MAP                                   */
/*********************************************************************/

/*************************************************************/
/******  OBS  ***  NOTE  ***  OBS  ***  NOTE  ***  OBS  ******/
/*************************************************************/
/* 
 * COMMONP Internal protocol reserved 50 - 55 and 240 - 254
 * NO OTHER MODULE THAN CP IS ALLOWED TO USE THIS INTERVAL!!!
 */
#define PRIVATE_CON_REQ      50
#define PRIVATE_CON_CONF     51
#define PRIVATE_CON_REL      52
#define PRIVATE_CON_REQ_AND_CONFIG   53
#define PRIVATE_CON_CONF_AND_CONFIG   53
#define PRIVATE_LOGDAEMON_CONFIG_REQ   54
#define HB_PRIMITIVE         245  
#define PRIVATE_CP_ERROR     246
#define PRIVATE_CP_CONFIG    247
#define SUBSCRIBE_CP_EVENT   248
/*************************************************************/
/******  OBS  ***  NOTE  ***  OBS  ***  NOTE  ***  OBS  ******/
/*************************************************************/




#define PRIVATE_CON_STATUS_OK             0
#define PRIVATE_CON_STATUS_BUSY           1
#define PRIVATE_CON_STATUS_USERUNKNOWN    2
#define PRIVATE_CON_STATUS_USERINTERNAL   3
#define PRIVATE_CON_STATUS_USERNOTAVAIL   4
#define PRIVATE_CON_STATUS_TIMEOUT        5
#define PRIVATE_CON_STATUS_USERDISABLED   6
#define PRIVATE_CON_STATUS_NOMGMT         7

#define PRIVATE_CON_ACTION_CONTINUE       0
#define PRIVATE_CON_ACTION_RETRY          1
#define PRIVATE_CON_ACTION_DIE            2


/* External Management request */
#define XM_INIT_REQ       10
#define XM_START_REQ      11
#define XM_STOP_REQ       12
#define XM_ALARM_REQ      13
#define XM_STATISTICS_REQ 14
#define XM_ORDER_REQ      15
#define XM_BIND_REQ       16
#define XM_UNBIND_REQ     17
#define XM_XSTAT_REQ      18
#define XM_MGMT_REQ       19
#define XM_STAT_REQ       20
#define XM_RESTART_REQ    22

/* External Management indication/confirm */
#define XM_INIT_CONF       16
#define XM_START_CONF      17
#define XM_STOP_CONF       18
#define XM_ALARM_CONF      19
#define XM_ALARM_IND       20
#define XM_STATISTICS_CONF 21
#define XM_ORDER_CONF      22
#define XM_BIND_CONF       23
#define XM_XSTAT_CONF      24
#define XM_MGMT_CONF       25
#define XM_ERROR_IND       26
#define XM_STAT_CONF       27
#define XM_SYSINFO_IND     28
#define XM_RESTART_CONF     29

/* Management request */

#define MM_INIT_REQ       1
#define MM_ALARM_REQ      2
#define MM_STATISTICS_REQ 3
#define MM_ORDER_REQ      4
#define MM_XSTAT_REQ      100
#define MM_STAT_REQ       101
#define MM_RESTART_REQ    121


/* Management indication/confirm */

#define MM_STAT_CONF       3
#define MM_XSTAT_CONF      4
#define MM_INIT_CONF       5
#define MM_ALARM_CONF      6
#define MM_ALARM_IND       7
#define MM_STATISTICS_CONF 8
#define MM_ORDER_CONF      9
#define MM_SYSINFO_IND	   21
#define MM_RESTART_CONF    23


/* Common Order Id included in MM_ORDER_REQ from management */

#define TERMINATE_REQ 1
#define START_REQ     2
#define TRACEON_REQ   3
#define TRACEOFF_REQ  4
/* (AHMA, 960920) Used for switch over */
#define REMAIN_ALIGNED 22


/* Constants used as result parameter in MM_X_conf primitive */

#define SUCCESS                  0
#define FILE_NOT_FOUND           1
#define FILE_COULD_NOT_OPEN      2
#define ORDER_NOT_PERFORMED      3
#define BAD_CONFIG               4
#define STATID_NOT_IMPLEMENTED   5
#define ORDER_NOT_IMPLEMENTED    6
#define ALARMID_NOT_IMPLEMENTED  7
#define FORMAT_ERROR             8
#define MM_RESOURCES_UNAVAILABLE 9
#define SSN_HAS_NOT_BOUND        10
#define ILLEGAL_STATE            11
#define MM_USER_ALLREADY_BOUND   12

#define EINSS7CP_SUCCESS SUCCESS       


/* MTP L2 */

#define DL_BIND_CONF          22
#define DL_MSU_IND            29
#define DL_INSERVICE_CONF     30
#define DL_OOS_IND            31
#define DL_RPO_IND            32
#define DL_RPOR_IND           33
#define DL_CONGESTED_IND      34
#define DL_CONGCEASE_IND      35
#define DL_RETRIEVEBSNT_CONF  40
#define DL_RETRIEVALMSG_IND   41
#define DL_RETRIEVALEND_CONF  42

#define DL_STATUS_CONF        43
#define DL_RBCLEARED_CONF     44
#define DL_RTBCLEARED_CONF    45
#define DL_ACK_CONF           64

#define DL_BIND_REQ           34     
#define DL_MSU_REQ            35    
#define DL_UNBIND_REQ         36    
#define DL_START_REQ          37    
#define DL_STOP_REQ           38
#define DL_EMERG_REQ          39
#define DL_EMERGCEASE_REQ     40
#define DL_LPO_REQ            41   
#define DL_LPOR_REQ           42   
#define DL_RETRIEVEBSNT_REQ   43   
#define DL_RETRIEVALSTART_REQ 44
#define DL_STATUS_REQ         45
#define DL_CONTINUE_REQ       46       
#define DL_FLUSHBUFFERS_REQ   47
#define DL_RTBCLEAR_REQ       48
#define WRITE_TXBUFFERS_REQ   56
#define DL_ACK_REQ            64

/* (AHMA, 960920) Internal BE, FE signals used for MT */
#define COMMUNICATION_UP_REQ     17
#define BROKEN_COMMUNICATION_REQ 18
#define FE_INIT_CONF             1

/* QINXAGO 010312: Added for FEIF HD */
#define FE_SSNBIND_REQ        57
#define FE_SSNUNBIND_REQ      58
#define FE_SSNSTATUS_IND      59
#define FE_SSNSTATUS_RESP     60
#define FE_UNBIND_REQ         61
#define FE_BIND_REQ           62
#define FE_BIND_CONF          63
#define FE_UNBIND_CONF        66

/* ISR card */

#define M2_STATISTICS_IND     4
#define M2_L1_INT_IND         5

#define M2_STATISTICS_REQ     13
#define M2_LPO_REQ            14
#define M2_LPOR_REQ           15
#define M2_FLUSHBUFFERS_REQ   16
#define M2_CONTINUE_REQ       21

#define M2_SIB_CONG_IND       23
#define M2_SIB_CEASED_IND     24

#define M2_BLOCKED_REQ	      25
#define M2_BLOCKCEASED_REQ    26
#define M2_STOP_REQ           27

/* module test of FE-MTP-L2-IF */
#define ISR_CLOSE_REQ         18
#define ISR_IOCTL_REQ         19
#define ISR_OPEN_REQ          20


/* MTP L3 */

#define MTP_TRANSFER_REQ   19
#define MTP_BIND_REQ       20
#define MTP_UNBIND_REQ     21

#define MTP_BIND_CONF      8            
#define MTP_PAUSE_IND      9
#define MTP_RESUME_IND     10
#define MTP_STATUS_IND     11   
#define MTP_TRANSFER_IND   12

#define DD_BIND_REQ	   150
#define DD_BIND_CONF	   151
#define DD_TRANSFER_IND	   152
#define DD_UNBIND_REQ	   153
#define DD_PAUSE_IND	   154
#define DD_RESUME_IND	   155
#define	DD_STATUS_IND	   156
#define DD_RSSTATUS_REQ	   157
#define DD_RSSTATUS_CONF   158
#define DD_TRANSFER_REQ    159

/* NM MTP-L3 */

#define NM3_ROUTEALLOW_IND  150
#define NM3_ROUTEPROHIB_IND 151
#define NM3_ROUTECONG_IND   152
#define NM3_MSU_IND         153
#define NM3_BUFFLINK_IND    154
#define NM3_REDIRLINK_IND   155
#define NM3_LINKAVAIL_IND   156
#define NM3_LINKUNAVAIL_IND 157


/* SCCP */

#define N_BIND_REQ         5
#define N_UNBIND_REQ       6
#define N_UNITDATA_REQ     7
#define N_CONNECT_REQ      13
#define N_CONNECT_RESP     14
#define N_DISCONNECT_REQ   15
#define N_DATA_REQ         16

#define N_BIND_CONF        19
#define N_NOTICE_IND       20
#define N_UNITDATA_IND     21
#define N_CONNECT_IND      22
#define N_CONNECT_CONF     23
#define N_DATA_IND         24
#define N_DISCONNECT_IND   25
#define N_STATE_IND        26

/* NM SCCP */

#define RM_SSNBIND_REQ      36
#define RM_SSNUNBIND_REQ    37
#define NM_RMBIND_CONF      38
#define NM_SSNSTATUS_IND    39
#define NM_SSNSTATUS_RESP   65
#define NM_SSNBIND_REQ      66
#define NM_SSNUNBIND_REQ    67
#define NM_FEBIND_CONF      68
#define NM_BIND_REQ         69
#define NM_BIND_CONF        70
#define NM_UNBIND_REQ       71
#define NM_UNBIND_CONF      72
#define NM_STATUS_IND       73
#define NM_STATUS_RESP      74


/* RM SCCP */

#define NM_SSNAVAILABLE_IND     36
#define NM_SSNUNAVAILABLE_IND   37
#define NM_SSNCONGESTED_IND     38
#define RM_SSNBIND_CONF         39
#define NM_RMBIND_REQ           40


/* SCTP primitives */

#define SCTP_ABORT_REQ                  6
#define SCTP_ASSOCIATE_REQ              7
#define SCTP_DESTROY_REQ                8
#define SCTP_INITIALIZE_REQ             9
#define SCTP_SEND_REQ                   10
#define SCTP_SET_PRIMARY_REQ            11
#define SCTP_SHUTDOWN_REQ               12
 
#define SCTP_DATA_ARRIVE_IND            10
#define SCTP_COMM_UP_IND                11
#define SCTP_COMM_LOST_IND              12
#define SCTP_COMM_ERROR_IND             13
#define SCTP_RESUME_SENDING_IND         14
#define SCTP_SEND_FAILURE_IND           15
#define SCTP_ASSOC_RESTART_IND          16
#define SCTP_NETWORK_STATUS_CHANGE_IND  17
#define SCTP_STOP_SENDING_IND           18
#define SCTP_INITIALIZE_CONF            22
#define SCTP_ASSOCIATE_CONF             23
#define SCTP_SHUTDOWN_CONF              24

#define SCTP_SIMULATED_IP_IND           31 /* Only for Module Test */
#define SCTP_SIMULATED_IP_REQ           32 /* Only for Module Test */


/* M3UA primitives */

#define M3UA_BIND_REQ                   25
#define M3UA_UNBIND_REQ                 26
#define M3UA_DATA_REQ                   27
#define M3UA_DUNA_REQ                   28
#define M3UA_DAVA_REQ                   29
#define M3UA_SCON_REQ                   30
#define M3UA_DUPU_REQ                   31

#define M3UA_BIND_CONF                  32
#define M3UA_DATA_IND                   33
#define M3UA_SCON_IND                   34
#define M3UA_STATUS_IND                 35
#define M3UA_DAUD_IND                   36


/* TCAP CCITT */

#define T_BIND_REQ       6
#define T_UNBIND_REQ     7
#define T_UNI_REQ        8
#define T_BEGIN_REQ      9
#define T_CONTINUE_REQ   10
#define T_END_REQ        11
#define T_U_ABORT_REQ    12
#define T_INVOKE_REQ     13
#define T_RESULT_NL_REQ  14
#define T_RESULT_L_REQ   15
#define T_U_ERROR_REQ    16
#define T_U_REJECT_REQ   17
#define T_U_CANCEL_REQ   18

#define T_BIND_CONF      20
#define T_UNI_IND        21
#define T_BEGIN_IND      22
#define T_CONTINUE_IND   23
#define T_END_IND        24
#define T_U_ABORT_IND    25
#define T_P_ABORT_IND    26
#define T_NOTICE_IND     27
#define T_INVOKE_IND     28
#define T_RESULT_NL_IND  29
#define T_RESULT_L_IND   30
#define T_U_ERROR_IND    31
#define T_U_REJECT_IND   32
#define T_L_REJECT_IND   33
#define T_R_REJECT_IND   34
#define T_L_CANCEL_IND   35
#define T_STATE_IND      36
/* rifr HD proj */
#define T_UNITDATA_IND   42

/* Change Request 0749, einhenb */
#define T_EXT_UNI_REQ       44
#define T_EXT_BEGIN_REQ     45
#define T_EXT_END_REQ       46
#define T_EXT_CONTINUE_REQ  47

#define T_EXT_UNI_IND       56
#define T_EXT_BEGIN_IND     57
#define T_EXT_END_IND       58
#define T_EXT_CONTINUE_IND  59
#define T_EXT_NOTICE_IND    62

/* TCAP ANSI */

#define T_BIND_REQ       6
#define T_UNBIND_REQ     7
#define T_UNI_REQ        8
#define T_QUERY_WP_REQ   9
#define T_CONV_WP_REQ    10
#define T_RESPONSE_REQ   11
#define T_U_ABORT_REQ    12
#define T_INVOKE_NL_REQ  13
#define T_RESULT_NL_REQ  14
#define T_RESULT_L_REQ   15
#define T_ERROR_REQ      16
#define T_R_REJECT_REQ   17
#define T_U_CANCEL_REQ   18

#define T_BIND_CONF      20
#define T_UNI_IND        21
#define T_QUERY_WP_IND   22
#define T_CONV_WP_IND    23
#define T_RESPONSE_IND   24
#define T_U_ABORT_IND    25
#define T_P_ABORT_IND    26
#define T_NOTICE_IND     27
#define T_INVOKE_NL_IND  28
#define T_RESULT_NL_IND  29
#define T_RESULT_L_IND   30
#define T_ERROR_IND      31

#define T_L_REJECT_IND   33
#define T_R_REJECT_IND   34
#define T_L_CANCEL_IND   35

#define T_QUERY_WOP_REQ  36
#define T_CONV_WOP_REQ   37
#define T_INVOKE_L_REQ   38

#define T_QUERY_WOP_IND  39
#define T_CONV_WOP_IND   40
#define T_INVOKE_L_IND   41


#define T_TIMER_RESET_REQ   43

/* Change Request 0749, einhenb*/
#define T_EXT_UNI_REQ        44
#define T_EXT_QUERY_WP_REQ   45
#define T_EXT_QUERY_WOP_REQ  46
#define T_EXT_RESPONSE_REQ   47
#define T_EXT_CONV_WP_REQ    48
#define T_EXT_CONV_WOP_REQ   49

/* Note 50-55 is reserved by Common Parts! */
#define T_EXT_UNI_IND        56
#define T_EXT_QUERY_WP_IND   57
#define T_EXT_QUERY_WOP_IND  58
#define T_EXT_RESPONSE_IND   59
#define T_EXT_CONV_WP_IND    60
#define T_EXT_CONV_WOP_IND   61
#define T_EXT_NOTICE_IND     62


/* MAP ETSI */
  /* Phase 1 events */

#define  MAP_BIND_REQ              121
#define  MAP_BIND_CONF             122
#define  MAP_UNBIND_REQ            123
#define  MAP_BEGIN_REQ             124
#define  MAP_BEGIN_IND             125
#define  MAP_END_REQ               126
#define  MAP_ERR_REQ               127
#define  MAP_ERR_IND               128

#define  MAP_DEL_SM_REQ            129
#define  MAP_RPT_SM_IND            130

#define  MAP_SUBMIT_SM_IND         131
#define  MAP_SUBMIT_SM_CONF        132
#define  MAP_ALERT_IND             133

  /* Phase 2 Dialogue events */

#define  MAP_OPEN_REQ              134
#define  MAP_OPEN_CONF             135
#define  MAP_OPEN_IND              136
#define  MAP_OPEN_RSP              137

#define  MAP_CLOSE_REQ             138
#define  MAP_CLOSE_IND             139

#define  MAP_DELIMIT_REQ           140
#define  MAP_DELIMIT_IND           141

#define  MAP_U_ABORT_REQ           142
#define  MAP_U_ABORT_IND           143

#define  MAP_P_ABORT_IND           144

#define  MAP_NOTICE_IND            145

  /* USSD Events */
  
#define  MAP_PROC_USSD_REQ         146   /* phase 2 only */
#define  MAP_PROC_USSD_CONF        147   /* phase 2 only */
#define  MAP_PROC_USSD_IND         148   /* phase 2 only */
#define  MAP_PROC_USSD_RSP         149   /* phase 2 only */

#define  MAP_USSD_REQ              150
#define  MAP_USSD_CONF             151
#define  MAP_USSD_IND              152
#define  MAP_USSD_RSP              153

#define  MAP_USSD_NOTIFY_REQ       154
#define  MAP_USSD_NOTIFY_CONF      155
#define  MAP_USSD_NOTIFY_IND       156
#define  MAP_USSD_NOTIFY_RSP       157

  /* Phase 2 Short Message Service Events */

#define  MAP_SND_RINFO_SM_REQ      158
#define  MAP_SND_RINFO_SM_CONF     159
#define  MAP_SND_RINFO_SM_IND      160
#define  MAP_SND_RINFO_SM_RSP      161

#define  MAP_FWD_SM_REQ            162
#define  MAP_FWD_SM_CONF           163
#define  MAP_FWD_SM_IND            164
#define  MAP_FWD_SM_RSP            165
  
#define  MAP_RPT_SM_DEL_REQ        166
#define  MAP_RPT_SM_DEL_CONF       167
#define  MAP_RPT_SM_DEL_IND        168
#define  MAP_RPT_SM_DEL_RSP        169

#define  MAP_RDY_FOR_SM_REQ        170
#define  MAP_RDY_FOR_SM_CONF       171
#define  MAP_RDY_FOR_SM_IND        172
#define  MAP_RDY_FOR_SM_RSP        173

#define  MAP_ALERT_SC_REQ          174
#define  MAP_ALERT_SC_CONF         175
#define  MAP_ALERT_SC_IND          176
#define  MAP_ALERT_SC_RSP          177
  
#define  MAP_INFORM_SC_REQ         178
#define  MAP_INFORM_SC_IND         179

#define  MAP_STATE_IND             180

/* EMAP USSD primitives */

#define  MAP_EMAP_USSD_REQ         181
#define  MAP_EMAP_USSD_CONF        182
#define  MAP_EMAP_USSD_NOTIFY_REQ  183
#define  MAP_EMAP_USSD_NOTIFY_CONF 184

#define MAP_V1_FWD_SM_REQ          185
#define MAP_V1_FWD_SM_IND          186
#define MAP_V1_FWD_SM_RSP          187
#define MAP_V1_FWD_SM_CONF         188
#define MAP_V1_RPT_SM_DEL_REQ      189
#define MAP_V1_RPT_SM_DEL_CONF     190
#define MAP_V1_ALERT_SC_IND        191
#define MAP_V1_SND_RINFO_SM_CONF   192
#define MAP_V1_SND_RINFO_SM_REQ    193
#define MAP_GET_AC_VERSION_REQ     194
#define MAP_GET_AC_VERSION_CONF    195
#define MAP_FWD_SM_MO_REQ          196
#define MAP_FWD_SM_MO_CONF         197

/* End of MAP ETSI */

/* Phase2+ Mobility Services Events */

#define MAP_ANY_TIME_INTERROGATION_REQ          200
#define MAP_ANY_TIME_INTERROGATION_IND          201
#define MAP_ANY_TIME_INTERROGATION_CONF         202
#define MAP_ANY_TIME_INTERROGATION_RESP         203

#define MAP_V3_SEND_ROUTING_INFO_FOR_LCS_REQ    204
#define MAP_V3_SEND_ROUTING_INFO_FOR_LCS_CONF   205
#define MAP_V3_PROVIDE_SUBSCRIBER_LOCATION_REQ  206
#define MAP_V3_PROVIDE_SUBSCRIBER_LOCATION_CONF 207
#define MAP_V3_SUBSCRIBER_LOCATION_REPORT_IND   208
#define MAP_V3_SUBSCRIBER_LOCATION_REPORT_RESP  209

/* EINHENB 010206 CR 0765 */
#define MAP_V3_FWD_SM_MT_REQ                    210
#define MAP_V3_FWD_SM_MT_CONF                   211
#define MAP_V3_FWD_SM_MO_IND                    212
#define MAP_V3_FWD_SM_MO_RSP                    213
#define MAP_V3_SND_RINFO_SM_REQ                 214
#define MAP_V3_SND_RINFO_SM_CONF                215
#define MAP_V3_RPT_SM_DEL_REQ                   216
#define MAP_V3_RPT_SM_DEL_CONF                  217
#define MAP_V3_INFORM_SC_IND                    218
#define MAP_V3_ALERT_SC_IND                     219
#define MAP_V3_ALERT_SC_RSP                     220

/* cr 785 */
#define MAP_BEGIN_SUB_ACTIVITY_REQ              221
#define MAP_BEGIN_SUB_ACTIVITY_CONF             222
#define MAP_BEGIN_SUB_ACTIVITY_IND              223
#define MAP_BEGIN_SUB_ACTIVITY_RSP              224
#define MAP_PROC_USSD_DATA_REQ                  225
#define MAP_PROC_USSD_DATA_CONF                 226
#define MAP_PROC_USSD_DATA_IND                  227
#define MAP_PROC_USSD_DATA_RSP                  228

#define MAP_V3_PROVIDE_SUBSCRIBER_INFO_REQ	229
#define MAP_V3_PROVIDE_SUBSCRIBER_INFO_CONF	230

/* MAP ANSI-4I */

#define A41MAP_SMSREQ_REQ          6
#define A41MAP_BIND_REQ            7
#define A41MAP_UNBIND_REQ          8
#define A41MAP_SMDPP_REQ           9
#define A41MAP_SMDPP_RESP         10
#define A41MAP_SMSNOT_RESP        11
#define A41MAP_REGISTRATION_RESP  12
#define A41MAP_REGCANCEL_RESP     13
#define A41MAP_MSINACTIVE_RESP    14
#define A41MAP_ORIGINATION_RESP   15
#define A41MAP_OTASP_REQ          16
#define A41MAP_ISPOSREQ_RESP      17
#define A41MAP_ISPOSCANC_RESP     18
#define A41MAP_ISPOSDIR_REQ       19
#define A41MAP_FEATREQ_RESP       56
#define A41MAP_REDDIR_REQ         57
#define A41MAP_ROUTREQ_REQ        58
#define A41MAP_ROUTREQ_RESP       59
#define A41MAP_ESPOSREQ_RESP      61        
#define A41MAP_LOCREQ_REQ         62
#define A41MAP_QUERY_RESP         60 


#define A41MAP_SMSREQ_CONF         6
#define A41MAP_BIND_CONF           7
#define A41MAP_SMDPP_CONF          8
#define A41MAP_SMSNOT_IND          9
#define A41MAP_SMDPP_IND          10
#define A41MAP_STATE_IND          11
#define A41MAP_REGISTRATION_IND   12
#define A41MAP_REGCANCEL_IND      13
#define A41MAP_MSINACTIVE_IND     14
#define A41MAP_ORIGINATION_IND    15
#define A41MAP_OTASP_CONF         16
#define A41MAP_ISPOSREQ_IND       17
#define A41MAP_ISPOSCANC_IND      18
#define A41MAP_ISPOSDIR_CONF      19
#define A41MAP_FEATREQ_IND        56
#define A41MAP_REDDIR_CONF        57
#define A41MAP_ROUTREQ_CONF       58
#define A41MAP_ROUTREQ_IND        59

#define A41MAP_ESPOSREQ_IND       61
#define A41MAP_LOCREQ_CONF        62
#define A41MAP_QUERY_IND          60

/* CR 0758 einhenb */
#define A41MAP_P_ABORT_IND        63

/* End of MAP ANSI-41 */


/* IS4I */

#define IS41_DELIVER_REQ        5
#define IS41_PROBE_REQ          6
#define IS41_BIND_REQ           7
#define IS41_UNBIND_REQ         8
#define IS41_SMSDIRECT_REQ      9
#define IS41_SMSDIRECT_RESP     10
#define IS41_ALERT_RESP         11
#define IS41_REGISTRATION_RESP  12
#define IS41_REGCANCEL_RESP     13
#define IS41_MSINACTIVE_RESP    14
#define IS41_ORIGINATION_RESP   15
#define IS41_OTASP_REQ          16
#define IS41_ISPOSREQ_RESP      17
#define IS41_ISPOSCANC_RESP     18
#define IS41_ISPOSDIR_REQ       19


#define IS41_REPORT_IND         5
#define IS41_PROBE_CONF         6
#define IS41_BIND_CONF          7
#define IS41_SMSDIRECT_CONF     8
#define IS41_ALERT_IND          9
#define IS41_SMSDIRECT_IND      10
#define IS41_STATE_IND          11
#define IS41_REGISTRATION_IND   12
#define IS41_REGCANCEL_IND      13
#define IS41_MSINACTIVE_IND     14
#define IS41_ORIGINATION_IND    15
#define IS41_OTASP_CONF         16
#define IS41_ISPOSREQ_IND       17
#define IS41_ISPOSCANC_IND      18
#define IS41_ISPOSDIR_CONF      19

/* INAP */

#define INAP_BIND_REQ       10
#define INAP_UNBIND_REQ     11
#define INAP_RETRIEVE_RESP  12
#define INAP_UPDATE_RESP    13
#define INAP_ABORT_REQ      14
#define INAP_BEGIN_REQ      15
#define INAP_CANCEL_REQ     16
#define INAP_DATA_REQ       17
#define INAP_END_REQ        18

#define INAP_BIND_CONF      14
#define INAP_RETRIEVE_IND   15
#define INAP_UPDATE_IND     16
#define INAP_ABORT_IND      17
#define INAP_BEGIN_IND      18
#define INAP_DATA_IND       19
#define INAP_END_IND        20
#define INAP_ERROR_IND      21
#define INAP_NOTICE_IND     22
#define INAP_P_ABORT_IND    23
#define INAP_STATE_IND      24

/* VAP */

#define VAP_BIND_REQ                    10
#define VAP_UNBIND_REQ                  11
#define VAP_PERSONALISEMAILBOX_RESP     12
#define VAP_DEPERSONALISEMAILBOX_RESP   13
#define VAP_INFORMMESSAGEWAITING_REQ    14
#define VAP_INFORMMESSAGERETRIEVED_REQ  15
#define VAP_INITIATEALERT_RESP          16
#define VAP_INFORMALERTRESULT_REQ       17
#define VAP_ENQUIREONMAILBOX_RESP       18

#define VAP_BIND_CONF                   20
#define VAP_PERSONALISEMAILBOX_IND      21
#define VAP_DEPERSONALISEMAILBOX_IND    22
#define VAP_INFORMMESSAGEWAITING_CONF   23
#define VAP_INFORMMESSAGERETRIEVED_CONF 24
#define VAP_INITIATEALERT_IND           25
#define VAP_INFORMALERTRESULT_CONF      26
#define VAP_ENQUIREONMAILBOX_IND        27
#define VAP_STATE_IND                   28

/* TMC */

#define TMC_SUBMITSHORTMESSAGE_REQ      60
#define TMC_DELETESHORTMESSAGE_REQ      61
#define TMC_REPLACESHORTMESSAGE_REQ     62
#define TMC_DELETEALLSHORTMESSAGE_REQ   63
#define TMC_ENQUIREONSHORTMESSAGE_REQ   64
#define TMC_CANCELSTATUSREPORT_REQ      65
#define TMC_ALERTREQUEST_REQ            66
#define TMC_STATUSREPORT_RESP           67
#define TMC_ALERTSME_RESP               68

#define TMC_SUBMITSHORTMESSAGE_CONF     69
#define TMC_DELETESHORTMESSAGE_CONF     70
#define TMC_REPLACESHORTMESSAGE_CONF    71
#define TMC_DELETEALLSHORTMESSAGE_CONF  72
#define TMC_ENQUIREONSHORTMESSAGE_CONF  73
#define TMC_CANCELSTATUSREPORT_CONF     74
#define TMC_ALERTREQUEST_CONF           75
#define TMC_STATUSREPORT_IND            76
#define TMC_ALERTSME_IND                77

/* TUP */

#define TUP_BIND_REQ            13
#define TUP_UNBIND_REQ          14
#define TUP_SETUP_REQ           15
#define TUP_SETUP_RESP          16
#define TUP_ALERT_REQ           17
#define TUP_RELEASE_REQ         18
#define TUP_RELEASE_RESP        19
#define TUP_I_RESUME            20  /* 951115 MiLi Added */

#define TUP_BIND_CONF           21  /* 951115 MiLi Increased by 1 */
#define TUP_SETUP_IND           22  /* 951115 MiLi Increased by 1 */
#define TUP_RELEASE_IND         23  /* 951115 MiLi Increased by 1 */
#define TUP_FREE_IND            24  /* 951115 MiLi Increased by 1 */
#define TUP_ALERT_IND           25  /* 951115 MiLi Increased by 1 */
#define TUP_SETUP_CONF          26  /* 951115 MiLi Increased by 1 */
#define TUP_RESOURCE_IND        27  /* 951115 MiLi Increased by 1 */
#define TUP_MISC_REQ            28  /* DeEr 970612 Added */
#define TUP_MISC_IND                    29      /* DeEr 970612 Added */
#define TUP_ACI_IND                     30
#define TUP_NEED_IND                    31
#define TUP_ACI_CONF                    32      /* 980420 Pelt Added CR381 */
#define TUP_ACI_REQ                     33
#define TUP_NEED_REQ                    34
#define TUP_CIRCUIT_RESERVATION_REQ     35
#define TUP_CIRCUIT_RESERVATION_CONF    36
#define TUP_CIRCUIT_CANCELLATION_REQ    37
#define TUP_CIRCUIT_CANCELLATION_CONF   38
#define TUP_CLEAR_REQ                   39
#define TUP_CLEAR_IND                   40
#define TUP_REANSWER_REQ                41
#define TUP_REANSWER_IND                42

/* ISUP */

#define ISUP_BIND_REQ           13
#define ISUP_UNBIND_REQ         14
#define ISUP_SETUP_REQ          15
#define ISUP_SETUP_RESP         16
#define ISUP_RELEASE_REQ        17
#define ISUP_ALERT_REQ          18
#define ISUP_PROG_REQ           19
#define ISUP_IBI_REQ            20
#define ISUP_PROC_REQ           21
#define ISUP_SUSPENDED_REQ      22
#define ISUP_RESUMED_REQ        23
#define ISUP_RESET_REQ          24
#define ISUP_RELEASE_RESP       25
#define ISUP_RESET_RESP         26

#define ISUP_BIND_CONF          27
#define ISUP_SETUP_IND          28
#define ISUP_SETUP_CONF         29
#define ISUP_RELEASE_IND        30
#define ISUP_RELEASE_CONF       31
#define ISUP_ALERT_IND          32
#define ISUP_PROG_IND           33
#define ISUP_IBI_IND            34
#define ISUP_PROC_IND           35
#define ISUP_SUSPENDED_IND      36
#define ISUP_RESUMED_IND        37
#define ISUP_RESET_IND          38
#define ISUP_RESET_CONF         39
#define ISUP_RESOURCE_IND       40

#define ISUP_SERVICE_REQ        41
#define ISUP_CONTINUITY_RESP    42
#define ISUP_SERVICE_IND        43
#define ISUP_CONTINUITY_IND     44
#define ISUP_CONTINUITY_REQ     45
#define ISUP_CONTINUITY_CONF    46
#define ISUP_MISC_REQ           47
#define ISUP_MISC_IND           48
#define ISUP_PROCEED_REQ        49

/* NOTE! 50 - 55 is reserved */
#define ISUP_PROGRESS_REQ               56 
#define ISUP_PROCEED_IND                57
#define ISUP_PROGRESS_IND               58
#define ISUP_CONG_IND                   59
#define ISUP_REPORT_IND                 60

/* 991206 - qinxsbu - LMF New Primitives */
#define ISUP_CIRCUIT_CANCELLATION_REQ   61
#define ISUP_CIRCUIT_CANCELLATION_CONF  62
#define ISUP_CIRCUIT_RESERVATION_REQ    63
#define ISUP_CIRCUIT_RESERVATION_CONF   64
#define ISUP_CIRCUIT_CANCELLATION_IND   65

/* Change Request 0749, einhenb */
#define ISUP_APM_REQ                    65
#define ISUP_PRI_REQ                    66
#define ISUP_APM_IND                    67
#define ISUP_PRI_IND                    68

/* For module testing of level 2 interface */

#define CP_SETEVENTS_REQ                 1
#define CP_REMEVENTS_REQ                 2
#define APC7_OPEN_REQ                    3
#define APC7_IOCTL_REQ                   4
#define APC7_FLUSH_REQ                   5
#define APC7_WRITE_REQ                   6
#define APC7_CLOSE_REQ                   7

#define APC7_OPENRES_REQ                 1
#define APC7_IOCTLRES_REQ                2
#define APC7_WRITERES_REQ                3
#define APC7_STRIO_IND                   4

/* For module testing of cello SAAL interface */

#define SAAL_ATTACH_REQ			 1
#define SAAL_DETACH_REQ			 2
#define SAAL_START_REQ			 3
#define SAAL_STOP_REQ			 4
#define SAAL_EMERG_REQ			 5
#define SAAL_EMERGCEASE_REQ		 6
#define SAAL_RETRIEVEBSNT_REQ		 7
#define SAAL_RETRIEVALSTART_REQ		 8
#define SAAL_STATUS_REQ			 9
#define SAAL_MSU_REQ			 10

#define SAAL_ATTACHRES_REQ		 1
#define SAAL_SIGNAL_IND			 2


/* Unisys VNMS_AL */
#define VAL_BIND_REQ                  113
#define VAL_BIND_CONF                 114
#define VAL_UNBIND_REQ                115
#define VAL_INITIATE_REQ              116
#define VAL_INITIATE_IND              117
#define VAL_ALERT_IND                 118
#define VAL_ANSWER_REQ                119
#define VAL_ANSWER_IND                120
#define VAL_ANSWER_CONF               121
#define VAL_TERMINATE_REQ             122
#define VAL_TERMINATE_IND             123
#define VAL_NETWORK_OP_REQ            124
#define VAL_NETWORK_OP_IND            125
#define VAL_NETWORK_OP_CONF           126
#define VAL_PERFORM_SIGNALING_REQ     127
#define VAL_PERFORM_SIGNALING_CONF    128
#define VAL_CONTINUITY_RESP           129
#define VAL_CONTINUITY_CONF           130

#define VAL_CIRCUIT_RESET_REQ         141  
#define VAL_CIRCUIT_RESET_IND         142  
#define VAL_CIRCUIT_RESET_CONF        143
#define VAL_SUSPEND_CIRCUIT_REQ       144
#define VAL_SUSPEND_CIRCUIT_IND       145
#define VAL_SUSPEND_CIRCUIT_CONF      146
#define VAL_RESUME_CIRCUIT_REQ        147
#define VAL_RESUME_CIRCUIT_IND        148
#define VAL_RESUME_CIRCUIT_CONF       149
#define VAL_CIRCUIT_INQUIRY_REQ       150
#define VAL_CIRCUIT_INQUIRY_CONF      151
#define VAL_HOST_UNAVAILABLE_REQ      152
#define VAL_DPC_AVAILABLE_IND         153
#define VAL_DPC_UNAVAILABLE_IND       154

/* For module testing of Artesyn level 2 interface */
#define PPE_OPEN_REQ        3
#define PPE_CLOSE_REQ       4
#define PPE_IOCTL_REQ       5
#define PPE_WRITE_REQ       6
#define PPE_READ_REQ        7
#define PPE_READY_REQ       8
#define PPE_BOOT_REQ        9
#define PPE_APIINIT_REQ    10
#define PPE_FDREM_REQ      11
#define PPE_FDGET_REQ      12
#define PPE_INIT_REQ       13
#define PPE_REBOOT_REQ     14

#define PPE_APIINITRES_REQ  1
#define PPE_FDGETRES_REQ    2
#define PPE_OPENRES_REQ     3
#define PPE_WRITERES_REQ    4
#define PPE_EVT_IND         5
#define PPE_MSU_IND         6

/* HA */
#define HA_INITIATE                      1
#define HA_SWITCHOVER_REQ                2
#define HA_TERMINATE_REQ                 3
#define HA_PROCESS_DIED_IND              4
#define HA_PROCESS_STARTED_IND           5
#define HA_TERMINATE_IND                 6

/* OAM */
#define OAM_BIND_REQ      1
#define OAM_UNBIND_REQ    2
#define OAM_INIT_REQ      3
#define OAM_START_REQ     4
#define OAM_STOP_REQ      5
#define OAM_ADD_REQ       6
#define OAM_ALARM_REQ    10
#define OAM_ORDER_REQ    11
#define OAM_STAT_REQ     12
#define OAM_MGMT_REQ     15
#define OAM_PCF_REQ      17
#define OAM_BIND_CONF   101
#define OAM_INIT_CONF   103
#define OAM_START_CONF  104
#define OAM_STOP_CONF   105
#define OAM_ALARM_CONF  110
#define OAM_ORDER_CONF  111
#define OAM_STAT_CONF   112
#define OAM_MGMT_CONF   115
#define OAM_PCF_CONF    117
#define OAM_ERROR_IND   200
#define OAM_BIND_IND    201
#define OAM_ALARM_IND   210
#define OAM_SYSINFO_IND 215
#define OAM_PCF_IND     217

/* PCF */
#define PCFMESSAGE	100
#define PCF_SWUPGRADE	101

/* DISTRIBUTOR */
#define DISTR_BIND_REQ		30
#define DISTR_BIND_CONF		31
#define DISTR_UNBIND_REQ	32
#define DISTR_UNBIND_CONF	33
#define DISTR_STATUS_IND	34
#define DISTR_STATUS_RESP	35


/* Super visor */
#define SV_MASTER_REQ                    10
#define SV_MASTER_IND                    11
#define SV_MASTER_CONF                   12
#define SV_BAD_CONNECTION_IND            13
#define SV_SWITCHOVER_REQ                14
#define SV_BROKEN_COM_IND                15

#define SV_FE_POLL_REQ                   20
#define SV_FE_POLL_CONF                  21



/* Timers */
#define X_TIMEREXP_IND               0


/* For test purposes only */
#ifndef UNKNOWN
#define UNKNOWN                  255
#endif /* UNKNOWN */

#endif /* __SS7MSG_H__ */
