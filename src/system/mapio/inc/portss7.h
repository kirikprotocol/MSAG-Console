#ifndef __PORTSS7_H__
#define __PORTSS7_H__
/*********************************************************************/
/*                                                                   */
/* portss7.h,v                                                       */
/*                                                                   */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* COPYRIGHT Ericsson Infotech AB 1998                               */
/*                                                                   */
/* The copyright to the computer  program herein is the property of  */
/* Ericsson Infotech AB. The program may be used and/or              */
/* copied only with the written permission from Ericsson Infotech AB */
/* or in the accordance with the terms and conditions                */
/* stipulated in the agreement/contract under which the program has  */
/* been supplied.                                                    */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Document Number:                 (Rev:    )                       */
/* 8/190 55-CAA 201 29 Ux                                            */
/*                                                                   */
/* Revision:                                                         */
/* @EINVER: portss7.h,v 1.37 2000/06/26 15:27:21 Exp                 */
/*                                                                   */
/* Programmer: Martin Wessmark                                       */
/*             Pär Larsson                                           */
/*                                                                   */
/* Purpose:    Module identities                                     */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Revision record:                                                  */
/*                                                                   */
/* 931010 M.Hammarlund Added SCCP_INIT_ID                            */
/* 940427 C.Lindberg   Added different error-offsets                 */
/* 950307 D.Eriksson   Added makeVTAG macro                          */
/* 960409 H.Berglund   Added ANY_ID for use in new common parts      */
/* 960920 A.Mahmoudi   Added module IDs for BE-if, FE-if, and L2-if  */
/* 961120 H.Berglund   Added module APC7 for l2if module testing     */
/* 970219 M.Wessmark   Added Error Offset for Unisys-SM (24000)      */
/* 970616 D.Liljemark  Added INAP01_ID                               */
/* 990226 D.Liljemark  Added ISRL2 IDs and error offset              */
/* 990505 R.Frostmark  Added LIC_ID                                  */
/* 010126 H.Bergkvist  Added A41MAP01_ID                             */
/* 010130 H.Bergkvist  Added SCTP_ID and M3UA_ID and error offsets   */
/* 010206 H.Bergkvist  Added ETSIMAP2_ID                             */
/* 010312 eemapgo      Added FEIF_ID                                 */
/* 010329 H.Bergkvist  Added OAM, PCF, MM_FE id's and error offset   */
/* 010423 H.Bergkvist  Added DISTRIBUTOR_ERROR_OFFSET                */
/* 010918 H.Bergkvist  Added LICENSE_ID and Error offset             */
/* 011116 eemjfmn      Added SCTP_IF_ID and MGMT_SCTP_ID             */
/* 011119 eemjfmn      Added SCTP_IF_ERROR_OFFSET                    */
/*                                                                   */
/*********************************************************************/

#include "ss7osdpn.h"

/*********************************************************************/
/*                                                                   */
/*                        C O N S T A N T S                          */
/*                                                                   */
/*********************************************************************/
/*-------------------------------------------------------------------*/
/* Portss7 Unit identifiers                                          */
/*-------------------------------------------------------------------*/

/* The id's below are used as unit identifiers in process-           */
/* communication , extended memory handling                          */
/* and timer services.                                               */


