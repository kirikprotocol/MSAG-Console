/*********************************************************************/
/* Don't remove these two lines, contain depends on them!            */
/* Document Number: %Container% Revision: %Revision% */
/*                                                                   */
/* et94inapcs1defapi.h,v */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* COPYRIGHT Ericsson Infocom AB 1996                                */
/*                                                                   */
/* The copyright to the computer program herein is the property of   */
/* Ericsson Infocom AB. The program may be used and/or               */
/* copied only with the written permission from Ericsson Infocom     */
/* AB or in the accordance with the terms and conditions             */
/* stipulated in the agreement/contract under which the program has  */
/* been supplied.                                                    */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Document Number:                 (Rev:    )                       */
/* <n>/190 55-<product number> Ux   (<revision>)  */
/*                                                                   */
/* Revision:                                                         */
/* @EINVER: et94inapcs1defapi.h,v 1.7 2000/01/27 15:30:56 Exp */
/*                                                                   */
/* Programmer:                                                       */
/* EIN/N/B Andreas Wilde                                             */
/*                                                                   */
/* Purpose:                                                          */ 
/* Detinitions for et94inapapi.c for CS1 functionality.              */
/*                                                                   */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Revision record:                                                  */
/*                                                                   */
/* 00-96XXXX  EIN/N/P Andreas Wilde                                  */
/*                    Initial issue                                  */
/*                                                                   */
/*-------------------------------------------------------------------*/
/* Notes:                                                            */
/*                                                                   */
/*********************************************************************/

