/*********************************************************************/
/* Don't remove these two lines, contain depends on them!            */
/* Document Number: %Container% Revision: %Revision% */
/*                                                                   */
/* et94inapcs1plusdefapi.h,v */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* COPYRIGHT Ericsson Infocom AB 1996                                */
/*                                                                   */
/* The copyright to the computer program herein is the property of   */
/* Ericsson Infocom Consultants AB. The program may be used and/or   */
/* copied only with the written permission from Ericsson Infocom     */
/* Consultants AB or in the accordance with the terms and conditions */
/* stipulated in the agreement/contract under which the program has  */
/* been supplied.                                                    */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Document Number:                 Rev:                             */
/* 19/190 55-CAA 201 15 Ux           A                               */
/*                                                                   */
/* Revision:                                                         */
/* @EINVER: et94inapcs1plusdefapi.h,v 1.5 2000/03/09 15:32:01 Exp */
/*                                                                   */
/* Programmer:                                                       */
/* EIN/N/B  Andreas Wilde                                            */
/*                                                                   */
/* Purpose:                                                          */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/* Revision record:                                                  */
/*                                                                   */
/* 00-97XXXX  EIN/N/B  Andreas Wilde                                 */
/*              Original version.                                    */
/*                                                                   */
/*-------------------------------------------------------------------*/
/* Notes:                                                            */
/*                                                                   */
/*********************************************************************/