#define SS7_BASE_ID       0 /* base id and id for linked stack */
#define MOBILE_ID         1   
#define PLMN_ID           2
#define MAP_ID            3
#define TCAP_ID           4
#define SCCP_ID           5
#define MTPL3_ID          6 
#define MGMT_ID           7
#define TESTREADER_ID     8 
#define TESTLOGGER_ID     9 
#define TUP_ID            10 
#define DUP_ID            11 
#define ISUP_ID           12 
#define MTPL2_ID          13 
#define MTPL2R1_ID        14 
#define MTPL2R2_ID        15 
#define SCCP_INIT_ID      16
#define TUPUP_ID          17
#define ISUPUP_ID         18
#define DUPUP_ID          19
#define BSSAP_ID          20
#define INAP_ID           21
#define INAPUP_ID         22
#define XMGMT_ID          23
#define CC_ID             24
#define IWE_ID            25
#define OM_ID             26
#define MMI_ID            27
#define MAPUP_ID          28
#define ISTUP_ID          29
#define IS41_ID           30
#define XMGMT2_ID         31
#define MTPL2R3_ID        32 /*********************************/
#define MTPL2R4_ID        33 /* FOR SUPPORTING UP TO 10 CARDS */
#define MTPL2R5_ID        34 /*                               */
#define MTPL2R6_ID        35 /* CURRENTLY USED ONLY BY ADAX   */
#define MTPL2R7_ID        36 /*                               */
#define MTPL2R8_ID        37 /* CHANGES HAVE TO BE MADE IN    */
#define MTPL2R9_ID        38 /* THE GLUE LAYER!               */
#define MTPL2R10_ID       39 /*********************************/
#define USER01_ID         40 /*********************************/
#define USER02_ID         41 /* User defined IDs for dynamic  */
#define USER03_ID         42 /* use. (40-59)                  */
#define USER04_ID         43 /* F ex 3 user definable TC-U    */
#define USER05_ID         44 /* or MAP-U...                   */
#define USER06_ID         45 /*                               */
#define USER07_ID         46 /* Do not CHANGE these IDs !!!   */
#define USER08_ID         47 /*                               */
#define USER09_ID         48 /* Reserved !!!                  */
#define USER10_ID         49 /* Reserved !!!                  */
#define USER11_ID         50 /* Reserved !!!                  */
#define USER12_ID         51 /* Reserved !!!                  */
#define USER13_ID         52 /* Reserved !!!                  */
#define USER14_ID         53 /* Reserved !!!                  */
#define USER15_ID         54 /* Reserved !!!                  */
#define USER16_ID         55 /* Reserved !!!                  */
#define USER17_ID         56 /* Reserved !!!                  */
#define USER18_ID         57 /* Reserved !!!                  */
#define USER19_ID         58 /* Reserved !!!                  */
#define USER20_ID         59 /*********************************/
#define MTPL3T_ID         60
#define MTPL3C_ID         61
#define VNMS_ID           62
#define EMAP_ID           63
#define VAP_ID            64
#define VAPUP_ID          65
#define IS41UP_ID         66
#define FEIF01_ID         67
#define FEIF02_ID         68
#define FEIF03_ID         69
#define FEIF04_ID         70
#define FEIF05_ID         71
#define FEIF06_ID         72
#define FEIF07_ID         73
#define FEIF08_ID         74
#define FEIF09_ID         75
#define FEIF10_ID         76
#define L201_ID           77
#define L202_ID           78
#define L203_ID           79
#define L204_ID           80
#define L205_ID           81
#define L206_ID           82
#define L207_ID           83
#define L208_ID           84
#define L209_ID           85
#define L210_ID           86
#define BEFEIF_ID         87
#define APC7_ID           88
#define HAMON0_ID         89
#define HAMON1_ID         90
#define HAMON2_ID         91
#define HAMON3_ID         92
#define HAMON4_ID         93
#define HAMON5_ID         94
#define HAMON6_ID         95
#define HAMON7_ID         96
#define HAMON8_ID         97
#define HAMON9_ID         98
#define HAMON_ID          99
#define HACTRL_ID        100
#define PSMON_ID         101
#define SS7CTRL_ID       102
#define TCAP01_ID        103
#define INAP01_ID        104
#define ANSITCAP_ID      105
#define ANSITCAP01_ID    106
#define ETSIMAP_ID       107
#define SS7IPLOCAL_ID    108
#define SS7IP1_ID        109
#define SS7IP2_ID        110
#define SS7IP3_ID        111
#define SS7IP4_ID        112
#define SS7IP5_ID        113
#define CP_ID            114
#define DAEMON_ID        115
#define SNMPAGENT_ID     116
#define BEFEIFFAST_ID    117
#define ISRL201_ID       118
#define ISRL202_ID       119
#define ISRL203_ID       120
#define ISRL204_ID       121
#define ISRL205_ID       122
#define ISRL206_ID       123
#define ISRL207_ID       124
#define ISRL208_ID       125
#define LIC_ID           126
#define DECODER_ID       127
#define MISC01_ID        128
#define MISC02_ID        129
#define MISC03_ID        130
#define MISC04_ID        131
#define MISC05_ID        132
#define MISC06_ID        133
#define MISC07_ID        134
#define MISC08_ID        135
#define MISC09_ID        136
#define MISC10_ID        137
#define A41MAP_ID        138
#define A41MAPUP_ID      139
#define VNMS_AL_ID       140
#define BSSAPLEUP_ID     141
#define MGMT1_ID         142
#define MGMT2_ID         143
#define SAALIF01_ID      144
#define SAALIF02_ID      145
#define SAALIF03_ID      146
#define SSCF01_ID        147
#define SSCF02_ID        148
#define SSCF03_ID        149
#define SSCOP01_ID       150
#define SSCOP02_ID       151
#define SSCOP03_ID       152
#define CPCS01_ID        153
#define CPCS02_ID        154
#define CPCS03_ID        155
#define A41MAP01_ID      156
#define SCTP_ID          157
#define M3UA_ID          158
#define ETSIMAP2_ID      159
#define FEIF_ID          160
#define CP_MANAGER_ID    161
#define OAM_ID           162
#define PCF_ID           163
#define MGMT_FE_ID       164
#define DIST_ID          165
#define LICENSE_ID       166
#define SCTP_IF_ID       167
#define MGMT_SCTP_ID     168