/*********************************************************************/
/*                                                                   */
/*            P A R A M E T E R   C O N S T A N T S                  */
/*                                                                   */
/*********************************************************************/
#ifndef __INAPCS1API_H__
#define __INAPCS1API_H__
#if defined __cplusplus || defined c_plusplus
extern "C" {
#endif

/* operation codes */

#define IN_OP_INITIAL_DP                0
#define IN_OP_EXECUTE                   10
#define IN_OP_ASSIST_REQ_INSTR          16
#define IN_OP_ESTABLISH_TEMP_CONN       17
#define IN_OP_DISCONNECT_FORW_CONN      18
#define IN_OP_CONN_2_RESOURCE           19
#define IN_OP_CONNECT                   20
#define IN_OP_RELEASE_CALL              22
#define IN_OP_REQ_REP_BCSM_EVENT        23
#define IN_OP_EVENT_REP_BCSM            24
#define IN_OP_REQ_NOTIF_CHRG_EVENT      25
#define IN_OP_EVENT_NOTIF_CHRG          26
#define IN_OP_COLLECT_INFO              27
#define IN_OP_CONTINUE                  31
#define IN_OP_INIT_CALL_ATTEMPT         32
#define IN_OP_RESET_TIMER               33
#define IN_OP_FURNISH_CHRG_INFO         34
#define IN_OP_APPLY_CHRG                35
#define IN_OP_APPLY_CHRG_REP            36
#define IN_OP_CALL_GAP                  41
#define IN_OP_ACT_SRVC_FLTR             42
#define IN_OP_SRVC_FLTR_RESP            43
#define IN_OP_CALL_INFO_REP             44
#define IN_OP_CALL_INFO_REQUEST         45
#define IN_OP_SEND_CHRG_INFO            46
#define IN_OP_PLAY_ANNOUNCE             47
#define IN_OP_PRMT_CLLCT_U_INFO         48
#define IN_OP_SPEC_RSRC_REP             49
#define IN_OP_CANCEL                    53
#define IN_OP_ACT_TEST                  55

/* error codes */

#define IN_ERRC_CANCELLED                       0
#define IN_ERRC_CANCELFAILED                    1
#define IN_ERRC_ATTRIBUTEERROR                  1
#define IN_ERRC_NAMEERROR                       2
#define IN_ERRC_ETCFAILED                       3
#define IN_ERRC_IMPROPERCALLERRESPONSE          4
#define IN_ERRC_MISSINGCUSTOMERRECORD           6
#define IN_ERRC_MISSINGPARAMETER                7
#define IN_ERRC_PARAMETEROUTOFRANGE             8
#define IN_ERRC_REQUESTEDINFOERROR              10
#define IN_ERRC_EXECUTION                       10
#define IN_ERRC_SYSTEMFAILURE                   11
#define IN_ERRC_TASKREFUSED                     12
#define IN_ERRC_UNAVAILABLERESOURCE             13
#define IN_ERRC_UNEXPECTEDCOMPONENTSEQUENCE     14
#define IN_ERRC_UNEXPECTEDDATAVALUE             15
#define IN_ERRC_UNEXPECTEDPARAMETER             16
#define IN_ERRC_UNKNOWNLEGID                    17

/* internal number for error codes */
#define IN_INTRNL_ERRC_ATTRIBUTEERROR           400
#define IN_INTRNL_ERRC_NAMEERROR                401
#define IN_INTRNL_ERRC_EXECUTION                402

/* length constants */

#define IN_RES_RECEIVEDINFORMATION              0
#define MAX_ABINFO_LEN                          255
#define MAX_ACNAME_LEN                          25
#define MAX_ADDRESS_LEN                         255
#define MAX_PARAM_LEN                           3000 /* HeUl TR1895 2000-01-27 */
#define MAXACHBILLINGCHARGINGLENGTH             6
#define ALERT_PTRN_LEN                          3
#define MAX_ANY_LEN                             255
#define MAXBEARERCAPABILITYLENGTH               11
#define MAXCALLEDPARTYNUMBERLENGTH              16
#define MAXCALLINGPARTYNUMBERLENGTH             16
#define CGPASCAT_LEN                            1
#define MAXCALLRESULTLENGTH                     197
#define MAXCAUSELENGTH                          2
#define CD_DIGIT_LEN                            2
#define NUMOFCOUNTERS                           100
#define DATEANDTIMELEN                          6
#define MAXDIGITSLEN                            15
#define MAXDISPLAYINFORMATIONLENGTH             127
#define MAXEVENTTYPECHARGINGLENGTH              20
#define MAXEVENTTYPECHARGINTLENGTH              255
#define MAXFCIBILLINGCHARGINGLENGTH             794
#define FORWARDCALLINDICATORSLEN                2
#define HIGHLAYERCOMPATIBILITYLENGTH            2
#define MAXIPAVAILABLELENGTH                    20
#define MAXIPSSPCAPABILITIESLEGNTH              16
#define MAXLOCATIONNUMBERLENGTH                 16
#define LEGTYPE_T_LEN                           1
#define MAXEVENTSPECIFICINFORMATIONCHARGINGLENGTH 20
#define MAXORIGINALCALLEDPARTYIDLENGTH          20
#define MAXREDIRECTINGPARTYIDLENGTH             20
#define REDRCTINFO_LEN                          2
#define ROUTELIST_LEN                           3
#define MAXROUTELISTLENGTH                      1
#define MAXSCFIDLENGTH                          20
#define MAXSCIBILLINGCHARGINGLENGTH             30
#define MAXSERVICEINTERACTIONINDICATORSLENGTH   20
#define MAXSFBILLINGCHARGINGLENGTH              30
#define MAX_UINFO_LEN                           300
#define MAXMESSAGECONTENTLENGTH                 20
#define MAXATTRIBUTESLENGTH                     1
#define NUMOFMESSAGEIDS                         32
#define NUMOFINFOITEMS                          5       
#define NUMOFEXTENSIONS                         16
#define VAR_P_TIME_LEN                          2
#define VAR_P_DATE_LEN                          3
#define VAR_P_PRICE_LEN                         4
#define NUMOFCHARGINGEVENTS                     3
#define NUMOFBCSMEVENTS                         24
#define MAX_ABINFO_LEN                          255
#define MAX_ADDRESS_LEN                         255
#define MSGID_VARPART_LEN                       255     
#define FORWARDCALLINDICATORS_LEN               2
#define REDIRECTINFO_LEN                        2
#define DATEANDTIME_LEN                         6
#define ATTRCHARLEN                             1
#define MAXATTRIBUTESTRINGLEN                   37

#define MAXATTRTYPEVALUE                        5
#define MAXRELDISTNAME                          5
#define MAXATTRVALUE                            5
#define MAX_NBR_OF_PROBLEMS                     5

#define UNDEFINEDATTRIBUTETYPE                  3
#define MISSINGINPUTVALUES                      1
#define EXECUTIONFAILURE                        2
#define NOSUCHOBJECT                            1

#define DEFAULT_RELEASECALLARG                  31

#define IN_ER_STDERRORANDINFO                   0

/* problem code tags */
#define IN_E_PROB_TAG_GENERAL                   0x80
#define IN_E_PROB_TAG_INVOKE                    0x81
#define IN_E_PROB_TAG_RESULT                    0x82
#define IN_E_PROB_TAG_ERROR                     0x83

/* problem codes */
#define IN_E_PR_G_UNREC_COMP                    0x00
#define IN_E_PR_G_MISTYPED_COMP                 0x01
#define IN_E_PR_G_BAD_COMP                      0x02
#define IN_E_PR_I_DUP_ID                        0x00
#define IN_E_PR_I_UNREC_OP                      0x01
#define IN_E_PR_I_MIST_PARM                     0x02
#define IN_E_PR_I_RES_LIMIT                     0x03
#define IN_E_PR_I_INIT_RELEASE                  0x04
#define IN_E_PR_I_UNREC_L_ID                    0x05
#define IN_E_PR_I_RESP_UNEXP                    0x06
#define IN_E_PR_I_UNEXP_OP                      0x07
#define IN_E_PR_R_UNREC_I_ID                    0x00
#define IN_E_PR_R_RES_UNEXP                     0x01
#define IN_E_PR_R_MIST_PARM                     0x02
#define IN_E_PR_E_UNREC_I_ID                    0x00
#define IN_E_PR_E_RET_ERR_UNEXP                 0x01
#define IN_E_PR_E_UNREC_ERR                     0x02
#define IN_E_PR_E_UNEXP_ERR                     0x03
#define IN_E_PR_E_MIST_PARM                     0x04

/* pAbortCause */
#define IN_E_ABORT_UNREC_MESS                   0x00
#define IN_E_ABORT_UNREC_ID                     0x01
#define IN_E_ABORT_BAD_TP                       0x02
#define IN_E_ABORT_INCORRECT_TP                 0x03
#define IN_E_ABORT_RESOURCE_LIMIT               0x04
#define IN_E_ABORT_ABN_DIALOGUE                 0x05

/* priority */
#define E94INPRI_LOLO                           0x00
#define E94INPRI_LOHI                           0x01
#define E94INPRI_HILO                           0x02
#define E94INPRI_HIHI                           0x03

/* reportCause */
#define IN_E_NO_TRANS_FOR_SUCH_NATURE           0x00
#define IN_E_NO_TRANS_FOR_SPEC_ADDRESS          0x01
#define IN_E_SUBSYSTEM_CONGESTION               0x02
#define IN_E_SUBSYSTEM_FAILURE                  0x03
#define IN_E_UNEQUIPPED_USER                    0x04
#define IN_E_NETWORK_FAILURE                    0x05
#define IN_E_NETWORK_CONGESTION                 0x06
#define IN_E_UNQUALIFIED                        0x07
#define IN_E_HOP_CNT_VIOLATION                  0x08

/* returnIndicator */
#define IN_E_NO_DIALOGUE                        0x00
#define IN_E_DIALOGUE                           0x01

/* termination */
#define IN_E_BASIC_TERM                         0x00
#define IN_E_PREARR_TERM                        0x01

/* userStatus */
#define IN_E_USTAT_AVAILABLE                    0x00
#define IN_E_USTAT_UNAVAILABLE                  0x01

/* constants for Et94InapGetEndOffs */
#define INDEFINITE_LENGTH               0x80
#define TAGTYPE_MASK                    0x20
#define PRIMITIVE                       0x00
#define CONSTRUCTED                     0x20

 
#define IN_INVOKE                               0
#define IN_LCANCEL                              1
#define IN_LREJECT                              2
#define IN_RESULTL                              3
#define IN_RREJECT                              5
#define IN_UCANCEL                              6
#define IN_UERROR                               7
#define IN_UREJECT                              8

#define IN_E_SAC_UNKNOWN                        255
#define IN_E_SAC_CS1_SSP_2_SCP                          0
#define IN_E_SAC_CS1_ASSIST_HOFF_SSP_2_SCP              1
#define IN_E_SAC_CS1_IP_2_SCP                           2
#define IN_E_SAC_CS1_SCP_2_SSP                          3
#define IN_E_SAC_CS1_SCP_2_SSP_TFCMGMT                  4
#define IN_E_SAC_CS1_SCP_2_SSP_SRVCMGMT                 5
#define IN_E_SAC_CS1_SSP_2_SCP_SRVCMGMT                 6
#define IN_E_SAC_CS2_EXT_DIR_ACCESS                    38

/* Attribute type values */
#define ID_OI_SERVICEKEY        3
#define ID_OI_MSISDN           13
#define ID_OI_ACCOUNTNUM       14
#define ID_OI_ACCOUNTLEFT      15
#define ID_OI_ACTIVEDAYS       16
#define ID_OI_PINNUMBER        17

/* Method Id values */
#define ID_MT_PPS_SUPPLYRETRIEVE      1
#define ID_MT_PPS_SUPPLYMODIFY        2
#define ID_MT_PPS_SUPPLYCLAIMMISSING  3

typedef struct
{
        UCHAR_T         dummy;
} CONTINUEARG_T;


typedef struct
{
        UCHAR_T         invokeId;
        BOOLEAN_T       linkedIdUsed;
        UCHAR_T         linkedId;
} INVOKE_T;

typedef struct
{
        UCHAR_T invokeId;
} LCANCEL_T;

typedef struct
{
        BOOLEAN_T       invokeIdUsed;
        UCHAR_T         invokeId;
        UCHAR_T         problemCodeTag;
        UCHAR_T         problemCode;
} LREJECT_T;

typedef struct
{
        UCHAR_T invokeId;
} RESULTL_T;

typedef struct
{
        BOOLEAN_T       invokeIdUsed;
        UCHAR_T         invokeId;
        UCHAR_T         problemCodeTag;
        UCHAR_T         problemCode;
} RREJECT_T;

typedef struct
{
        UCHAR_T addressLen;
        UCHAR_T address[MAX_ADDRESS_LEN];
} SS7_ADDRESS_T;

typedef struct
{
        UCHAR_T invokeId;
} UCANCEL_T;

typedef struct
{
        UCHAR_T invokeId;
} UERROR_T;

typedef struct
{
        BOOLEAN_T       invokeIdUsed;
        UCHAR_T         invokeId;
        UCHAR_T         problemCodeTag;
        UCHAR_T         problemCode;
} UREJECT_T;

/*********************************************************************/
/*                                                                   */
/*                   COMMON DATATYPES FOR ASN.1                      */
/*                                                                   */
/*********************************************************************/

typedef struct
{
        USHORT_T        nbrOfElements;
        UCHAR_T         acbcc[MAXACHBILLINGCHARGINGLENGTH];
} ACHBILLINGCHARGINGCHARACTERISTICS_T;

typedef UCHAR_T ALERTINGPATTERN_T[ALERT_PTRN_LEN];

typedef struct
{
        UCHAR_T         buf[MAX_ANY_LEN];
} ANY_T;

typedef USHORT_T APPLICATIONTIMER_T;

typedef struct
{
        UCHAR_T         chooser;        /* should be zero */
        union
        {
                struct
                {
                USHORT_T        nbrOfElements;
                UCHAR_T         bCap[MAXBEARERCAPABILITYLENGTH];
                } bCap;
        } bc;
} BEARERCAPABILITY_T;

typedef struct
{
        USHORT_T        nbrOfElements;
        UCHAR_T         cdpn[MAXCALLEDPARTYNUMBERLENGTH];
} CALLEDPARTYNUMBER_T;

typedef struct
{
        USHORT_T        nbrOfElements;
        UCHAR_T         cgpn[MAXCALLINGPARTYNUMBERLENGTH];
} CALLINGPARTYNUMBER_T;

typedef UCHAR_T CALLINGPARTYSCATEGORY_T[CGPASCAT_LEN];

typedef struct
{
        USHORT_T        nbrOfElements;
        UCHAR_T         cRes[MAXCALLRESULTLENGTH];
} CALLRESULT_T;

typedef struct
{
        USHORT_T        nbrOfElements;
        UCHAR_T         cause[MAXCAUSELENGTH];
} CAUSE_T;

typedef enum
{
        IN_E_MANUALCGENCOUNTERED = 1,
        IN_E_SCPOVERLOAD
} CGENCOUNTERED_T;

typedef enum
{
        IN_E_SCPOVERLOADED = 0,
        IN_E_MANUALLYINITIATED
} CONTROLTYPE_T;

typedef UCHAR_T COUNTERID_T;

typedef enum
{
        IN_E_STDERRORANDINFO = 0,
        IN_E_HELP,
        IN_E_REPEATPROMPT
} ERRORTREATMENT_T;

typedef struct
{
        USHORT_T        nbrOfElements;
        UCHAR_T         esic[MAXEVENTSPECIFICINFORMATIONCHARGINGLENGTH];
} EVENTSPECIFICINFORMATIONCHARGING_T;

typedef struct
{
        USHORT_T        nbrOfElements;
        UCHAR_T         etc[MAXEVENTTYPECHARGINGLENGTH];
} EVENTTYPECHARGING_T;

typedef UCHAR_T LEGTYPE_T[LEGTYPE_T_LEN];

typedef struct
{
        UCHAR_T         chooser; 
                
        union
        {
                LEGTYPE_T               sendingSideID;
                LEGTYPE_T               receivingSideID;
        } u;
} LEGID_T;

typedef enum
{
        IN_E_INTERRUPTED = 0,
        IN_E_NOTIFYANDCONTINUE,
        IN_E_TRANSPARENT
} MONITORMODE_T;

typedef enum
{
        IN_E_ORIGATTEMPTAUTHORIZED = 1,
        IN_E_COLLECTEDINFO,
        IN_E_ANALYZEDINFORMATION,
        IN_E_ROUTESELECTFAILURE,
        IN_E_OCALLEDPARTYBUSY,
        IN_E_ONOANSWER,
        IN_E_OANSWER,
        IN_E_OMIDCALL,
        IN_E_ODISCONNECT,
        IN_E_OABANDON,
        IN_E_TERMATTEMPTAUTHORIZED = 12,
        IN_E_TCALLEDPARTYBUSY,
        IN_E_TNOANSWER,
        IN_E_TANSWER,
        IN_E_TMIDCALL,
        IN_E_TDISCONNECT,
        IN_E_TABANDON
} EVENTTYPEBCSM_T;

typedef struct
{
        UCHAR_T         nbrOfElements;
        UCHAR_T         dig[MAXDIGITSLEN];
} DIGITS_T;

typedef UCHAR_T CUTANDPASTE_T;

typedef UCHAR_T DATEANDTIME_T[DATEANDTIMELEN];

typedef LONG_T DURATION_T;

typedef enum
{
        IN_E_IGNORE,
        IN_E_ABORT
} CRITICALITY_T;

typedef struct
{
        LONG_T  type;
        CRITICALITY_T criticality;
        ANY_T   value;
} EXTENSIONFIELD_T;


typedef struct
{
        USHORT_T        nbrOfElements;
        UCHAR_T         fbcc[MAXFCIBILLINGCHARGINGLENGTH];
} FCIBILLINGCHARGINGCHARACTERISTICS_T;

typedef FCIBILLINGCHARGINGCHARACTERISTICS_T FURNISHCHARGINGINFORMATIONARG_T;

typedef UCHAR_T FORWARDCALLINDICATORS_T[FORWARDCALLINDICATORSLEN];

typedef UCHAR_T HIGHLAYERCOMPATIBILITY_T[HIGHLAYERCOMPATIBILITYLENGTH];

typedef TEXT_T IA5STRING_T;

typedef ULONG_T INTEGER4_T;

typedef INTEGER4_T TIMERVALUE_T;

typedef INTEGER4_T SERVICEKEY_T;

typedef UCHAR_T NUMBEROFDIGITS_T;

typedef struct
{
        INTEGER4_T              toneID;
        BOOLEAN_T               durationUsed;
        INTEGER4_T              duration;
} TONE_T;




typedef struct
{
        UCHAR_T         nbrOfElements;
        IA5STRING_T     di[MAXDISPLAYINFORMATIONLENGTH];
} DISPLAYINFORMATION_T;

typedef struct
{
        USHORT_T        nbrOfElements;
        UCHAR_T         ocdpid[MAXORIGINALCALLEDPARTYIDLENGTH];
} ORIGINALCALLEDPARTYID_T;

typedef struct
{
        USHORT_T        nbrOfElements;
        UCHAR_T         rdgpid[MAXREDIRECTINGPARTYIDLENGTH];
} REDIRECTINGPARTYID_T;

typedef UCHAR_T REDIRECTIONINFORMATION_T[REDRCTINFO_LEN];


typedef LONG_T INTERVAL_T;

typedef struct
{
        USHORT_T        nbrOfElements;
        UCHAR_T         isCap[MAXIPAVAILABLELENGTH];
} IPAVAILABLE_T;

typedef struct
{
        USHORT_T        nbrOfElements;
        UCHAR_T         ipSspCap[MAXIPSSPCAPABILITIESLEGNTH];
} IPSSPCAPABILITIES_T;

typedef struct
{
        USHORT_T        nbrOfElements;
        UCHAR_T         lNbr[MAXLOCATIONNUMBERLENGTH];
} LOCATIONNUMBER_T;

typedef UCHAR_T MAXIMUMNUMBEROFCOUNTERS_T;

typedef enum
{
        IN_E_REQUEST = 0,
        IN_E_NOTIFICATION
} MESSAGETYPE_T;

typedef struct
{
        MESSAGETYPE_T messageType;
} MISCCALLINFO_T;

typedef enum
{
        IN_E_CALLATTEMPTELAPSEDTIME = 0,
        IN_E_CALLSTOPTIME,
        IN_E_CALLCONNECTEDELAPSEDTIME,
        IN_E_CALLEDADDRESS,
        IN_E_RELEASECAUSE = 30
} REQUESTEDINFORMATIONTYPE_T;

typedef struct
{
        USHORT_T                nbrOfElements;
        USHORT_T                lenOfElements[ROUTELIST_LEN];
        UCHAR_T                 rl[ROUTELIST_LEN][MAXROUTELISTLENGTH];
} ROUTELIST_T;

typedef struct
{
        USHORT_T                nbrOfElements;
        UCHAR_T                 sid[MAXSCFIDLENGTH];
} SCFID_T;

typedef struct
{
        USHORT_T                nbrOfElements;
        UCHAR_T                 sbcc[MAXSCIBILLINGCHARGINGLENGTH];
} SCIBILLINGCHARGINGCHARACTERISTICS_T;

typedef struct
{
        USHORT_T                nbrOfElements;
        UCHAR_T                 sii[MAXSERVICEINTERACTIONINDICATORSLENGTH];
} SERVICEINTERACTIONINDICATORS_T;

typedef struct
{
        USHORT_T                nbrOfElements;
        UCHAR_T                 sbcc[MAXSFBILLINGCHARGINGLENGTH];
} SFBILLINGCHARGINGCHARACTERISTICS_T;

typedef enum
{
        IN_E_TSSF = 0
} TIMERID_T;


typedef DIGITS_T ASSISTINGSSPIPROUTINGADDRESS_T;

typedef DIGITS_T ADDITIONALCALLINGPARTYNUMBER_T;

typedef enum
{
        IN_E_UNAVAILABLERESOURCES = 0,
        IN_E_COMPONENTFAILURE,
        IN_E_BASICCALLPROCESSINGEXCEPTION,
        IN_E_RESOURCESTATUSFAILURE,
        IN_E_ENDUSERFAILURE
} UNAVAILABLENETWORKRESOURCE_T;

typedef struct
{
        EVENTTYPECHARGING_T     eventTypeCharging;
        MONITORMODE_T           monitorMode;
        BOOLEAN_T               legIDUsed;
        LEGID_T                 legID;
} CHARGINGEVENT_T;

typedef struct
{
        UCHAR_T         minimumNbOfDigits;
        UCHAR_T         maximumNbOfDigits;
        UCHAR_T         nbrOfEndOfReplyDigit;
        UCHAR_T         endOfReplyDigit[CD_DIGIT_LEN];
        UCHAR_T         nbrOfCancelDigit;
        UCHAR_T         cancelDigit[CD_DIGIT_LEN];
        UCHAR_T         nbrOfStartDigit;
        UCHAR_T         startDigit[CD_DIGIT_LEN];
        BOOLEAN_T       firstDigitTimeOutUsed;
        UCHAR_T         firstDigitTimeOut;
        BOOLEAN_T       interDigitTimeOutUsed;
        UCHAR_T         interDigitTimeOut;
        ERRORTREATMENT_T errortreatment;
        BOOLEAN_T       interruptableAnnInd;
        BOOLEAN_T       voiceInformation;
        BOOLEAN_T       voiceBack;
} COLLECTEDDIGITS_T;

typedef struct
{
        UCHAR_T         chooser; /* should be 0 */
        union
        {
                COLLECTEDDIGITS_T       collectedDigits;
        } u;
} COLLECTEDINFO_T;


typedef DIGITS_T CORRELATIONID_T;

typedef struct
{
        UCHAR_T                 chooser;        /* 0 = integer, 1 = number,
                                2 = time, 3 = date, 4 = price */
        union
        {
        INTEGER4_T              integer;
        DIGITS_T                number;
        UCHAR_T                 time[VAR_P_TIME_LEN];
        UCHAR_T                 date[VAR_P_DATE_LEN];
        UCHAR_T                 price[VAR_P_PRICE_LEN];
        } u;
} VARIABLEPART_T;

typedef struct
{
        UCHAR_T         chooser;        /* 0 = elementaryMessageID
                                1 = text
                                29 = elementaryMessageIDs
                                30 = variableMessage */
        union
        {
        INTEGER4_T              elementaryMessageID;
        struct
        {
                USHORT_T        nbrOfMsgCnt;
                IA5STRING_T     messageContent[MAXMESSAGECONTENTLENGTH];
                USHORT_T        nbrOfAttr;
                UCHAR_T         attributes[MAXATTRIBUTESLENGTH];
        } text;
        struct
        {
                USHORT_T        nbrOfElmMsgId;
                INTEGER4_T      elementaryMessageIDs[NUMOFMESSAGEIDS];
        } elementaryMessageIDs;

        struct
        {
                INTEGER4_T              elementaryMessageID;
                USHORT_T                nbrOfElements;
                VARIABLEPART_T          variableParts[MSGID_VARPART_LEN];
        } variableMessage;

        } u;
} MESSAGEID_T;

typedef struct
{
        MESSAGEID_T             messageID;
        BOOLEAN_T               numberOfRepetitionsUsed;
        UCHAR_T                 numberOfRepetitions;
        BOOLEAN_T               durationUsed;
        USHORT_T                duration;
        BOOLEAN_T               intervalUsed;
        USHORT_T                interval;
} INBANDINFO_T;

typedef struct
{
        UCHAR_T         chooser;        /* 0 = inbandinfo, 1 = tone,
                                2 =displayInformation */
        union
        {
                INBANDINFO_T            inbandinfo;
                TONE_T                  tone;
                DISPLAYINFORMATION_T    displayInformation;
        } u;
} INFORMATIONTOSEND_T;

typedef struct
{
        COUNTERID_T     counterID;
        INTEGER4_T      counterValue;
} COUNTERANDVALUE_T;


typedef struct
{
        USHORT_T                nbrOfElements;
        COUNTERANDVALUE_T       cv[NUMOFCOUNTERS];
} COUNTERSVALUE_T;


typedef struct
{
        CALLEDPARTYNUMBER_T     calledPartyNumber;
} DESTINATIONROUTINGADDRESS_T;




typedef struct
{
        UCHAR_T         chooser;        /* 0 = numberOfDIGITS_T,
                                1 = applicationTimer */
        union
        {
                NUMBEROFDIGITS_T        numberOfDigits;
                APPLICATIONTIMER_T      applicationTimer;
        } u;
} DPSPECIFICCRITERIA_T;

typedef struct
{
        EVENTTYPEBCSM_T         eventTypeBCSM;
        MONITORMODE_T           monitorMode;
        BOOLEAN_T               legIDUsed;
        LEGID_T                 legID;
        BOOLEAN_T               dPSpecificCriteriaUsed;
        DPSPECIFICCRITERIA_T    dPSpecificCriteria;
} BCSMEVENT_T;





typedef struct
{
        UCHAR_T         chooser; /* same value as chosen tag */
        union
        {
                struct
                {
                CALLEDPARTYNUMBER_T     calledPartyNumber;
                } collectedInfoSpecificInfo;

                struct
                {
                CALLEDPARTYNUMBER_T     calledPartyNumber;
                } analyzedInfoSpecificInfo;

                struct
                {
                BOOLEAN_T               failureCauseUsed;
                CAUSE_T                 failureCause;
                } routeSelectFailureSpecificInfo;

                struct
                {
                BOOLEAN_T               failureCauseUsed;
                CAUSE_T                 failureCause;
                } oCalledPartyBusySpecificInfo;

                struct
                {
                UCHAR_T                 dummy;
                } oNoAnswerSpecificInfo;

                struct
                {
                UCHAR_T                 dummy;
                } oAnswerSpecificInfo;

                struct
                {
                UCHAR_T                 dummy;
                } oMidCallSpecificInfo;

                struct
                {
                BOOLEAN_T               releaseCauseUsed;
                CAUSE_T                 releaseCause;
                } oDisconnectSpecificInfo;

                struct
                {
                BOOLEAN_T               busyCauseUsed;
                CAUSE_T                 busyCause;
                } tCalledPartyBusySpecificInfo;

                struct
                {
                UCHAR_T                 dummy;
                } tNoAnswerSpecificInfo;

                struct
                {
                UCHAR_T                 dummy;
                } tAnswerSpecificInfo;

                struct
                {
                UCHAR_T                 dummy;
                } tMidCallSpecificInfo;

                struct
                {
                BOOLEAN_T               releaseCauseUsed;
                CAUSE_T                 releaseCause;
                } tDisconnectSpecificInfo;
        } u;
} EVENTSPECIFICINFORMATIONBCSM_T;


typedef struct
{
        SFBILLINGCHARGINGCHARACTERISTICS_T sFBillingChargingCharacteristics;
        BOOLEAN_T                       informationToSendUsed;
        INFORMATIONTOSEND_T             informationToSend;
        BOOLEAN_T                       maximumNumberOfCountersUsed;
        MAXIMUMNUMBEROFCOUNTERS_T       maximumNumberOfCounters;
        BOOLEAN_T                       releaseCauseUsed;
        CAUSE_T                         releaseCause;
} FILTEREDCALLTREATMENT_T;

typedef struct
{
        UCHAR_T         chooser; /* 0 = interval, 1 = numberOfCalls */
        union
        {
                LONG_T          interval;
                INTEGER4_T      numberOfCalls;
        } u;
} FILTERINGCHARACTERISTICS_T;

typedef struct
{
        UCHAR_T         chooser;        /* 2 = SERVICEKEY_T,
                                30 = addressAndService */
        union
        {
                SERVICEKEY_T            serviceKey;
                struct
                {
                DIGITS_T                calledAddressValue;
                SERVICEKEY_T            serviceKey;
                BOOLEAN_T               callingAddressValueUsed;
                DIGITS_T                callingAddressValue;
                BOOLEAN_T               locationNumberUsed;
                LOCATIONNUMBER_T        locationNumber;
                }       addressAndService;
        } u;
} FILTERINGCRITERIA_T;

typedef struct
{
        UCHAR_T         chooser; /* 0 = duration, 1 = stopTime */
        union
        {
                DURATION_T              duration;
                DATEANDTIME_T           stopTime;
        } u;
} FILTERINGTIMEOUT_T;

typedef struct
{
        SERVICEKEY_T            serviceKey;
} GAPONSERVICE_T;

typedef struct
{
        DURATION_T              duration;
        INTERVAL_T              gapInterval;
} GAPINDICATORS_T;

typedef struct
{
        UCHAR_T                 chooser;        /* 0 = informationToSend,
                                1 = releaseCause, 2 = both */
        union
        {
        INFORMATIONTOSEND_T     informationToSend;
        CAUSE_T                 releaseCause;
        struct
        {
                INFORMATIONTOSEND_T     informationToSend;
                CAUSE_T                 releaseCause;
        } both;
        } u;
} GAPTREATMENT_T;

typedef struct
{
        UCHAR_T chooser;        /* 0 = calledAddressValue, 2 = gapOnService,
                        29 = calledAddressAndService,
                        30 = callingAddressAndService */
        union
        {
                DIGITS_T                calledAddressValue;
                GAPONSERVICE_T          gapOnService;
                struct
                {
                        DIGITS_T                calledAddressValue;
                        SERVICEKEY_T            serviceKey;

                } calledAddressAndService;

                struct
                {
                        DIGITS_T                callingAddressValue;
                        SERVICEKEY_T            serviceKey;
                        BOOLEAN_T               locationNumberUsed;
                        LOCATIONNUMBER_T        locationNumber;
                } callingAddressAndService;
        } u;
} GAPCRITERIA_T;

typedef struct
{
        UCHAR_T         chooser;        /* 0 = callAttemptElapsed-
                                TimeValue,
                                1 = callStopTimeValue ,
                                2 = callConnectedElapsed-
                                TimeValue,
                                3 = calledAddressValue,
                                30 = releaseCauseValue */
        union
        {
        UCHAR_T                 callAttemptElapsedTimeValue;
        DATEANDTIME_T           callStopTimeValue;
        INTEGER4_T              callConnectedElapsedTimeValue;
        DIGITS_T                calledAddressValue;
        CAUSE_T                 releaseCauseValue;
        } u;
} REQUESTEDINFORMATIONVALUE_T;

typedef CHAR_T INVOKEIDTYPE_T;

typedef INVOKEIDTYPE_T INVOKEID_T;


typedef CALLEDPARTYNUMBER_T IPROUTINGADDRESS_T;

typedef struct
{
        REQUESTEDINFORMATIONTYPE_T      requestedInformationType;
        REQUESTEDINFORMATIONVALUE_T     requestedInformationValue;
} REQUESTEDINFORMATION_T;

typedef struct
{
        USHORT_T                nbrOfElements;
        REQUESTEDINFORMATION_T  rinfo[NUMOFINFOITEMS];
} REQUESTEDINFORMATIONLIST_T;

/*typedef REQUESTEDINFORMATION_T REQUESTEDINFORMATIONLIST_T[NUMOFINFOITEMS];*/

typedef struct
{
        USHORT_T                        nbrOfElements;
        REQUESTEDINFORMATIONTYPE_T      rinfo[NUMOFINFOITEMS];
} REQUESTEDINFORMATIONTYPELIST_T;

typedef UCHAR_T ATTRIBUTETYPE_T;

typedef struct
{
    UCHAR_T         chooser; /* 0 = intElement,
                                    1 = int4Element,
                                    2 = charElement,
                                    3 = stringElement */
    union
    {
        USHORT_T       intElement;
        INTEGER4_T     int4Element;
        UCHAR_T        charElement[ATTRCHARLEN];
        struct
        {
            USHORT_T      nbrOfElements;
            UCHAR_T       stringElement[MAXATTRIBUTESTRINGLEN];
        }stringElement;
    }u;
} ATTRIBUTEVALUE_T;

typedef struct 
{
    ATTRIBUTETYPE_T            attributeType;
    ATTRIBUTEVALUE_T           attributeValue;
} ATTRIBUTETYPEANDVALUE_T;

typedef struct 
{
    USHORT_T                   nbrOfElements;
    ATTRIBUTETYPEANDVALUE_T    attribute[MAXATTRTYPEVALUE];
} INPUTASSERTIONS_T;

typedef UCHAR_T METHODID_T;

typedef ATTRIBUTETYPEANDVALUE_T RELDISTNAME_T;

typedef struct
{
    USHORT_T                   nbrOfElements;
    RELDISTNAME_T              relativeDistinguishedName[MAXRELDISTNAME];
} RDNSEQUENCE_T;

typedef struct
{
    USHORT_T                     chooser; /* should be 0 */
    union
    {
        RDNSEQUENCE_T                rdnSequence;
    }u;
} NAME_T;

typedef struct
{
    USHORT_T                    nbrOfElements;
    ATTRIBUTETYPEANDVALUE_T     attribute[MAXATTRTYPEVALUE];
} OUTPUTASSERTIONS_T;

typedef struct
{
    USHORT_T                    nbrOfElements;
    ATTRIBUTEVALUE_T            attributeValue[MAXATTRVALUE];
} SPECIFICINPUT_T;

typedef struct
{
    USHORT_T                    nbrOfElements;
    ATTRIBUTEVALUE_T            attributeValue[MAXATTRVALUE];
} SPECIFICOUTPUT_T;

typedef UCHAR_T ATTRIBUTEPROBLEM_T;

typedef struct
{
    ATTRIBUTEPROBLEM_T          attributeProblem;
    ATTRIBUTETYPE_T             attributeType;
    BOOLEAN_T                   attributeValueUsed;
    ATTRIBUTEVALUE_T            attributeValue;
} ATTRPROBLEMS_T;

typedef struct
{
    NAME_T                      name;
    USHORT_T                    nbrOfElements;
    ATTRPROBLEMS_T              problems[MAX_NBR_OF_PROBLEMS];
} ATTRIBUTEERROR_T;

typedef UCHAR_T EXECUTIONPROBLEM_T;

typedef struct
{
    EXECUTIONPROBLEM_T  executionProblem;
} EXECERROR_T;

typedef UCHAR_T NAMEPROBLEM_T;

typedef struct
{
    NAMEPROBLEM_T       nameProblem;
    NAME_T              name;
} NAMEERROR_T;


typedef struct 
{
        UCHAR_T                 dummy;
} ACTIVITYTESTARG_T;

typedef struct
{
        FILTEREDCALLTREATMENT_T         filteredCallTreatment;
        FILTERINGCHARACTERISTICS_T      filteringCharacteristics;
        FILTERINGTIMEOUT_T              filteringTimeOut;
        FILTERINGCRITERIA_T             filteringCriteria;
        BOOLEAN_T                       startTimeUsed;
        DATEANDTIME_T                   startTime;
        UCHAR_T                         nbrOfExtensions;
        EXTENSIONFIELD_T                extensions[NUMOFEXTENSIONS];
} ACTIVATESERVICEFILTERINGARG_T;

typedef struct
{
        ACHBILLINGCHARGINGCHARACTERISTICS_T aChBillingChargingCharacteristics;
        BOOLEAN_T               sendCalculationToSCPIndication;
        BOOLEAN_T               partyToChargeUsed;
        LEGID_T                 partyToCharge;
        UCHAR_T                 nbrOfExtensions;
        EXTENSIONFIELD_T        extensions[NUMOFEXTENSIONS];
} APPLYCHARGINGARG_T;

typedef CALLRESULT_T APPLYCHARGINGREPORTARG_T;

typedef struct
{
        CORRELATIONID_T         correlationID;
        BOOLEAN_T               iPAvailableUsed;
        IPAVAILABLE_T           iPAvailable;
        BOOLEAN_T               iPSSPCapabilitiesUsed;
        IPSSPCAPABILITIES_T     iPSSPCapabilities;
        UCHAR_T                 nbrOfExtensions;
        EXTENSIONFIELD_T        extensions[NUMOFEXTENSIONS];
} ASSISTREQUESTINSTRUCTIONSARG_T;

typedef struct
{
        GAPCRITERIA_T           gapCriteria;
        GAPINDICATORS_T         gapIndicators;
        BOOLEAN_T               controlTypeUsed;
        CONTROLTYPE_T           controlType;
        BOOLEAN_T               gapTreatmentUsed;
        GAPTREATMENT_T          gapTreatment;
        UCHAR_T                 nbrOfExtensions;
        EXTENSIONFIELD_T        extensions[NUMOFEXTENSIONS];
} CALLGAPARG_T;

typedef struct
{
        REQUESTEDINFORMATIONLIST_T      requestedInformationList;
        UCHAR_T                         nbrOfExtensions;
        EXTENSIONFIELD_T                extensions[NUMOFEXTENSIONS];
} CALLINFORMATIONREPORTARG_T;

typedef struct
{
        REQUESTEDINFORMATIONTYPELIST_T  requestedInformationTypeList;
        UCHAR_T                         nbrOfExtensions;
        EXTENSIONFIELD_T                extensions[NUMOFEXTENSIONS];
} CALLINFORMATIONREQUESTARG_T;

typedef struct
{
        UCHAR_T         chooser; /* 0 = invokeID, 1 = allRequests */
        union
        {
        INVOKEID_T              invokeID;
        UCHAR_T                 allRequests;
        } u;
} CANCELARG_T;

typedef struct
{
        UCHAR_T                 nbrOfExtensions;
        EXTENSIONFIELD_T        extensions[NUMOFEXTENSIONS];
} COLLECTINFORMATIONARG_T;


typedef struct
{
        DESTINATIONROUTINGADDRESS_T     destinationRoutingAddress;
        BOOLEAN_T               alertingPatternUsed;
        ALERTINGPATTERN_T       alertingPattern;
        BOOLEAN_T               correlationIDUsed;
        CORRELATIONID_T         correlationID;
        BOOLEAN_T               cutAndPasteUsed;
        CUTANDPASTE_T           cutAndPaste;
        BOOLEAN_T               originalCalledPartyIDUsed;
        ORIGINALCALLEDPARTYID_T originalCalledPartyID;
        BOOLEAN_T               routeListUsed;
        ROUTELIST_T             routeList;
        BOOLEAN_T               scfIDUsed;
        SCFID_T                 scfID;
        UCHAR_T                 nbrOfExtensions;
        EXTENSIONFIELD_T        extensions[NUMOFEXTENSIONS];
        BOOLEAN_T               serviceInteractionIndicatorsUsed;
        SERVICEINTERACTIONINDICATORS_T  serviceInteractionIndicators;
        BOOLEAN_T               callingPartyNumberUsed;
        CALLINGPARTYNUMBER_T    callingPartyNumber;
        BOOLEAN_T               callingPartysCategoryUsed;
        CALLINGPARTYSCATEGORY_T callingPartysCategory;
        BOOLEAN_T               redirectingPartyIDUsed;
        REDIRECTINGPARTYID_T    redirectingPartyID;
        BOOLEAN_T               redirectionInformationUsed;
        REDIRECTIONINFORMATION_T        redirectionInformation;
} CONNECTARG_T;

typedef struct
{
        UCHAR_T         chooser;        /* 0 = ipRoutingAddress,
                                1 = none*/
        union
        {
                IPROUTINGADDRESS_T      ipRoutingAddress;
                UCHAR_T                 none;
        } resourceAddress;
        UCHAR_T                 nbrOfExtensions;
        EXTENSIONFIELD_T        extensions[NUMOFEXTENSIONS];
        BOOLEAN_T               serviceInteractionIndicatorsUsed;
        SERVICEINTERACTIONINDICATORS_T  serviceInteractionIndicators;
} CONNECTTORESOURCEARG_T;

typedef struct
{
        UCHAR_T                 dummy;
} DISCONNECTFORWARDCONNECTIONARG_T;

typedef struct
{
        ASSISTINGSSPIPROUTINGADDRESS_T  assistingSSPIPRoutingAddress;
        BOOLEAN_T               correlationIDUsed;
        CORRELATIONID_T         correlationID;
        BOOLEAN_T               ScfIDUsed;
        SCFID_T                 ScfID;
        UCHAR_T                 nbrOfExtensions;
        EXTENSIONFIELD_T        extensions[NUMOFEXTENSIONS];
        BOOLEAN_T               serviceInteractionIndicatorsUsed;
        SERVICEINTERACTIONINDICATORS_T  serviceInteractionIndicators;
} ESTABLISHTEMPORARYCONNECTIONARG_T;

typedef struct
{
        EVENTTYPECHARGING_T     eventTypeCharging;
        BOOLEAN_T               eventSpecificInformationChargingUsed;
        EVENTSPECIFICINFORMATIONCHARGING_T eventSpecificInformationCharging;
        BOOLEAN_T               legIDUsed;
        LEGID_T                 legID;
        UCHAR_T                 nbrOfExtensions;
        EXTENSIONFIELD_T        extensions[NUMOFEXTENSIONS];
        MONITORMODE_T           monitorMode;
} EVENTNOTIFICATIONCHARGINGARG_T;

typedef struct
{
        EVENTTYPEBCSM_T         eventTypeBCSM;
        BOOLEAN_T               eventSpecificInformationBCSMUsed;
        EVENTSPECIFICINFORMATIONBCSM_T  eventSpecificInformationBCSM;
        BOOLEAN_T               legIDUsed;
        LEGID_T                 legID;
        MISCCALLINFO_T          miscCallInfo;
        UCHAR_T                 nbrOfExtensions;
        EXTENSIONFIELD_T        extensions[NUMOFEXTENSIONS];
} EVENTREPORTBCSMARG_T;


typedef struct
{
        SERVICEKEY_T                    serviceKey;
        BOOLEAN_T                       calledPartyNumberUsed;
        CALLEDPARTYNUMBER_T             calledPartyNumber;
        BOOLEAN_T                       callingPartyNumberUsed;
        CALLINGPARTYNUMBER_T            callingPartyNumber;
        BOOLEAN_T                       callingPartysCategoryUsed;
        CALLINGPARTYSCATEGORY_T         callingPartysCategory;
        BOOLEAN_T                       cGEncounteredUsed;
        CGENCOUNTERED_T                 cGEncountered;
        BOOLEAN_T                       iPSSPCapabilitiesUsed;
        IPSSPCAPABILITIES_T             iPSSPCapabilities;
        BOOLEAN_T                       iPAvailableUsed;
        IPAVAILABLE_T                   iPAvailable;
        BOOLEAN_T                       locationNumberUsed;
        LOCATIONNUMBER_T                locationNumber;
        BOOLEAN_T                       originalCalledPartyIDUsed;
        ORIGINALCALLEDPARTYID_T         originalCalledPartyID;
        UCHAR_T                         nbrOfExtensions;
        EXTENSIONFIELD_T                extensions[NUMOFEXTENSIONS];
        BOOLEAN_T                       highLayerCompatibilityUsed;
        HIGHLAYERCOMPATIBILITY_T        highLayerCompatibility;
        BOOLEAN_T                       serviceInteractionIndicatorsUsed;
        SERVICEINTERACTIONINDICATORS_T  serviceInteractionIndicators;
        BOOLEAN_T                       additionalCallingPartyNumberUsed;
        ADDITIONALCALLINGPARTYNUMBER_T  additionalCallingPartyNumber;
        BOOLEAN_T                       forwardCallIndicatorsUsed;
        FORWARDCALLINDICATORS_T         forwardCallIndicators;
        BOOLEAN_T                       bearerCapabilityUsed;
        BEARERCAPABILITY_T              bearerCapability;
        BOOLEAN_T                       eventTypeBCSMUsed;
        EVENTTYPEBCSM_T                 eventTypeBCSM;
        BOOLEAN_T                       redirectingPartyIDUsed;
        REDIRECTINGPARTYID_T            redirectingPartyID;
        BOOLEAN_T                       redirectionInformationUsed;
        REDIRECTIONINFORMATION_T        redirectionInformation;
} INITIALDPARG_T;

typedef struct
{
        DESTINATIONROUTINGADDRESS_T     destinationRoutingAddress;
        BOOLEAN_T                       alertingPatternUsed;
        ALERTINGPATTERN_T               alertingPattern;
        UCHAR_T                         nbrOfExtensions;
        EXTENSIONFIELD_T                extensions[NUMOFEXTENSIONS];
        BOOLEAN_T                       serviceInteractionIndicatorsUsed;
        SERVICEINTERACTIONINDICATORS_T  serviceInteractionIndicators;
        BOOLEAN_T                       callingPartyNumberUsed;
        CALLINGPARTYNUMBER_T            callingPartyNumber;
} INITIATECALLATTEMPTARG_T;

typedef struct
{
        INFORMATIONTOSEND_T             informationToSend;
        BOOLEAN_T                       disconnectFromIPForbidden;
        BOOLEAN_T                       requestAnnouncementComplete;
        UCHAR_T                         nbrOfExtensions;
        EXTENSIONFIELD_T                extensions[NUMOFEXTENSIONS];
} PLAYANNOUNCEMENTARG_T;

typedef struct
{
        COLLECTEDINFO_T                 collectedInfo;
        BOOLEAN_T                       disconnectFromIPForbidden;
        BOOLEAN_T                       informationToSendUsed;
        INFORMATIONTOSEND_T             informationToSend;
        UCHAR_T                         nbrOfExtensions;
        EXTENSIONFIELD_T                extensions[NUMOFEXTENSIONS];
} PROMPTANDCOLLECTUSERINFORMATIONARG_T;

typedef struct
{
        UCHAR_T         chooser;
        union
        {
                DIGITS_T                digitResponse;
        } u;
} RESEIVEDINFORMATIONARG_T;

typedef struct
{
        UCHAR_T         chooser;        /* 0 = DIGITS_T */
        union
        {
                DIGITS_T        digitsResponse;
        } u;
} RECEIVEDINFORMATIONARG_T;

typedef CAUSE_T RELEASECALLARG_T;

typedef struct
{
        UCHAR_T                 nbrOfChrgEvents;
        CHARGINGEVENT_T         chargingEvents[NUMOFCHARGINGEVENTS];
} REQUESTNOTIFICATIONCHARGINGEVENTARG_T;

typedef struct
{
        UCHAR_T                 nbrOfbcsmEvents;
        BCSMEVENT_T             bcsmEvents[NUMOFBCSMEVENTS];
        UCHAR_T                 nbrOfExtensions;
        EXTENSIONFIELD_T        extensions[NUMOFEXTENSIONS];
} REQUESTREPORTBCSMEVENTARG_T;

typedef struct
{
        TIMERID_T               timerID;
        TIMERVALUE_T            timervalue;
        UCHAR_T                 nbrOfExtensions;
        EXTENSIONFIELD_T        extensions[NUMOFEXTENSIONS];
} RESETTIMERARG_T;

typedef struct
{
        SCIBILLINGCHARGINGCHARACTERISTICS_T sCIBillingChargingCharacteristics;
        LEGID_T                 legID;
        UCHAR_T                 nbrOfExtensions;
        EXTENSIONFIELD_T        extensions[NUMOFEXTENSIONS];
} SENDCHARGINGINFORMATIONARG_T;


typedef struct
{
        COUNTERSVALUE_T         countersValue;
        FILTERINGCRITERIA_T     filteringCriteria;
        UCHAR_T                 nbrOfExtensions;
        EXTENSIONFIELD_T        extensions[NUMOFEXTENSIONS];
} SERVICEFILTERINGRESPONSEARG_T;

typedef UCHAR_T SPECIALIZEDRESOURCEREPORTARG_T;

typedef struct
{
        NAME_T                  name;
        METHODID_T              methodId;
        BOOLEAN_T               inputAssertionsUsed;
        INPUTASSERTIONS_T       inputAssertions;
        BOOLEAN_T               specificInputUsed;
        SPECIFICINPUT_T         specificInput;
} EXECUTEARG_T;

typedef struct
{
        METHODID_T              methodId;
        BOOLEAN_T               outputAssertionsUsed;
        OUTPUTASSERTIONS_T      outputAssertions;
        BOOLEAN_T               specificOutputUsed;
        SPECIFICOUTPUT_T        specificOutput;
} EXECUTERESULT_T;

typedef enum
{
        IN_E_UNKNOWNOPERATION = 0,
        IN_E_TOOLATE,
        IN_E_OPERATIONNOTCANCELLABLE
} PROBLEM_T;

typedef struct
{
        PROBLEM_T problem;
        INVOKEID_T              operation;
} CANCELFAILED_T;


typedef enum
{
        IN_E_UNKNOWNREQUESTEDINFO = 1,
        IN_E_REQUESTEDINFONOTAVAILABLE
} REQUESTEDINFOERROR_T;

typedef UNAVAILABLENETWORKRESOURCE_T SYSTEMFAILURE_T;


typedef enum
{
        IN_E_GENERIC = 0,
        IN_E_UNOBTAINABLE,
        IN_E_CONGESTION
} TASKREFUSED_T;



#if defined __cplusplus || defined c_plusplus
}
#endif
#endif /* __INAPCS1API_H__ */