#ifndef __INAPCS1PLUSDEFAPI_H__
#define __INAPCS1PLUSDEFAPI_H__
#if defined __cplusplus || defined c_plusplus
extern "C" {
#endif

/* operation codes */

#define IN_OP_CALL_LIMIT                        255
#define IN_OP_DIALOGUE_USER_INFO                254
#define IN_OP_HAND_OVER                         253
#define IN_OP_HOLD_CALL_PARTY_CON               252
#define IN_OP_RECONNECT                         251
#define IN_OP_RELEASE_CALL_PARTY_CON            250
#define IN_OP_RETRIEVE                          16
#define IN_OP_SIGN_INFO                         248
#define IN_OP_UPDATE                            21

/* internal operation codes */

#define IN_OP_INTRNL_RETRIEVE                   400

/* error codes */

#define IN_ERR_CONGESTION                       2
#define IN_ERR_ERRORINPARAMVALUE                3
#define IN_ERR_EXECUTIONERROR                   4
#define IN_ERR_IMPROPERCALLRESPONSE             4
#define IN_ERR_ILLEGALCOMBOFPARAM               7
#define IN_ERR_INFONOTAVAILABLE                 8
#define IN_ERR_INVDATAITEMID                    10
#define IN_ERR_NOTAUTHORIZED                    12
#define IN_ERR_OTHERERROR                       100
#define IN_ERR_PARAMETERMISSING                 13

/* internal error codes */

#define IN_INTRNL_ERR_EXECUTIONERROR            300
#define IN_INTRNL_ERR_ILLEGALCOMBOFPARAM        301
#define IN_INTRNL_ERR_INFONOTAVAILABLE          302
#define IN_INTRNL_ERR_INVDATAITEMID             303
#define IN_INTRNL_ERR_NOTAUTHORIZED             304
#define IN_INTRNL_ERR_PARAMETERMISSING          305

/* length constants */

#define BCAP_CPL_TMR_LEN                        1
#define CPLCD_DIGIT_LEN                         2
#define CPLMAXFCIBILLINGCHRGLENGTH              794
#define LENOFAPPLICATIONID                      1
#define LENOFBWRDCALLIND                        2
#define LENOFBWRDGVNSIND                        1
#define LENOFBWRDSUPPIND                        2
#define LENOFCALLDIVTREATIND                    1
#define LENOFCALLOFFTREATIND                    1
#define LENOFCALLTREATIND                       1
#define LENOFCHARGINGORIGIN                     1
#define LENOFCONFTREATIND                       1
#define LENOFCTRANSTREATIND                     1
#define LENOFCTREATIND                          1
#define LENOFCUGCALLIND                         1
#define LENOFDESTADDR                           1
#define LENOFDISTRIBUTED                        4
#define LENOFFWRDSUPPIND                        2
#define LENOFINTERLOCKCODE                      4
#define LENOFIPSSPCAPABILITIES                  2
#define LENOFOPERATIONID                        2
#define LENOFOPERATIONRETURNID                  3
#define LENOFPCANDSUBSYSNUM                     3
#define LENOFPCANDSUBSYSNUMANSI                 4
#define LENOFPCANDSUBSYSNUMCHINA                4
#define LENOFPROTOCOLIND                        2
#define LENOFRECEIVINGSCPCORRELATIONINFO        16
#define LENOFROUTEINDEX                         1
#define LENOFROUTELIST                          1
#define LENOFROUTEORIGIN                        2
#define LENOFSCFCORRINFO                        16
#define LENOFSENDINGSCPCORRINFO                 16
#define LENOFSERVICEFEATCODE                    2
#define LENOFTRAFFICACTCODE                     4
#define LENOFTARIFFINDICATOR                    2
#define MAXBEARERCAPABILITYLEN                  11
#define MAXBWRDSUPPIND                          2
#define MAXCANCELDIGITS                         2
#define MAX_CPL_ATTR_LEN                        127
#define MAX_CPLGEN_DIGITS_LEN                   14
#define MAX_CPL_GEN_NBR_LEN                     17
#define MAXDATAITEMLEN                          31
#define MAXDISPINFOLEN                          127
#define MAXENDOFREPLYDIGITS                     2
#define MAXEXISLEGSLEN                          30
#define MAXFWRDSUPPINDLEN                       10
#define MAXGENDIGITSSETLEN                      20
#define MAXGENNUMBERSETLEN                      20
#define MAXGLOBALTITLELEN                       12
#define MAXGLOBALTITLEANDSUBSYSNUMLEN           13
#define MAXGTITLEANDSUBSYSNUMLEN                13
#define MAXGVNSUSERGROUPIDLEN                   9
#define MAXNUMBERLEN                            16
#define MAXOPSPIDENTIFICLEN                     5
#define MAXREQINFOELMNTLEN                      5
#define MAXSTARTDIGITS                          2
#define MAXTARIFFCHARGEINFO                     4
#define MAXTNRNUMBERLEN                         10
#define MAXVARIABLEPARTSLEN                     20
#define SCF_CORRINFO_LEN                        16
#define VP_CPL_PRICE_WHOLE_LEN                  4
#define VP_CPL_PRICE_10_LEN                     4
#define VP_CPL_PRICE_100_LEN                    4
#define VP_CPL_PRICE_1000_LEN                   4
#define VP_CPL_TIME_LEN                         2

#define CS1P_ACLEN                              12
#define BLUE_TCAP_AC_LEN                        1

/* Default values */

#define DEFAULT_CS1P_CHRGNOCHRGIND                      2
#define DEFAULT_CS1P_DSCNNCTFRMIPFRBDDN                 TRUE
#define DEFAULT_CS1P_ERRORTREATMENT                     IN_E_STDERRORANDINFO
#define DEFAULT_CS1P_INITCALLATTMPT_LEG2BCRTD           1
#define DEFAULT_CS1P_INTERDIGITTIMEOUT                  10
#define DEFAULT_CS1P_INTRRPTBLANNIND                    TRUE
#define DEFAULT_CS1P_MAXNUMOFCOUNTERS                   1
#define DEFAULT_CS1P_MINIMUMNBOFDIGITS                  1
#define DEFAULT_CS1P_REQANNCMNTSTRTD                    FALSE
#define DEFAULT_CS1P_REQANNCMNTCMPLTE                   FALSE
#define DEFAULT_CS1P_SENDCALCULATIONTOSCPINDICATION     TRUE
#define DEFAULT_CS1P_SNDNGFNCTNSACTIVE                  1
#define DEFAULT_CS1P_SNDNGSIDEID1                       1
#define DEFAULT_CS1P_SNDNGSIDEID2                       2
#define DEFAULT_CS1P_VOICEBACK                          FALSE
#define DEFAULT_CS1P_VOICEINFORMATION                   FALSE

/* AC Short values */

#define IN_CPL_SAC_SSP_TO_SCP                           20
#define IN_CPL_SAC_ASSIST_HOFF_SSP_TO_SCP               21
#define IN_CPL_SAC_IP_TO_SCP                            22
#define IN_CPL_SAC_SCP_TO_SSP                           23
#define IN_CPL_SAC_SCP_TO_SSP_TFCMGMT                   24
#define IN_CPL_SAC_SCP_TO_SSP_SRVCMGMT                  25
#define IN_CPL_SAC_SSP_TO_SCP_SRVCMGMT                  26
#define IN_CPL_SAC_DATA_MGMT                            27
#define IN_CPL_SAC_SCP_SSP_TFCLIM                       28
#define IN_CPL_SAC_SSP_TO_SCP_LOCAL                     29
#define IN_CPL_SAC_HOFF_SSP_TO_SCP_LOCAL                30
#define IN_CPL_SAC_IP_TO_SCP_LOCAL                      31
#define IN_CPL_SAC_SCP_TO_SSP_LOCAL                     32
#define IN_CPL_SAC_SCP_TO_SSP_TFCMGMT_LOCAL             33
#define IN_CPL_SAC_SCP_TO_SSP_SRVCMGMT_LOCAL            34
#define IN_CPL_SAC_SSP_TO_SCP_SRVCMGMT_LOCAL            35
#define IN_CPL_SAC_DATA_MGMT_LOCAL                      36
#define IN_CPL_SAC_SCP_SSP_TFCLIM_LOCAL                 37


/* Constants for GenericDigits */

#define IN_E_ACCOUNT_CODE               0x00
#define IN_E_AUTH_CODE                  0x01
#define IN_E_PRIV_NETW_TRVL             0x02
#define IN_E_BUSINESS_COMM_GRPID        0x03
#define IN_E_BCD_EVEN                   0x00
#define IN_E_BCD_ODD                    0x20
#define IN_E_BINARY                     0x60

/* Constants for GenericNumber */

#define IN_E_ADD_CLLD_NBR               0x01
#define IN_E_ADD_CNCTD_NBR              0x05
#define IN_E_ADD_CLING_NBR              0x06
#define IN_E_ADD_ORG_CLLD_NBR           0x07
#define IN_E_ADD_RDRCTNG_NBR            0x08
#define IN_E_ADD_RDRCTON_NBR            0x09
#define IN_E_CLLD_FPH_NBR               0x0a


/* Error data types */

typedef enum
{
        IN_E_CPL_UNKNOWNOPERATION = 0, 
        IN_E_CPL_OPERATIONNOTCANCELLABLE = 2
} CPLPROBLEM_T;

typedef struct
{
        CPLPROBLEM_T                    problem;
        INVOKEID_T                      operation;
} CPLCANCELFAILED_T;

typedef UCHAR_T CONGESTION_T[LENOFOPERATIONRETURNID];

typedef enum
{
        IN_E_NOINFORMATIONRECEIVED = 0,
        IN_E_NOTENOUGHINFORMATIONRECEIVED
} IMPROPERCALLERRESPONSE_T;

typedef UCHAR_T ERRORINPARAMETERVALUE_T[LENOFOPERATIONRETURNID];

typedef UCHAR_T EXECUTIONERROR_T[LENOFOPERATIONRETURNID];

typedef UCHAR_T ILLEGALCOMBINATIONOFPARAMETERS_T[LENOFOPERATIONRETURNID];

typedef UCHAR_T INFONOTAVAILABLE_T[LENOFOPERATIONRETURNID];

typedef UCHAR_T INVALIDDATAITEMID_T[LENOFOPERATIONRETURNID];

typedef UCHAR_T NOTAUTHORIZED_T[LENOFOPERATIONRETURNID];

typedef UCHAR_T OTHERERROR_T[LENOFOPERATIONRETURNID];

typedef UCHAR_T PARAMETERMISSING_T[LENOFOPERATIONRETURNID];


/* Common data types */

typedef struct
{
        struct
        {
                UCHAR_T         chooser;        /* 1 = reportAtEndOfConnection,
                                                   2 = reportAtChargeLimit,
                                                   3 = reportImmediately */
                union
                {
                        UCHAR_T         reportAtEndOfConnection;
                        USHORT_T        reportAtChargeLimit;
                        UCHAR_T         reportImmediately;
                } u;
        } reportCondition;
        struct
        {
                BOOLEAN_T       accumulatedChargeUsed;
                UCHAR_T         accumulatedCharge;
                BOOLEAN_T       actualTariffUsed;
                UCHAR_T         actualTariff;
                BOOLEAN_T       tariffChargeInformationUsed;
                UCHAR_T         tariffChargeInformation;
                BOOLEAN_T       chargeableDurationUsed;
                UCHAR_T         chargeableDuration;
                BOOLEAN_T       timeOfAnswerUsed;
                UCHAR_T         timeOfAnswer;
        } requestedReportInfo;
} CPLACHBILLINGCHARGINGCHARACTERISTICS_T;

typedef UCHAR_T APPLICATIONID_T[LENOFAPPLICATIONID];

typedef struct
{
        USHORT_T                nbrOfElements;
        UCHAR_T                 attr[MAX_CPL_ATTR_LEN];
} CPLATTRIBUTE_T;

typedef UCHAR_T BACKWARDCALLINDICATORS_T[LENOFBWRDCALLIND];

typedef UCHAR_T BACKWARDGVNSINDICATOR_T[LENOFBWRDGVNSIND];

typedef UCHAR_T BACKWARDSUPPRESSIONIND_T[LENOFBWRDSUPPIND];

typedef struct
{
        UCHAR_T                 nbrOfElements;
        BACKWARDSUPPRESSIONIND_T bwrdSuppressionInd[MAXBWRDSUPPIND];
} BACKWARDSUPPRESSIONINDICATORS_T;

typedef UCHAR_T CPLLEGTYPE_T;

typedef struct
{
        UCHAR_T         chooser;
        union
        {
                CPLLEGTYPE_T receivingSideID;
        } u;
} RECEIVINGSIDEID_T;

typedef struct
{
        UCHAR_T         chooser;
        union
        {
                CPLLEGTYPE_T            sendingSideID;
        } u;
} SENDINGSIDEID_T;

typedef enum
{
        IN_E_CPL_TREANSWER = -9,
        IN_E_CPL_TSUSPENDED,
        IN_E_CPL_TCALLEDPARTYNOTRECHABLE,
        IN_E_CPL_TALERTING,
        IN_E_CPL_TROUTESELECTFAILURE,
        IN_E_CPL_OREANSWER,
        IN_E_CPL_OSUSPENDED,
        IN_E_CPL_OCALLEDPARTYNOTREACHABLE,
        IN_E_CPL_OALERTING,
        IN_E_CPL_ORIGATTEMPTAUTHORIZED = 1,
        IN_E_CPL_COLLECTEDINFO,
        IN_E_CPL_ANALYZEDINFORMATION,
        IN_E_CPL_ROUTESELECTFAILURE,
        IN_E_CPL_OCALLEDPARTYBUSY,
        IN_E_CPL_ONOANSWER,
        IN_E_CPL_OANSWER,
        IN_E_CPL_OMIDCALL ,
        IN_E_CPL_ODISCONNECT,
        IN_E_CPL_OABANDON,
        IN_E_CPL_TERMATTEMPTAUTHORIZED = 12,
        IN_E_CPL_TCALLEDPARTYBUSY,
        IN_E_CPL_TNOANSWER,
        IN_E_CPL_TANSWER,
        IN_E_CPL_TMIDCALL,
        IN_E_CPL_TDISCONNECT,
        IN_E_CPL_TABANDON 
} CPLEVENTTYPEBCSM_T;

typedef struct
{
        CPLEVENTTYPEBCSM_T      eventTypeBCSM;
        MONITORMODE_T           monitorMode;
        BOOLEAN_T               legIDUsed;
        SENDINGSIDEID_T         legID;
        BOOLEAN_T               dPSpecificCriteriaUsed;
        UCHAR_T chooser;        /* 0 = numberOfDigits,
                                 1 = applicationTimer,
                                11 = midCallEvents */
        union
        {
                UCHAR_T         numberOfDigits;
                USHORT_T        applicationTimer;
                struct
                {
                        UCHAR_T chooser; /* 0 = flash, 1 = userCallSuspend,
                                        2 = userCallResume,3 = dTMFMonitoring */
                        union
                        {
                                UCHAR_T         flash;
                                UCHAR_T         userCallSuspend;
                                UCHAR_T         userCallResume;
                                struct
                                {
                                        UCHAR_T minimumNumberOfDigits;
                                        UCHAR_T maximumNumberOfDigits;
                                        UCHAR_T nbrOfEndOfReplyDigits;
                                        UCHAR_T endOfReplyDigit[
                                                MAXENDOFREPLYDIGITS];
                                        UCHAR_T nbrOfCancelDigits;
                                        UCHAR_T cancelDigit[
                                                MAXCANCELDIGITS];
                                        UCHAR_T nbrOfStartDigits;
                                        UCHAR_T startDigit[
                                                MAXSTARTDIGITS];
                                        USHORT_T        interDigitTimeOut;
                                } dTMFMonitoring;
                        } u;
                }midCallEvents;
        } dPSpecificCriteria;
} CPLBCSMEVENT_T;

typedef struct
{
        UCHAR_T         chooser;        /* 0 = bearerCap, 1 = tmr */
        union
        {
                struct
                {
                        USHORT_T                nbrOfElements;
                        UCHAR_T                 bCap[MAXBEARERCAPABILITYLEN];
                } bearerCap;
                UCHAR_T         tmr[BCAP_CPL_TMR_LEN];
        } bc;
} CPLBEARERCAPABILITY_T;

typedef enum
{
        IN_E_BOTHWAYPATHREQUIRED = 0,
        IN_E_BOTHWAYPATHNOTREQUIRED
} BOTHWAYTHROUGHCONNECTIONINDICATOR_T;

typedef enum
{
        IN_E_ENDOFCONNECTION = 1,
        IN_E_CHARGELIMIT,
        IN_E_IMMEDIATLEY
} REPORTCONDITION_T;

typedef enum
{
        IN_E_TENMILLISECONDS = 1, 
        IN_E_ONEHUNDREDMILLISECONDS,
        IN_E_SECONDS
} INTERVALACCURACY_T;

typedef struct
{
        UCHAR_T                 numberOfStartPulses;
        USHORT_T                startInterval;
        INTERVALACCURACY_T      startIntervalAccuracy;
        UCHAR_T                 numberOfPeriodicPulses;
        USHORT_T                periodicInterval;
        INTERVALACCURACY_T      periodicIntervalAccuracy;
        BOOLEAN_T               activationTimeUsed;
        DATEANDTIME_T           activationTime;
} TARIFFINFORMATION_T;

typedef struct
{
        REPORTCONDITION_T               reportCondition;
        DATEANDTIME_T                   timeStamp;
        RECEIVINGSIDEID_T               partyToCharge;
        BOOLEAN_T                       accumulatedChargeUsed;
        USHORT_T                        accumulatedCharge;
        BOOLEAN_T                       actualTariffUsed;
        TARIFFINFORMATION_T             actualTariff;
        UCHAR_T                         nbrOfTariffChangeInformation;
        TARIFFINFORMATION_T             tariffChangeInformation[
                                        MAXTARIFFCHARGEINFO];
        BOOLEAN_T                       chargeableDurationUsed;
        ULONG_T                         chargeableDuration;
        BOOLEAN_T                       timeOfAnswerUsed;
        DATEANDTIME_T                   timeOfAnswer;
} CALLRESULTDETAILS_T;

typedef CALLRESULTDETAILS_T CPLCALLRESULT_T;

typedef struct
{
        BOOLEAN_T                       chargingOriginUsed;
        UCHAR_T                         chargingOrigin[LENOFCHARGINGORIGIN];
        BOOLEAN_T                       trafficActivityCodeUsed;
        UCHAR_T                         trafficActivityCode[
                                        LENOFTRAFFICACTCODE];
        BOOLEAN_T                       chargingCodeUsed;
        USHORT_T                        chargingCode;
} CHARGINGANALYSISINPUTDATA_T;

typedef enum
{
        IN_E_TARIFFINFORMATION = 1,
        IN_E_TARIFFINDICATOR, 
        IN_E_CHARGENOCHARGEINDICATION 
} CPLEVENTTYPECHARGING_T;

typedef struct
{
        CPLEVENTTYPECHARGING_T          eventTypeCharging;
        MONITORMODE_T                   monitorMode;
        SENDINGSIDEID_T                 legID;
} CPLCHARGINGEVENT_T;

typedef struct
{
        UCHAR_T         chooser;        /* should be zero */
        union
        {
                struct
                {
                UCHAR_T         minimumNbrOfDigits;
                UCHAR_T         maximumNbrOfDigits;
                UCHAR_T         nbrOfEndOfReplyDigit;
                UCHAR_T         endOfReplyDigit[CPLCD_DIGIT_LEN];
                UCHAR_T         nbrOfCancelDigit;
                UCHAR_T         cancelDigit[CPLCD_DIGIT_LEN];
                UCHAR_T         nbrOfStartDigit;
                UCHAR_T         startDigit[CPLCD_DIGIT_LEN];
                BOOLEAN_T       firstDigitTimeOutUsed;
                UCHAR_T         firstDigitTimeOut;
                BOOLEAN_T       interDigitTimeOutUsed;
                UCHAR_T         interDigitTimeOut;
                ERRORTREATMENT_T errorTreatment;
                BOOLEAN_T       interruptableAnnInd;
                BOOLEAN_T       voiceInformation;
                BOOLEAN_T       voiceBack;
                BOOLEAN_T       totalElapsedTimeOutUsed;
                UCHAR_T         totalElapsedTimeOut;
                } collectedDigits;
        } u;
} CPLCOLLECTEDINFO_T;

typedef enum
{
        IN_E_NOINIMPACT = 0,
        IN_E_PRESENTATIONRESTRICTED,
        IN_E_PRESENTCALLEDINNUMBER
} CONNECTEDNUMBERTREATMENTINDICATOR_T;

typedef struct
{
        BOOLEAN_T               conferenceTreatmentIndicatorUsed;
        UCHAR_T                 conferenceTreatmentIndicator;
        BOOLEAN_T               callDiversionTreatmentIndicatorUsed;
        UCHAR_T                 callDiversionTreatmentIndicator;
        BOOLEAN_T               callOfferingTreatmentIndicatorUsed;
        UCHAR_T                 callOfferingTreatmentIndicator;
        BOOLEAN_T               callTransferTreatmentIndicatorUsed;
        UCHAR_T                 callTransferTreatmentIndicator;
} FORWARDSERVICEINTERACTIONINDICATORS_T;

typedef struct
{
        BOOLEAN_T       conferenceTreatmentIndicatorUsed;
        struct
        {
                UCHAR_T         cTreatInd[LENOFCTREATIND];
        } conferenceTreatmentIndicator;
        BOOLEAN_T       callTransferTreatmentIndicatorUsed;
        struct
        {
                UCHAR_T         cTransTreatInd[LENOFCTRANSTREATIND];
        } callTransferTreatmentIndicator;
} BACKWARDSERVICEINTERACTIONINDICATORS_T;

typedef UCHAR_T SUSPENDTIMER_T;

typedef struct
{
        BOOLEAN_T               forwardServiceInteractionIndicatorsUsed;
        FORWARDSERVICEINTERACTIONINDICATORS_T
                                forwardServiceInteractionIndicators;
        BOOLEAN_T               backwardServiceInteractionIndicatorsUsed;
        BACKWARDSERVICEINTERACTIONINDICATORS_T
                                backwardServiceInteractionIndicators;
        BOOLEAN_T               suspendTimerUsed;
        SUSPENDTIMER_T          suspendTimer;
        BOOLEAN_T               connectedNumberTreatmentIndicatorUsed;
        CONNECTEDNUMBERTREATMENTINDICATOR_T
                                connectedNumberTreatmentIndicator;
} CONSERVICEINTERACTIONINDICATORS_T;

typedef enum
{
        IN_E_CPL_SCPOVERLOADED = 0,
        IN_E_CPL_MANUALLYINITIATED,
        IN_E_CPL_DESTINATIONOVERLOAD,
        IN_E_CPL_SOCC = -1
} CPLCONTROLTYPE_T;

typedef struct
{
        BOOLEAN_T               backwardServiceInteractionIndicatorsUsed;
        BACKWARDSERVICEINTERACTIONINDICATORS_T
                                backwardServiceInteractionIndicators;
        BOOLEAN_T               bothwayThroughConnectIndicatorUsed;
        BOTHWAYTHROUGHCONNECTIONINDICATOR_T
                                bothwayThroughConnectIndicator;
        BOOLEAN_T               connectedNumberTreatmentIndicatorUsed;
        CONNECTEDNUMBERTREATMENTINDICATOR_T
                                connectedNumberTreatmentIndicator;
} CTRSERVICEINTERACTIONINDICATORS_T;

typedef UCHAR_T CUGCALLINDICATOR_T[LENOFCUGCALLIND];

typedef UCHAR_T CUGINTERLOCKCODE_T[LENOFINTERLOCKCODE];

typedef struct
{
        UCHAR_T                 nbrOfElements;
        BOOLEAN_T               attributeUsed[MAXDATAITEMLEN];
        CPLATTRIBUTE_T          attributes[MAXDATAITEMLEN];
} DATAITEMID_T;

typedef DATAITEMID_T CPLDATAITEMINFORMATION_T;

typedef struct
{
        UCHAR_T         nbrOfElements;
        UCHAR_T         number[MAXNUMBERLEN];
} CPLNUMBER_T;

typedef CPLNUMBER_T CPLDESTINATIONROUTINGADDRESS_T[LENOFDESTADDR];

typedef USHORT_T CPLELEMENTARYMESSAGEID_T;

typedef CTRSERVICEINTERACTIONINDICATORS_T ETCSERVICEINTERACTIONINDICATORS_T;

typedef struct
{
        UCHAR_T         oct1;
        UCHAR_T         nbrOfElements;
        UCHAR_T         genDigits[MAX_CPLGEN_DIGITS_LEN];
} CPLGENERICDIGITS_T;

typedef struct
{
        BOOLEAN_T                       timeToAnswerUsed;
        USHORT_T                        timeToAnswer;
        BOOLEAN_T                       backwardCallIndicatorsUsed;
        BACKWARDCALLINDICATORS_T        backwardCallIndicators;
        BOOLEAN_T                       backwardGVNSIndicatorUsed;
        BACKWARDGVNSINDICATOR_T         backwardGVNSIndicator;
} ANSWERSPECIFICINFO_T;

typedef struct
{
        BOOLEAN_T               midCallEventsUsed;
        UCHAR_T                 chooser;  /* 0= flash,
                                             1= userCallSuspend,
                                             2= userCallResume,
                                             3= dTMFDigitsCompleted,
                                             4= dTMFDigitsTimeOut */

        union
        {
                UCHAR_T                 flash;
                UCHAR_T                 userCallSuspend;
                UCHAR_T                 userCallResume;
                CPLGENERICDIGITS_T      dTMFDigitsCompleted;
                CPLGENERICDIGITS_T      dTMFDigitsTimeOut;
        } midCallEvents;
} MIDCALLSPECIFICINFO_T;

typedef struct
{
        UCHAR_T         chooser; /* same value as chosen tag except:
                                    21 = oCalledPartyNotReachableSpecificInfo,
                                    22 = oAlertingSpecificInfo
                                    23 = tCalledPartyNotReachableSpecificInfo
                                    26 = tRouteSelectFailureSpecificInfo
                                    27 = tAlertingSpecificInfo*/
        union
        {
                struct
                {
                        CPLNUMBER_T             calledPartyNumber;
                } collectedInfoSpecificInfo;
                struct
                {
                        CPLNUMBER_T             calledPartyNumber;
                } analyzedInfoSpecificInfo;
                struct
                {
                        BOOLEAN_T               failureCauseUsed;
                        CAUSE_T                 failureCause; 
                } routeSelectFailureSpecificInfo;
                struct
                {
                        BOOLEAN_T               busyCauseUsed;
                        CAUSE_T                 busyCause;
                } oCalledPartyBusySpecificInfo;
                struct
                {
                        BOOLEAN_T               notReachableCauseUsed;
                        CAUSE_T                 notReachableCause;
                } oCalledPartyNotReachableSpecificInfo;
                struct
                {
                        BACKWARDCALLINDICATORS_T
                                                backwardCallIndicators;
                } oAlertingSpecificInfo;
                struct
                {
                        UCHAR_T                 dummy;
                } oNoAnswerSpecificInfo;
                ANSWERSPECIFICINFO_T            oAnswerSpecificInfo;
                MIDCALLSPECIFICINFO_T           oMidCallSpecificInfo;
                struct
                {
                        BOOLEAN_T               releaseCauseUsed;
                        CAUSE_T                 releaseCause;
                } oDisconnectSpecificInfo;
                struct
                {
                        BOOLEAN_T               failureCauseUsed;
                        CAUSE_T                 failureCause;
                } tRouteSelectFailureSpecificInfo;
                struct
                {
                        BOOLEAN_T               busyCauseUsed;
                        CAUSE_T                 busyCause;
                } tCalledPartyBusySpecificInfo;
                struct
                {
                        BOOLEAN_T               notReachableCauseUsed;
                        CAUSE_T                 notReachableCause;
                } tCalledPartyNotReachableSpecificInfo;
                struct
                {
                        BACKWARDCALLINDICATORS_T
                                                backwardCallIndicators;
                } tAlertingSpecificInfo;
                struct
                {
                UCHAR_T                         dummy;
                } tNoAnswerSpecificInfo;
                ANSWERSPECIFICINFO_T            tAnswerSpecificInfo;
                MIDCALLSPECIFICINFO_T           tMidCallSpecificInfo;
                struct
                {
                        BOOLEAN_T               releaseCauseUsed;
                        CAUSE_T                 releaseCause;
                } tDisconnectSpecificInfo;
        } u;
} CPLEVENTSPECIFICINFORMATIONBCSM_T;

typedef enum
{
        EN_E_CALLFREEOFCHARGE = 1,
        IN_E_CHARGEABLECALL
} CHARGENOCHARGEINDICATION_T;

typedef struct
{
        UCHAR_T                 chooser; /* same value as 
                                        chosen tag */
        union
        {
                TARIFFINFORMATION_T             tariffInformation;
                UCHAR_T                         tariffIndicator[
                                                LENOFTARIFFINDICATOR];
                CHARGENOCHARGEINDICATION_T      chargeNoChargeIndication;
        } u;
} EVENTSPECIFICINFOCHARGING_T;

typedef struct
{
        RECEIVINGSIDEID_T               legID;
        BOOLEAN_T                       linkIndUsed;
} EXISTINGLEGS_T;

typedef enum
{
        IN_E_APARTYTOBECHARGED = 0,
        IN_E_BPARTYTOBECHARGED,
        IN_E_CPARTYTOBECHARGED,
        IN_E_OTHERPARTYTOBECHARGED
} SINGLE_T;

typedef enum
{
        IN_E_ATRESOURCECONNECTION = 0,
        IN_E_ATANSWER,
        IN_E_WHENORDERED
} STARTOFCHARGINGINDICATOR_T;

typedef enum
{
        IN_E_RESUMECHARGING = 1,
        IN_E_HOLDCHARGING
} CHARGINGCHANGEINDICATOR_T;

typedef struct
{
        UCHAR_T                 nbrQualInd;
        UCHAR_T                 natOfAddrInd;
        UCHAR_T                 oct3;
        UCHAR_T                 nbrOfElements;
        UCHAR_T                 genNbr[MAX_CPL_GEN_NBR_LEN];
} CPLGENERICNUMBER_T;

typedef struct
{
        UCHAR_T                 nbrOfElements;
        CPLGENERICNUMBER_T      genNumberSet[MAXGENNUMBERSETLEN];
} GENERICNUMBERSET_T;

typedef struct
{
        BOOLEAN_T               tariffInformationUsed;
        TARIFFINFORMATION_T     tariffInformation;
        BOOLEAN_T               tariffIndicatorUsed;
        UCHAR_T                 tariffIndicator[LENOFTARIFFINDICATOR];
        CHARGENOCHARGEINDICATION_T
                                chargeNoChargeIndication;
} CPLSFBILLINGCHARGINGCHARACTERISTICS_T;

typedef struct
{
        UCHAR_T                 nbrOfElements;
        CPLGENERICDIGITS_T      genDigitsSet[MAXGENDIGITSSETLEN];
} GENERICDIGITSSET_T;

typedef struct
{
        SENDINGSIDEID_T                         partyToCharge;
        BOOLEAN_T                               createINBillingRecordUsed;
        UCHAR_T                                 createINBillingRecord;
        BOOLEAN_T                               additionalBillingInfoUsed;
        struct
        {
                BOOLEAN_T                       chargePartyUsed;
                UCHAR_T                         chooser; /* same value as chosen
                                                                tag */
                union
                {
                        SINGLE_T                single;
                        UCHAR_T                 distributed[LENOFDISTRIBUTED];
                } chargeParty;
                BOOLEAN_T                       serviceFeatureCodeUsed;
                UCHAR_T                         serviceFeatureCode[
                                                LENOFSERVICEFEATCODE];
                BOOLEAN_T                       chargingUnitsAdditionUsed;
                USHORT_T                        chargingUnitsAddition;
                BOOLEAN_T                       genericChargingDigitsUsed;
                GENERICDIGITSSET_T              genericChargingDigits;
                BOOLEAN_T                       genericNumberSetUsed;
                GENERICNUMBERSET_T              genericNumberSet;
        } additionalBillingInfo;
        BOOLEAN_T                               tariffInformationUsed;
        TARIFFINFORMATION_T                     tariffInformation;
        BOOLEAN_T                               startOfChargingIndicatorUsed;
        STARTOFCHARGINGINDICATOR_T              startOfChargingIndicator;
        BOOLEAN_T                               orderStartOfChargingUsed;
        UCHAR_T                                 orderStartOfCharging;
        BOOLEAN_T                               chargingChangeIndicatorUsed;
        CHARGINGCHANGEINDICATOR_T               chargingChangeIndicator;
} FCIBILLINGDETAILS_T;

typedef FCIBILLINGDETAILS_T CPLFCIBILLINGCHARGINGCHARACTERISTICS_T;

typedef CPLFCIBILLINGCHARGINGCHARACTERISTICS_T 
                CPLFURNISHCHARGINGINFORMATIONARG_T;

typedef struct
{
        UCHAR_T         chooser;        /* 0 = integer, 
                                1 = number, 
                                2 = time, 
                                11 = priceInWholeUnits,
                                12 = priceInTenthsOfUnits,
                                4 = priceInHundredthsOfUnits, 
                                13 = priceInThousandthsOfUnits
                                */
        union
        {
                USHORT_T                integer;
                CPLGENERICDIGITS_T      number;
                UCHAR_T                 time[VP_CPL_TIME_LEN];
                UCHAR_T                 priceInWholeUnits[
                                        VP_CPL_PRICE_WHOLE_LEN];
                UCHAR_T                 priceInTenthsOfUnits[
                                        VP_CPL_PRICE_10_LEN];
                UCHAR_T                 priceInHundredthsOfUnits[
                                        VP_CPL_PRICE_100_LEN];
                UCHAR_T                 priceInThousandthsOfUnits[
                                        VP_CPL_PRICE_1000_LEN];
        } u;
} CPLVARIABLEPARTS_T;

typedef struct
{
        UCHAR_T         chooser;                /* 0 = inbandinfo, 
                                        1 = tone,
                                        2 = displayInformation 
                                        11 = textID */
        union
        {
                struct
                {
                        UCHAR_T         chooser;        /* 0 = elementary-
                                                        MessageID */
                        union
                        {
                                CPLELEMENTARYMESSAGEID_T elementaryMessageID;
                                struct
                                {
                                        CPLELEMENTARYMESSAGEID_T
                                                        elementaryMessageID;
                                        USHORT_T        nbrOfVarPElements;
                                        CPLVARIABLEPARTS_T
                                                        variableParts[
                                                        MAXVARIABLEPARTSLEN];
                                } variableMessage;
                        } messageID;
                        BOOLEAN_T                       numberOfRepetitionsUsed;
                        UCHAR_T                         numberOfRepetitions;
                        BOOLEAN_T                       durationUsed;
                        USHORT_T                        duration;
                        BOOLEAN_T                       intervalUsed;
                        USHORT_T                        interval;
                } inbandInfo;
                struct
                {
                        USHORT_T                        toneID;
                        BOOLEAN_T                       durationUsed;
                        UCHAR_T                         duration;
                } tone;
                struct
                {
                        USHORT_T                nbrOfElements;
                        IA5STRING_T             dispInfo[MAXDISPINFOLEN];
                } displayInformation;
                UCHAR_T                 textID;
        } u;
} CPLINFORMATIONTOSEND_T;

typedef struct
{
        CPLSFBILLINGCHARGINGCHARACTERISTICS_T 
                                        sFBillingChargingCharacteristics;
        BOOLEAN_T                       informationToSendUsed;
        CPLINFORMATIONTOSEND_T          informationToSend;
        UCHAR_T                         maximumNumberOfCounters;
        BOOLEAN_T                       releaseCauseUsed;
        CAUSE_T                         releaseCause;
} CPLFILTEREDCALLTREATMENT_T;

typedef struct
{
        UCHAR_T         chooser;        /* 2 = ServiceKey, 
                                30 = addressAndService */
        union
        {
                SERVICEKEY_T            serviceKey;
                struct 
                {
                CPLGENERICNUMBER_T      calledAddressValue;
                SERVICEKEY_T            serviceKey;
                BOOLEAN_T               callingAddressValueUsed;
                CPLGENERICNUMBER_T      callingAddressValue;
                BOOLEAN_T               locationNumberUsed;
                CPLNUMBER_T             locationNumber;
                } addressAndService;
        } u;
} CPLFILTERINGCRITERIA_T;

typedef struct
{
        UCHAR_T         nbrOfElements;
        UCHAR_T         oPSPIdentification[MAXOPSPIDENTIFICLEN];
} OPSPIDENTIFICATION_T;

typedef struct
{
        UCHAR_T         nbrOfElements;
        UCHAR_T         gVNSUserGroupID[MAXGVNSUSERGROUPIDLEN];
} GVNSUSERGROUPID_T;

typedef struct
{
        UCHAR_T                 nbrOfTNRNum;
        UCHAR_T                 tNRNumber[MAXTNRNUMBERLEN];
} TNRNUMBER_T;

typedef struct
{
        OPSPIDENTIFICATION_T                    oPSPIdentification;
        GVNSUSERGROUPID_T                       gVNSUserGroupID;
        TNRNUMBER_T                             tNRNumber;
} FORWARDGVNSINDICATOR_T;

typedef UCHAR_T FWRDSUPPRESSIONIND_T[LENOFFWRDSUPPIND];

typedef struct
{
        UCHAR_T                 nbrOfElements;
        FWRDSUPPRESSIONIND_T    fwrdSuppressionInd[MAXFWRDSUPPINDLEN];
} FORWARDSUPPRESSIONINDICATORS_T;

typedef struct
{
        UCHAR_T chooser;        /* 0 = calledAddressValue, 2 = gapOnService,
                        29 = calledAddressAndService,
                        30 = callingAddressAndService */
        union
        {
        CPLGENERICNUMBER_T              calledAddressValue;
        GAPONSERVICE_T          gapOnService;
        struct
        {
                CPLGENERICNUMBER_T      calledAddressValue;
                SERVICEKEY_T            serviceKey;
                
        } calledAddressAndService;
        struct
        {
                CPLGENERICNUMBER_T      callingAddressValue;
                SERVICEKEY_T            serviceKey;
                BOOLEAN_T               locationNumberUsed;
                CPLNUMBER_T             locationNumber;
        } callingAddressAndService;
        } u;
} CPLGAPCRITERIA_T;

typedef struct
{
        UCHAR_T                 allowedNumberOfCalls;
        DURATION_T              duration;
        INTERVAL_T              gapInterval;
} CPLGAPINDICATORS_T;

typedef enum
{
        IN_DUI_NORMAL = 1, 
        IN_DUI_SIMULATION, 
        IN_DUI_MESSAGELOGGING, 
        IN_DUI_SIMULATIONPRETTYPRINT, 
        IN_DUI_MESSAGELOGGINGPRETTYPRINT, 
        IN_DUI_SERVICELOGICTRACE,
        IN_DUI_LOGANDTRACE,
        IN_DUI_LOGGINGPRETTYPRINTANDTRACE
} DUI_ENUM_T;

typedef struct
{
        DUI_ENUM_T              sendingFunctionsActive;
        DUI_ENUM_T              receivingFunctionsRequested;
        BOOLEAN_T               trafficSimulationSessionIDUsed;
        USHORT_T                trafficSimulationSessionID;
} DIALOGUEUSERINFORMATIONARG_T;

typedef struct
{
        UCHAR_T                 chooser; /* same value as chosen tag */
        union
        {
                UCHAR_T         coLocated;
#ifdef EINSS7_TWENTYFOURBITPC /* CHINESE ADDRESSING */
                UCHAR_T         pointCodeAndSubSysNum[LENOFPCANDSUBSYSNUMCHINA];
#else
                UCHAR_T         pointCodeAndSubSysNum[LENOFPCANDSUBSYSNUM];
#endif /* EINSS7_TWENTYFOURBITPC */
                struct
                {
                        UCHAR_T         nbrOfGlobalTitle;
                        UCHAR_T         glTitle[MAXGLOBALTITLELEN];
                } globalTitle;
                struct
                {
                        UCHAR_T         nbrGlobalTitleAndSubSysNum;
                        UCHAR_T         glTitleAndSubSysNum[
                                        MAXGTITLEANDSUBSYSNUMLEN];
                } globalTitleAndSubSysNum;
                UCHAR_T                 pointCodeAndSubSysNumANSI[
                                        LENOFPCANDSUBSYSNUMANSI];
        } u;
} SCPADDRESS_T; 

typedef struct
{
        UCHAR_T                         protocolIndicator[LENOFPROTOCOLIND];
        BOOLEAN_T                       userInformationUsed;
        DIALOGUEUSERINFORMATIONARG_T    userInformation;
} SCPDIALOGUEINFO_T;

typedef struct
{
        UCHAR_T                 handOverCounter;
        SCPADDRESS_T            sendingSCPAddress;
        SCPDIALOGUEINFO_T       sendingSCPDialogueInfo;
        BOOLEAN_T               sendingSCPCorrInfoUsed;
        UCHAR_T                 sendingSCPCorrInfo[LENOFSENDINGSCPCORRINFO];
        SCPADDRESS_T            receivingSCPAddress;
        SCPDIALOGUEINFO_T       receivingSCPDialogueInfo;
        BOOLEAN_T               receivingSCPCorrelationInfoUsed;
        UCHAR_T                 receivingSCPCorrelationInfo[
                                LENOFRECEIVINGSCPCORRELATIONINFO];
        BOOLEAN_T               handOverNumberUsed;
        CPLNUMBER_T             handOverNumber;
        BOOLEAN_T               handOverDataUsed;
        USHORT_T                handOverData;
} HANDOVERINFO_T;

typedef struct
{
        BOOLEAN_T               forwardServiceInteractionIndicatorsUsed;
        FORWARDSERVICEINTERACTIONINDICATORS_T
                                forwardServiceInteractionIndicators;
        BOOLEAN_T               suspendTimerUsed;
        SUSPENDTIMER_T          suspendTimer;
} ICASERVICEINTERACTIONINDICATORS_T;

typedef struct
{
        BOOLEAN_T               forwardServiceInteractionIndicatorsUsed;
        FORWARDSERVICEINTERACTIONINDICATORS_T
                                forwardServiceInteractionIndicators;
        BOOLEAN_T               backwardServiceInteractionIndicatorsUsed;
        BACKWARDSERVICEINTERACTIONINDICATORS_T
                                backwardServiceInteractionIndicators;
} IDPSERVICEINTERACTIONINDICATORS_T;

typedef struct
{
        UCHAR_T                 chooser;        /* 0 = informationToSend,
                                1 = releaseCause, 2 = both */
        union
        {
        CPLINFORMATIONTOSEND_T                  informationToSend;
        CAUSE_T                                 releaseCause;
        struct
        {
                CPLINFORMATIONTOSEND_T           informationToSend;
                CAUSE_T                         releaseCause;
        } both;
        } u;
} CPLGAPTREATMENT_T;

typedef CPLNUMBER_T CPLIPROUTINGADDRESS_T;

typedef UCHAR_T CPLIPSSPCAPABILITIES_T[LENOFIPSSPCAPABILITIES];

typedef struct
{
        UCHAR_T                 nbrOfElements;
        EXISTINGLEGS_T          existingLeg[MAXEXISLEGSLEN];
} LEGIDS_T;

typedef CPLGAPCRITERIA_T LIMITCRITERIA_T;

typedef struct
{
        LONG_T          duration;
} LIMITINDICATORS_T;

typedef CPLGAPTREATMENT_T LIMITTREATMENT_T;

typedef struct
{
        REQUESTEDINFORMATIONTYPE_T      requestedInformationType;
        UCHAR_T                 chooser; /* same value as 
                                        chosen tag */
        union
        {
                UCHAR_T         callAttemptElapsedTimeValue;
                DATEANDTIME_T   callStopTimeValue;
                ULONG_T         callConnectedElapsedTimeValue;
                CPLGENERICNUMBER_T      calledAddressValue;
                CAUSE_T         releaseCause;
        } requestedInformationValue;
} REQUESTEDINFORMATIONELEMENT_T;

typedef struct
{
        UCHAR_T                         nbrOfElements;
        REQUESTEDINFORMATIONELEMENT_T   requestedInformationElement[
                                        MAXREQINFOELMNTLEN];
} CPLREQUESTEDINFORMATIONLIST_T;

typedef enum
{
        IN_E_INTERMEDIATERESPONSE = 0, 
        IN_E_LASTRESPONSE
} RESPONSECONDITION_T;

typedef UCHAR_T ROUTEINDEX_T[LENOFROUTEINDEX];

typedef ROUTEINDEX_T CPLROUTELIST_T[LENOFROUTELIST];

typedef UCHAR_T ROUTEORIGIN_T[LENOFROUTEORIGIN];

typedef struct
{
        UCHAR_T                 chooser; /* same value as chosen tag */
        union
        {
                struct
                {
                        BOOLEAN_T       orderStartOfChargingUsed;
                        UCHAR_T         orderStartOfCharging;
                        BOOLEAN_T       chargeMessageUsed;
                        struct
                        {
                                CPLEVENTTYPECHARGING_T  eventTypeCharging;
                                EVENTSPECIFICINFOCHARGING_T
                                                eventSpecificInfoCharging;
                        } chargeMessage;
                        BOOLEAN_T       pulseBurstUsed;
                        UCHAR_T         pulseBurst;
                        BOOLEAN_T       createDefaultBillingRecordUsed;
                        UCHAR_T         createDefaultBillingRecord;
                } chargingInformation;
                CHARGINGANALYSISINPUTDATA_T     chargingAnalysisInputData;
        } u;
} CPLSCIBILLINGCHARGINGCHARACTERISTICS_T;

typedef UCHAR_T TRIGGERTYPE_T;



/* Argument datatypes */

typedef struct
{
        CPLFILTEREDCALLTREATMENT_T      filteredCallTreatment;
        FILTERINGCHARACTERISTICS_T      filteringCharacteristics;
        FILTERINGTIMEOUT_T              filteringTimeOut;
        CPLFILTERINGCRITERIA_T          filteringCriteria;
        BOOLEAN_T                       startTimeUsed;
        DATEANDTIME_T                   startTime;
        UCHAR_T                         nbrOfExtensions;
        EXTENSIONFIELD_T                extensions[NUMOFEXTENSIONS];
        BOOLEAN_T                       sCFCorrelationInfoUsed;
        UCHAR_T                         sCFCorrelationInfo[SCF_CORRINFO_LEN];
} CPLACTIVATESERVICEFILTERINGARG_T;

typedef struct
{
        CPLACHBILLINGCHARGINGCHARACTERISTICS_T 
                                aChBillingChargingCharacteristics;
        BOOLEAN_T               sendCalculationToSCPIndication;
        SENDINGSIDEID_T         partyToCharge;
        UCHAR_T                 nbrOfExtensions;
        EXTENSIONFIELD_T        extensions[NUMOFEXTENSIONS];
} CPLAPPLYCHARGINGARG_T;

typedef CPLCALLRESULT_T CPLAPPLYCHARGINGREPORTARG_T;

typedef struct
{
        CPLGENERICNUMBER_T      correlationID;
        BOOLEAN_T               iPSSPCapabilitiesUsed;
        CPLIPSSPCAPABILITIES_T  iPSSPCapabilities;
        UCHAR_T                 nbrOfExtensions;
        EXTENSIONFIELD_T        extensions[NUMOFEXTENSIONS];
} CPLASSISTREQUESTINSTRUCTIONSARG_T;

typedef struct
{
        BOOLEAN_T                       startTimeUsed;
        DATEANDTIME_T                   startTime;
        CPLGAPCRITERIA_T                gapCriteria;
        CPLGAPINDICATORS_T              gapIndicators;
        BOOLEAN_T                       controlTypeUsed;
        CPLCONTROLTYPE_T                controlType;
        BOOLEAN_T                       gapTreatmentUsed;
        CPLGAPTREATMENT_T               gapTreatment;
        UCHAR_T                         nbrOfExtensions;
        EXTENSIONFIELD_T                extensions[NUMOFEXTENSIONS];
} CPLCALLGAPARG_T;

typedef struct
{
        BOOLEAN_T                       legIDUsed;
        RECEIVINGSIDEID_T               legID;
        CPLREQUESTEDINFORMATIONLIST_T   requestedInformationList;
        UCHAR_T                         nbrOfExtensions;
        EXTENSIONFIELD_T                extensions[NUMOFEXTENSIONS];
} CPLCALLINFORMATIONREPORTARG_T;

typedef struct
{
        BOOLEAN_T                       legIDUsed;
        SENDINGSIDEID_T                 legID;
        REQUESTEDINFORMATIONTYPELIST_T requestedInformationTypeList;
        UCHAR_T                         nbrOfExtensions;
        EXTENSIONFIELD_T                extensions[NUMOFEXTENSIONS];
} CPLCALLINFORMATIONREQUESTARG_T;

typedef struct
{
        BOOLEAN_T                       startTimeUsed;
        DATEANDTIME_T                   startTime;
        LIMITCRITERIA_T                 limitCriteria;
        LIMITINDICATORS_T               limitIndicators;
        BOOLEAN_T                       limitTreatmentUsed;
        LIMITTREATMENT_T                limitTreatment;
} CALLLIMITARG_T;

typedef struct
{
        SENDINGSIDEID_T                 legToBeCreated;
        BOOLEAN_T                       bearerCapabilitiesUsed;
        CPLBEARERCAPABILITY_T           bearerCapabilities;
        BOOLEAN_T                       cUGCallIndicatorUsed;
        CUGCALLINDICATOR_T              cUGCallIndicator;
        BOOLEAN_T                       cUGInterLockCodeUsed;
        CUGINTERLOCKCODE_T              cUGInterLockCode;
        BOOLEAN_T                       forwardCallIndicatorsUsed;
        FORWARDCALLINDICATORS_T         forwardCallIndicators;
        BOOLEAN_T                       genericDigitsSetUsed;
        GENERICDIGITSSET_T              genericDigitsSet;
        BOOLEAN_T                       genericNumberSetUsed;
        GENERICNUMBERSET_T              genericNumberSet;
        BOOLEAN_T                       highLayerCompatibilityUsed;
        HIGHLAYERCOMPATIBILITY_T        highLayerCompatibility;
        BOOLEAN_T                       forwardGVNSIndicatorUsed;
        FORWARDGVNSINDICATOR_T          forwardGVNSIndicator;
        BOOLEAN_T                       destinationRoutingAddressUsed;
        CPLDESTINATIONROUTINGADDRESS_T  destinationRoutingAddress;
        BOOLEAN_T                       alertingPatternUsed;
        ALERTINGPATTERN_T               alertingPattern;
        BOOLEAN_T                       correlationIDUsed;
        CPLGENERICDIGITS_T              correlationID;
        BOOLEAN_T                       cutAndPasteUsed;
        CUTANDPASTE_T                   cutAndPaste;
        BOOLEAN_T                       originalCalledPartyIDUsed;
        CPLNUMBER_T                     originalCalledPartyID;
        BOOLEAN_T                       routeListUsed;
        CPLROUTELIST_T                  routeList;
        BOOLEAN_T                       sCFIDUsed;
        CPLGENERICNUMBER_T              sCFID;
        UCHAR_T                         nbrOfExtensions;
        EXTENSIONFIELD_T                extensions[NUMOFEXTENSIONS];
        BOOLEAN_T                       serviceInteractionIndicatorsUsed;
        CONSERVICEINTERACTIONINDICATORS_T
                                        serviceInteractionIndicators;
        BOOLEAN_T                       callingPartyNumberUsed;
        CPLNUMBER_T                     callingPartyNumber;
        BOOLEAN_T                       callingPartysCategoryUsed;
        CALLINGPARTYSCATEGORY_T         callingPartysCategory;
        BOOLEAN_T                       redirectingPartyIDUsed;
        CPLNUMBER_T                     redirectingPartyID;
        BOOLEAN_T                       redirectionInformationUsed;
        REDIRECTIONINFORMATION_T        redirectionInformation;
} CPLCONNECTARG_T;

typedef struct
{
        UCHAR_T         chooser; /* 0 = iPRoutingAddress,
                                1 = legID, 3 = none */
        union
        {
                CPLIPROUTINGADDRESS_T   iPRoutingAddress;
                RECEIVINGSIDEID_T       legID;
                UCHAR_T                 none;
        } resourceAddress;
        UCHAR_T                         nbrOfExtensions;
        EXTENSIONFIELD_T                extensions[NUMOFEXTENSIONS];
        BOOLEAN_T                       serviceInteractionIndicatorsUsed;
        CTRSERVICEINTERACTIONINDICATORS_T
                                        serviceInteractionIndicators;
} CPLCONNECTTORESOURCEARG_T;

typedef struct
{
        BOOLEAN_T                       argumentUsed;
        SENDINGSIDEID_T                 legID;
} CPLCONTINUEARG_T;

typedef struct
{
        BOOLEAN_T                       argumentUsed;
        SENDINGSIDEID_T                 legID;
} CPLDISCONNECTFORWARDCONNECTIONARG_T;

typedef struct
{
        BOOLEAN_T                       legIDUsed;
        SENDINGSIDEID_T                 legID;
        CPLGENERICNUMBER_T              assistingSSPIPRoutingAddress;
        BOOLEAN_T                       correlationIDUsed;
        CPLGENERICDIGITS_T              correlationID;
        BOOLEAN_T                       sCFIDUsed;
        CPLGENERICNUMBER_T              sCFID;
        UCHAR_T                         nbrOfExtensions;
        EXTENSIONFIELD_T                extensions[NUMOFEXTENSIONS];
        BOOLEAN_T                       serviceInteractionIndicatorsUsed;
        ETCSERVICEINTERACTIONINDICATORS_T
                                        serviceInteractionIndicators;
        BOOLEAN_T                       routeListUsed;
        CPLROUTELIST_T                  routeList;
} CPLESTABLISHTEMPORARYCONNECTIONARG_T;

typedef struct
{
        CPLEVENTTYPECHARGING_T          eventTypeCharging;
        BOOLEAN_T                       eventSpecificInfoChargingUsed;
        EVENTSPECIFICINFOCHARGING_T     eventSpecificInfoCharging;
        RECEIVINGSIDEID_T               legID;
        UCHAR_T                         nbrOfExtensions;
        EXTENSIONFIELD_T                extensions[
                                        NUMOFEXTENSIONS];
        MONITORMODE_T                   monitorMode;
} CPLEVENTNOTIFICATIONCHARGINGARG_T;

typedef struct
{
        CPLEVENTTYPEBCSM_T      eventTypeBCSM;
        BOOLEAN_T               eventSpecificInformationBCSMUsed;
        CPLEVENTSPECIFICINFORMATIONBCSM_T
                                eventSpecificInformationBCSM;
        BOOLEAN_T               legIDUsed;
        RECEIVINGSIDEID_T       legID;
        MISCCALLINFO_T          miscCallInfo;
        UCHAR_T                 nbrOfExtensions;
        EXTENSIONFIELD_T        extensions[NUMOFEXTENSIONS];
} CPLEVENTREPORTBCSMARG_T;

typedef struct
{
        SENDINGSIDEID_T                 legID;
} HOLDCALLPARTYCONNECTIONARG_T;

typedef struct
{
        SERVICEKEY_T                    serviceKey;
        BOOLEAN_T                       calledPartyNumberUsed;
        CPLNUMBER_T                     calledPartyNumber;
        BOOLEAN_T                       callingPartyNumberUsed;
        CPLNUMBER_T                     callingPartyNumber;
        BOOLEAN_T                       callingPartysCategoryUsed;
        CALLINGPARTYSCATEGORY_T         callingPartysCategory;
        BOOLEAN_T                       cGEncounteredUsed;
        CGENCOUNTERED_T                 cGEncountered;
        BOOLEAN_T                       iPSSPCapabilitiesUsed;
        CPLIPSSPCAPABILITIES_T          iPSSPCapabilities;
        BOOLEAN_T                       locationNumberUsed;
        CPLNUMBER_T                     locationNumber;
        BOOLEAN_T                       originalCalledPartyIDUsed;
        CPLNUMBER_T                     originalCalledPartyID;
        UCHAR_T                         nbrOfExtensions;
        EXTENSIONFIELD_T                extensions[NUMOFEXTENSIONS];
        BOOLEAN_T                       highLayerCompatibilityUsed;
        HIGHLAYERCOMPATIBILITY_T        highLayerCompatibility;
        BOOLEAN_T                       serviceInteractionIndicatorsUsed;
        IDPSERVICEINTERACTIONINDICATORS_T
                                        serviceInteractionIndicators;
        BOOLEAN_T                       additionalCallingPartyNumberUsed;
        CPLGENERICNUMBER_T              additionalCallingPartyNumber;
        BOOLEAN_T                       forwardCallIndicatorsUsed;
        FORWARDCALLINDICATORS_T         forwardCallIndicators;
        BOOLEAN_T                       bearerCapabilityUsed;
        CPLBEARERCAPABILITY_T           bearerCapability;
        BOOLEAN_T                       eventTypeBCSMUsed;
        CPLEVENTTYPEBCSM_T              eventTypeBCSM;
        BOOLEAN_T                       redirectingPartyIDUsed;
        CPLNUMBER_T                     redirectingPartyID;
        BOOLEAN_T                       redirectionInformationUsed;
        REDIRECTIONINFORMATION_T        redirectionInformation;
        BOOLEAN_T                       triggerTypeUsed;
        TRIGGERTYPE_T                   triggerType;
        BOOLEAN_T                       legIDsUsed;
        LEGIDS_T                        legIDs;
        BOOLEAN_T                       routeOriginUsed;
        ROUTEORIGIN_T                   routeOrigin;
        BOOLEAN_T                       testIndicationUsed;
        UCHAR_T                         testIndication;
        BOOLEAN_T                       cUGCallIndicatorUsed;
        CUGCALLINDICATOR_T              cUGCallIndicator;
        BOOLEAN_T                       cUGInterLockCodeUsed;
        CUGINTERLOCKCODE_T              cUGInterLockCode;
        BOOLEAN_T                       genericDigitsSetUsed;
        GENERICDIGITSSET_T              genericDigitsSet;
        BOOLEAN_T                       genericNumberSetUsed;
        GENERICNUMBERSET_T              genericNumberSet;
        BOOLEAN_T                       causeUsed;
        CAUSE_T                         cause;
        BOOLEAN_T                       handOverInfoUsed;
        HANDOVERINFO_T                  handOverInfo;
        BOOLEAN_T                       forwardGVNSIndicatorUsed;
        FORWARDGVNSINDICATOR_T          forwardGVNSIndicator;
        BOOLEAN_T                       backwardGVNSIndicatorUsed;
        BACKWARDGVNSINDICATOR_T         backwardGVNSIndicator;
} CPLINITIALDPARG_T;

typedef struct
{
        BOOLEAN_T                       originalCalledPartyIDUsed;
        CPLNUMBER_T                     originalCalledPartyID;
        SENDINGSIDEID_T                 legToBeCreated;
        BOOLEAN_T                       callingPartysCategoryUsed;
        CALLINGPARTYSCATEGORY_T         callingPartysCategory;
        BOOLEAN_T                       redirectingPartyIDUsed;
        CPLNUMBER_T                     redirectingPartyID;
        BOOLEAN_T                       redirectionInformationUsed;
        REDIRECTIONINFORMATION_T        redirectionInformation;
        BOOLEAN_T                       bearerCapabilitiesUsed;
        CPLBEARERCAPABILITY_T           bearerCapabilities;
        BOOLEAN_T                       cUGCallIndicatorUsed;
        CUGCALLINDICATOR_T              cUGCallIndicator;
        BOOLEAN_T                       cUGInterLockCodeUsed;
        CUGINTERLOCKCODE_T              cUGInterLockCode;
        BOOLEAN_T                       forwardCallIndicatorsUsed;
        FORWARDCALLINDICATORS_T         forwardCallIndicators;
        BOOLEAN_T                       genericDigitsSetUsed;
        GENERICDIGITSSET_T              genericDigitsSet;
        BOOLEAN_T                       genericNumberSetUsed;
        GENERICNUMBERSET_T              genericNumberSet;
        BOOLEAN_T                       highLayerCompatibilityUsed;
        HIGHLAYERCOMPATIBILITY_T        highLayerCompatibility;
        BOOLEAN_T                       forwardGVNSIndicatorUsed;
        FORWARDGVNSINDICATOR_T          forwardGVNSIndicator;
        CPLDESTINATIONROUTINGADDRESS_T  destinationRoutingAddress;
        BOOLEAN_T                       alertingPatternUsed;
        ALERTINGPATTERN_T               alertingPattern;
        UCHAR_T                         nbrOfExtensions;
        EXTENSIONFIELD_T                extensions[NUMOFEXTENSIONS];
        BOOLEAN_T                       serviceInteractionIndicatorsUsed;
        ICASERVICEINTERACTIONINDICATORS_T
                                        serviceInteractionIndicators;
        BOOLEAN_T                       callingPartyNumberUsed;
        CPLNUMBER_T                     callingPartyNumber;
        BOOLEAN_T                       routeListUsed;
        CPLROUTELIST_T                  routeList;
} CPLINITIATECALLATTEMPTARG_T;

typedef struct
{
        BOOLEAN_T                       legIDUsed;
        SENDINGSIDEID_T                 legID;
        BOOLEAN_T                       requestAnnouncementStarted;
        CPLINFORMATIONTOSEND_T          informationToSend;
        BOOLEAN_T                       disconnectFromIPForbidden;
        BOOLEAN_T                       requestAnnouncementComplete;
        UCHAR_T                         nbrOfExtensions;
        EXTENSIONFIELD_T                extensions[NUMOFEXTENSIONS];
} CPLPLAYANNOUNCEMENTARG_T;

typedef struct
{
        BOOLEAN_T                       legIDUsed;
        SENDINGSIDEID_T                 legID;
        BOOLEAN_T                       requestAnnouncementStarted;
        BOOLEAN_T                       requestAnnouncementComplete;
        CPLCOLLECTEDINFO_T              collectedInfo;
        BOOLEAN_T                       disconnectFromIPForbidden;
        BOOLEAN_T                       informationToSendUsed;
        CPLINFORMATIONTOSEND_T          informationToSend;
        UCHAR_T                         nbrOfExtensions;
        EXTENSIONFIELD_T                extensions[NUMOFEXTENSIONS];
} CPLPROMPTANDCOLLECTUSERINFORMATIONARG_T;

typedef struct
{
        UCHAR_T         chooser;        /* 0 = Digits */
        union
        {
                CPLGENERICDIGITS_T      digitsResponse;
        } u;
} CPLRECEIVEDINFORMATIONARG_T;

typedef struct
{
        SENDINGSIDEID_T                 legID;
} RECONNECTARG_T;

typedef struct
{
        SENDINGSIDEID_T                 legToBeReleased;
        BOOLEAN_T                       releaseCauseUsed;
        CAUSE_T                         releaseCause;
} RELEASECALLPARTYCONNECTIONARG_T;

typedef struct
{
        UCHAR_T                         nbrOfChrgEvents;
        CPLCHARGINGEVENT_T              chargingEvents[NUMOFCHARGINGEVENTS];
} CPLREQUESTNOTIFICATIONCHARGINGEVENTARG_T;

typedef struct
{
        UCHAR_T                 nbrOfBcsmEvents;
        CPLBCSMEVENT_T          bcsmEvents[NUMOFBCSMEVENTS];
        UCHAR_T                 nbrOfExtensions;
        EXTENSIONFIELD_T        extensions[NUMOFEXTENSIONS];
} CPLREQUESTREPORTBCSMEVENTARG_T;

typedef struct
{
        UCHAR_T                 operationID[LENOFOPERATIONID];
        APPLICATIONID_T         applicationID;
        DATAITEMID_T            dataItemID;
} RETRIEVEARG_T;


typedef struct
{
        UCHAR_T                 operationReturnID[LENOFOPERATIONRETURNID];
        CPLDATAITEMINFORMATION_T dataItemInformation;
} RETRIEVERESULTARG_T;

typedef struct
{
        CPLSCIBILLINGCHARGINGCHARACTERISTICS_T  
                                sCIBillingChargingCharacteristics;
        SENDINGSIDEID_T         legID;
        UCHAR_T                 nbrOfExtensions;
        EXTENSIONFIELD_T        extensions[NUMOFEXTENSIONS];
} CPLSENDCHARGINGINFORMATIONARG_T;

typedef struct
{
        COUNTERSVALUE_T         countersValue;
        CPLFILTERINGCRITERIA_T  filteringCriteria;
        UCHAR_T                 nbrOfExtensions;
        EXTENSIONFIELD_T        extensions[NUMOFEXTENSIONS];
        RESPONSECONDITION_T     responseCondition;
        BOOLEAN_T               sCFCorrelationInfoUsed;
        UCHAR_T                 sCFCorrelationInfo[LENOFSCFCORRINFO];
} CPLSERVICEFILTERINGRESPONSEARG_T;

typedef struct
{
        BOOLEAN_T                       backwardSuppressionIndicatorsUsed;
        BACKWARDSUPPRESSIONINDICATORS_T backwardSuppressionIndicators;
        BOOLEAN_T                       connectedNumberUsed;
        CPLNUMBER_T                     connectedNumber;
        BOOLEAN_T                       forwardSuppressionIndicatorsUsed;
        FORWARDSUPPRESSIONINDICATORS_T  forwardSuppressionIndicators;
        BOOLEAN_T                       backwardGVNSIndicatorUsed;
        BACKWARDGVNSINDICATOR_T         backwardGVNSIndicator;
        UCHAR_T                         nbrOfExtensions;
        EXTENSIONFIELD_T                extensions[NUMOFEXTENSIONS];
} SIGNALLINGINFORMATIONARG_T;

typedef struct
{
        UCHAR_T         chooser; /* 0 = announcementCompleted,
                                1 = announcementStarted */
        union
        {
                UCHAR_T         announcementCompleted;
                UCHAR_T         announcementStarted;
        } u;
} CPLSPECIALIZEDRESOURCEREPORTARG_T;

typedef struct
{
        UCHAR_T                         operationID[LENOFOPERATIONID];
        APPLICATIONID_T                 applicationID;
        DATAITEMID_T                    dataItemID;
        CPLDATAITEMINFORMATION_T        dataItemInformation;
} UPDATEARG_T;

typedef struct
{
        UCHAR_T         chooser; /*     0 = operationReturnID */
        union
        {
                UCHAR_T         operationReturnID[LENOFOPERATIONRETURNID];
        } u;
} UPDATERESULTARG_T;

#if defined __cplusplus || defined c_plusplus
}
#endif
#endif /* __INAPCS1PLUSDEFAPI_H__ */