#define MAX_ID_NR        169

#define MAX_USERS     MAX_ID_NR

#define MODULE_NAMES \
"SS7_BASE",\
"MOBILE",\
"PLMN",\
"MAP",\
"TCAP",\
"SCCP",\
"MTPL3",\
"MGMT",\
"TESTREADER",\
"TESTLOGGER",\
"TUP",\
"DUP",\
"ISUP",\
"MTPL2",\
"MTPL2R1",\
"MTPL2R2",\
"SCCP_INIT",\
"TUPUP",\
"ISUPUP",\
"DUPUP",\
"BSSAP",\
"INAP",\
"INAPUP",\
"XMGMT",\
"CC",\
"IWE",\
"O&M",\
"MMI",\
"MAPUP",\
"ISTUP",\
"IS41",\
"XMGMT2",\
"MTPL2R3",\
"MTPL2R4",\
"MTPL2R5",\
"MTPL2R6",\
"MTPL2R7",\
"MTPL2R8",\
"MTPL2R9",\
"MTPL2R10",\
"USER01",\
"USER02",\
"USER03",\
"USER04",\
"USER05",\
"USER06",\
"USER07",\
"USER08",\
"USER09",\
"USER10",\
"USER11",\
"USER12",\
"USER13",\
"USER14",\
"USER15",\
"USER16",\
"USER17",\
"USER18",\
"USER19",\
"USER20",\
"MTPL3T",\
"MTPL3C",\
"VNMS",\
"EMAP",\
"VAP",\
"VAPUP",\
"IS41UP",\
"FEIF01",\
"FEIF02",\
"FEIF03",\
"FEIF04",\
"FEIF05",\
"FEIF06",\
"FEIF07",\
"FEIF08",\
"FEIF09",\
"FEIF10",\
"L201",\
"L202",\
"L203",\
"L204",\
"L205",\
"L206",\
"L207",\
"L208",\
"L209",\
"L210",\
"BEFEIF",\
"APC7",\
"HAMON0",\
"HAMON1",\
"HAMON2",\
"HAMON3",\
"HAMON4",\
"HAMON5",\
"HAMON6",\
"HAMON7",\
"HAMON8",\
"HAMON9",\
"HAMON",\
"HACTRL",\
"PSMON",\
"SS7CTRL",\
"TCAP01",\
"INAP01",\
"ANSITCAP",\
"ANSITCAP01",\
"ETSIMAP",\
"SS7IPLOCAL",\
"SS7IP1",\
"SS7IP2",\
"SS7IP3",\
"SS7IP4",\
"SS7IP5",\
"CP",\
"DAEMON",\
"SNMPAGENT",\
"BEFEIFFAST",\
"ISRL201",\
"ISRL202",\
"ISRL203",\
"ISRL204",\
"ISRL205",\
"ISRL206",\
"ISRL207",\
"ISRL208",\
"LIC", \
"DECODER", \
"MISC01", \
"MISC02", \
"MISC03", \
"MISC04", \
"MISC05", \
"MISC06", \
"MISC07", \
"MISC08", \
"MISC09", \
"MISC10", \
"A41MAP", \
"A41MAPUP", \
"VNMS_AL", \
"BSSAPLEUP", \
"MGMT1", \
"MGMT2", \
"SAALIF01", \
"SAALIF02", \
"SAALIF03", \
"SSCF01", \
"SSCF02", \
"SSCF03", \
"SSCOP01", \
"SSCOP02", \
"SSCOP03", \
"CPCS01", \
"CPCS02", \
"CPCS03", \
"A41MAP01", \
"SCTP", \
"M3UA", \
"ETSIMAP2", \
"FEIF", \
"CP_MANAGER", \
"OAM", \
"PCF", \
"MGMT_FE", \
"DIST", \
"LICENSE",\
"SCTP_IF_ID", \
"MGMT_SCTP_ID"

#define TIMER_ID              200
#define ANY_ID                201
/* Used internally to indicate "No user" */
enum
{
	EINSS7CP_NO_ID = 202
};


/* General ERROR-OFFSETS    */

#define MTPL2_ERROR_OFFSET       10000
#define MTPL3_ERROR_OFFSET       11000
#define SCCP_ERROR_OFFSET        12000
#define TUP_ERROR_OFFSET         13000
#define ISUP_ERROR_OFFSET        14000
#define TCAP_ERROR_OFFSET        15000
#define INAP_ERROR_OFFSET        16000
#define MAP_ERROR_OFFSET         17000
#define IS41_ERROR_OFFSET        18000
#define A41MAP_ERROR_OFFSET      18000
#define MGMT_ERROR_OFFSET        19000
#define XMGMT_ERROR_OFFSET       20000
#define CALLC_ERROR_OFFSET       21000
#define VAP_ERROR_OFFSET         22000
#define SV_ERROR_OFFSET          23000
#define SM_ERROR_OFFSET          24000
#define BE_ERROR_OFFSET          25000
#define FE_ERROR_OFFSET          26000
#define ETINAP_ERROR_OFFSET      27000
#define ETSIMAP_ERROR_OFFSET     28000
#define SS7IP_ERROR_OFFSET       29000
#define SNMPAGENT_ERROR_OFFSET   30000
#define ISRL2_ERROR_OFFSET       31000
#define LIC_ERROR_OFFSET         32000
#define SAALIF_ERROR_OFFSET      33000
#define PPEL2_ERROR_OFFSET       34000
#define EQSA_ERROR_OFFSET        35000
#define BSSAPLE_ERROR_OFFSET     36000
#define SCTP_ERROR_OFFSET        37000
#define M3UA_ERROR_OFFSET        38000
#define OAM_ERROR_OFFSET         39000
#define DISTRIBUTOR_ERROR_OFFSET 40000
#define FEHD_ERROR_OFFSET	 41000
#define PCF_ERROR_OFFSET	 42000
#define MTPL2IFISR_ERROR_OFFSET  43000
#define BEHD_ERROR_OFFSET	 44000
#define LICENSE_ERROR_OFFSET     45000
#define SCTP_IF_ERROR_OFFSET     46000
#define ISRL2_H110_ERROR_OFFSET  47000


/*********************************************************************/
/*                                                                   */
/*                           M A C R O S                             */
/*                                                                   */
/*********************************************************************/
#ifndef MAX_NUMBER_OF_INSTANCES
#define MAX_NUMBER_OF_INSTANCES 10
#endif /* MAX_NUMBER_OF_INSTANCES */

#if !defined( EINSS7_WINNT) && !defined(EINSS7_TELORB)
  #define makeVTAG static const char *versionTAG = EINVERSTRING
#else
  #ifndef EINVERSTRING
    #ifdef EINSS7_WINNT
      #define makeVTAG static const char *versionTAG = "WINDOWS NT VERSION"
    #else
      #define makeVTAG static const char *versionTAG = "TelORB VERSION"
    #endif
  #else
    #define makeVTAG static const char *versionTAG = EINVERSTRING
  #endif /* EINVERSTRING */
#endif /* EINSS7_WINNT */

#endif /* __PORTSS7_H__ */
