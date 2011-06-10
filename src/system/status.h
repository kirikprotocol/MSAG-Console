#ifndef __SMSC_SYSTEM_STATUS_H__
#define __SMSC_SYSTEM_STATUS_H__

namespace smsc{
namespace system{
namespace Status{

static const int OK=0; //No Error
static const int INVMSGLEN=1; //Message Length is invalid
static const int INVCMDLEN=2; //Command Length is invalid
static const int INVCMDID=3; //Invalid Command ID
static const int INVBNDSTS=4; //Incorrect BIND Status for given command
static const int ALYBND=5; //ESME Already in Bound State
static const int INVPRTFLG=6; //Invalid Priority Flag
static const int INVREGDLVFLG=7; //Invalid Registered Delivery Flag
static const int SYSERR=8; //System Error
static const int INVSRCADR=10; //Invalid Source Address
static const int INVDSTADR=11; //Invalid Dest Addr
static const int INVMSGID=12; //Message ID is invalid
static const int BINDFAIL=13; //Bind Failed
static const int INVPASWD=14; //Invalid Password
static const int INVSYSID=15; //Invalid System ID
static const int CANCELFAIL=17; //Cancel SM Failed
static const int REPLACEFAIL=19; //Replace SM Failed
static const int MSGQFUL=20; //Message Queue Full
static const int INVSERTYP=21; //Invalid Service Type
static const int INVNUMDESTS=51; //Invalid number of destinations
static const int INVDLNAME=52; //Invalid Distribution List name
static const int INVDESTFLAG=64; //Destination flag is invalid (submit_multi)
static const int INVSUBREP=66; //Invalid 'submit with replace' request (i.e.      submit_sm with replace_if_present_flag set)
static const int INVESMCLASS=67; //Invalid esm_class field data
static const int CNTSUBDL=68; //Cannot Submit to Distribution List
static const int SUBMITFAIL=69; //submit_sm or submit_multi failed
static const int INVSRCTON=72; //Invalid Source address TON
static const int INVSRCNPI=73; //Invalid Source address NPI
static const int INVDSTTON=80; //Invalid Destination address TON
static const int INVDSTNPI=81; //Invalid Destination address NPI
static const int INVSYSTYP=83; //Invalid system_type field
static const int INVREPFLAG=84; //Invalid replace_if_present flag
static const int INVNUMMSGS=85; //Invalid number of messages
static const int THROTTLED=88; //Throttling error (ESME has exceeded allowed message limits)
static const int INVSCHED=97; //Invalid Scheduled Delivery Time
static const int INVEXPIRY=98; //Invalid message validity period (Expiry time)
static const int INVDFTMSGID=99; //Predefined Message Invalid or Not Found
static const int RX_T_APPN=100; //ESME Receiver Temporary App Error Code
static const int RX_P_APPN=101; //ESME Receiver Permanent App Error Code
static const int RX_R_APPN=102; //ESME Receiver Reject Message Error Code
static const int QUERYFAIL=103; //query_sm request failed
static const int INVOPTPARSTREAM=192; //Error in the optional part of the PDU Body
static const int OPTPARNOTALLWD=193; //Optional Parameter not allowed
static const int INVPARLEN=194; //Invalid Parameter Length
static const int MISSINGOPTPARAM=195; //Expected Optional Parameter missing
static const int INVOPTPARAMVAL=196; //Invalid Optional Parameter Value
static const int DELIVERYFAILURE=254; //Delivery Failure (used for data_sm_resp)
static const int UNKNOWNERR=255; //Unknown Error
static const int PROHIBITED=257; //Prohibited
static const int INVDCS=260; //Invalid Data Coding Scheme Specified DCS is invalid or MC does not support it.

static const int SMSC_ERR_BASE=1024;
static const int NOROUTE=1025; //No route defined for given source and destination addresses !
static const int EXPIRED=1026; //Message expired !
static const int DELIVERYTIMEDOUT=1027; //Delivery attempt timed out
static const int SMENOTCONNECTED=1028; //Sme is not connected
static const int BLOCKEDMSC=1029; //Msc is blocked !
static const int DELETED=1030; //Message deleted !
static const int USSDDLGNFOUND=1031; // USSD dialog not found for response (PSSR_RESP) !
static const int USSDMSGTOOLONG=1032; // USSD msg can't be concatenated or truncated !
static const int INCOMPLETECONCATMSG=1033; //not all parts of concatenated message received !
static const int DUPLICATECONCATPART=1034; //duplicate part of concatenated message !
static const int TRANSACTIONTIMEDOUT=1035; //
static const int LOCKEDBYMO=1036; //MT should wait MO
static const int LICENSELIMITREJECT=1037;//sms rejected due to license limit
static const int USSDDLGREFMISM=1038; // USSD dialog exists but reference mismatch (PSSR_RESP) !
static const int DENIEDBYINMAN=1039; // inman return charge resp with CHARGE NOT POSSIBLE
static const int DEFERREDDELIVERY=1040; // deferred sms with charge on delivery policy
static const int DENIEDBYCLOSEDGROUP=1041; // member of closed group tried to send sms to restricted address
static const int DENIEDBYACCESSMASK=1042;  // access mask prohibited submit
static const int NOCONNECTIONTOINMAN=1043; // failed to connect to inman
static const int MSMEMCAPACITYFULL=1044;  // mobile station cannot receive sms due to memory limit
static const int MAPINTERNALFAILURE=1045;  // map internal exception
static const int SCHEDULERLIMIT=1046;     //either soft or hard scheduler limit reached
static const int DPFSMENOTCONNECTED=1047; //failed to send alert notification, sme not connected. pseudoerror.
static const int BACKUPSMERESCHEDULE=1048; //(noresp|blockedmsc)+(backupsme on route)=>this error
static const int MISSINGIMSIINNIUSSD=1049; //dest number requires imsi as dest ref, but imsi is empty
static const int DENIEDBYGLOBALBL=1050; // denied by global black list (informer)
static const int DENIEDBYLOCALBL=1051;  // denied by local black list (informer)
static const int EXPIREDATSTART=1052;   // msg is expired at start of informer (msgid lost?)

static const int MAP_UR_NO_REASON=1109;
static const int MAP_USER_REASON_BASE=1110;
static const int MAP_UR_RESOURCE_LIM_T=1110;
static const int MAP_UR_RESOURCE_UNAVAIL=1111;
static const int MAP_UR_APPL_PROC_CANCEL=1112;
static const int MAP_UR_PROC_ERROR=1113;

static const int MAP_PROVIDER_REASON_BASE=1120;
static const int MAP_ABNORMAL_DIALOGUE = 1120;
static const int MAP_PROVIDER_MALFUNCTION = 1121;
static const int MAP_TRANSACTION_RELEASED = 1122;
static const int MAP_RESOURCE_LIM_T = 1123;
static const int MAP_MAINTENANCE_ACTIVITY = 1124;
static const int MAP_VERSION_INCOMPATIBILITY = 1125;
static const int MAP_DIALOGUE_TIMEOUT = 1126;

static const int MAP_REFUSE_REASON_BASE=1128;
static const int MAP_REFUSE_REASON_NO_REASON=1128; // !
static const int MAP_REFUSE_REASON_INV_DEST_REF=1129; // !
static const int MAP_REFUSE_REASON_INV_ORIG_REF=1130; // !
static const int MAP_REFUSE_REASON_APP_CONTEXT_NOT_SUPP=1131; // !
static const int MAP_REFUSE_REASON_NODE_NOT_REACHABLE=1132; // !
static const int MAP_REFUSE_REASON_VERS_INCOMPATIBLE=1133; // !

static const int RESCHEDULEDNOW=1134; // not error, message was rescheduled for immediate delivery

static const int MAP_PROVIDER_ERR_BASE=1135;
static const int MAP_DUPLICATED_INVOKE_ID=1136;
static const int MAP_NOT_SUPPORTED_SERVICE=1137; // !
static const int MAP_MISTYPED_PARAMETER=1138;
static const int MAP_RESOURCE_LIMITATION=1139;
static const int MAP_INITIATING_RELEASE=1140;
static const int MAP_UNEXPECTED_RESPONSE_FROM_PEER=1141;  // !
static const int MAP_SERVICE_COMPLETION_FAILURE=1142;
static const int MAP_NO_RESPONSE_FROM_PEER=1143;
static const int MAP_INVALID_RESPONSE_RECEIVED=1144; // !
static const int MAP_UNEXPECTED_TRANSACTION_ID=1145;
static const int MAP_UNRECOGNIZED_COMPONENT=1146; // !
static const int MAP_MISTYPED_COMPONENT=1147; // !
static const int MAP_BADSTRUCTURED_COMPONENT=1148; // !
static const int MAP_UNRECOGNIZED_LINKED_ID=1149; // !
static const int MAP_LINKED_RESPONSE_UNEXPECTED=1150; // !
static const int MAP_UNEXPECTED_LINKED_OPERATION=1151; // !

static const int MAP_ERR_BASE=1152;
static const int UNKSUBSCRIBER=1153; // 1 - Unknown subscriber !
static const int MAPINVDESTREF=1154; // 2 - !
static const int MAPINVORIGREF=1155; // 3 - !
static const int UNDEFSUBSCRIBER=1157; // 5 - Unidentified subscriber
static const int ABSENTSUBSCRIBERSM=1158; // 6 - Absent subscriber SM
static const int ROAMINGNOTALWD=1160; // 8
static const int ILLEGALSUBSCRIBER=1161; // 9 - Illegal subscriber !
static const int TELSVCNOTPROVIS=1163; // 12 - Teleservice not provisioned
static const int INVEQUIPMENT=1164; // 12 - Illegal Equipment !
static const int CALLBARRED=1165; // 13 - Call Barred
static const int CUGREJECT=1167; // 15 - CUG reject
static const int FACILITYNOTSUPP=1173; // 21 - Facility Not Supported
static const int ABSENTSUBSCR=1179; // 27 - Absent Subscriber
static const int SUBSCRBUSYMT=1183; //Subscriber Busy for MT SMS.
static const int SMDELIFERYFAILURE=1184; //SM Delivery Failure
static const int MSGWLISTFULL=1185; //
static const int SYSFAILURE=1186; // 34 - System Failure
static const int DATAMISSING=1187; // 35 - Data Missing !
static const int UNEXPDATA=1188; // 36 - Unexpected Data value
static const int USSDSESSIONTERMABN=1217;// smpp+ error
static const int UNKALPHABET=1223; // 71 - Unknown alphabet !
static const int USSDBUSY=1224; // 72 - USSD busy
static const int INVPRBRESLIM=1258; // 106 - Invoke problem. Resource limitation
static const int INVPRBINITREL=1259; // 107 - Invoke problem. Initiating release

static const bool errorStatus[1280] = {
0, //No Error
1, //Message Length is invalid
1, //Command Length is invalid
1, //Invalid Command ID
0, //Incorrect BIND Status for given command
1, //ESME Already in Bound State
1, //Invalid Priority Flag
1, //Invalid Registered Delivery Flag
0, //System Error
1, //reserved
1, //Invalid Source Address
1, //Invalid Dest Addr
1, //Message ID is invalid
1, //Bind Failed
1, //Invalid Password
1, //Invalid System ID
1, //reserved
1, //Cancel SM Failed
1, //reserved
1, //Replace SM Failed
0, //Message Queue Full
1, //Invalid Service Type
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //Invalid number of destinations
1, //Invalid Distribution List name
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //Destination flag is invalid (submit_multi)
1, //reserved
1, //Invalid 'submit with replace' request (i.e.      submit_sm with replace_if_present_flag set)
1, //Invalid esm_class field data
1, //Cannot Submit to Distribution List
1, //submit_sm or submit_multi failed
1, //reserved
1, //reserved
1, //Invalid Source address TON
1, //Invalid Source address NPI
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //Invalid Destination address TON
1, //Invalid Destination address NPI
1, //reserved
1, //Invalid system_type field
1, //Invalid replace_if_present flag
1, //Invalid number of messages
1, //reserved
1, //reserved
0, //Throttling error (ESME has exceeded allowed message limits)
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //Invalid Scheduled Delivery Time
1, //Invalid message validity period (Expiry time)
1, //Predefined Message Invalid or Not Found
0, //ESME Receiver Temporary App Error Code
1, //ESME Receiver Permanent App Error Code
1, //ESME Receiver Reject Message Error Code
1, //query_sm request failed
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //Error in the optional part of the PDU Body
1, //Optional Parameter not allowed
1, //Invalid Parameter Length
1, //Expected Optional Parameter missing
1, //Invalid Optional Parameter Value
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //Delivery Failure (used for data_sm_resp)
1, //Unknown Error
1, //reserved
1, //Prohibited
1, //reserved
1, //reserved
1, //Invalid Data Coding Scheme Specified DCS is invalid or MC does not support it.
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, //reserved
1, // 272 - 1023 reserved
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1, // reserved 32
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1, // reserved 32
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1, // reserved 32
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1, // reserved 32
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1, // reserved 32
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1, // reserved 32
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1, // reserved 32
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1, // reserved 32
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1, // reserved 32
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1, // reserved 32
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1, // reserved 32
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1, // reserved 32
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1, // reserved 32
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1, // reserved 32
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1, // reserved 32
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1, // reserved 32
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1, // reserved 32
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1, // reserved 32
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1, // reserved 32
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1, // reserved 32
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1, // reserved 32
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1, // reserved 32
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1, // reserved 32
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1, // reserved 16
1, // SMSC vendor codes 1024-1279, SMSC_ERR_BASE
1, // NOROUTE=1025; No route defined for given source and destination addresses !
1, // EXPIRED=1026; //Message expired !
0, // DELIVERYTIMEDOUT=1027; //Delivery attempt timed out
0, // SMENOTCONNECTED=1028; //Sme is not connected
1, // BLOCKEDMSC=1029; //Msc is blocked !
1, // DELETED=1030; //Message deleted !
1, // USSDDLGNFOUND=1031; // USSD dialog not found for response (PSSR_RESP) !
1, // USSDMSGTOOLONG=1032; // USSD msg can't be concatenated or truncated !
1, // INCOMPLETECONCATMSG=1033; //not all parts of concatenated message received !
1, // DUPLICATECONCATPART=1034; //duplicate part of concatenated message !
1, // TRANSACTIONTIMEDOUT=1035
0, // LOCKEDBYMO=1036
0, // LICENSELIMITREJECT=1037
1, // USSDDLGREFMISM=1038
1, // DENIEDBYINMAN=1039
0, // DEFERREDDELIVERY=1040
1, // DENIEDBYCLOSEDGROUP=1041
1, // DENIEDBYACCESSMASK=1042
0, // NOCONNECTIONTOINMAN=1043
0, // MSMEMCAPACITYFULL=1044
0, // MAPINTERNALFAILURE=1045
1, // SCHEDULERLIMIT=1046
0, // DPFSMENOTCONNECTED=1047
0, // BACKUPSMERESCHEDULE=1048
1, // MISSINGIMSIINNIUSSD=1049
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1, // reserved 32
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
1,
0, // MAP_UR_NO_REASON=1109
0, // MAP_USER_REASON_BASE=1110, MAP_UR_RESOURCE_LIM_T=1110;
0, // MAP_UR_RESOURCE_UNAVAIL=1111
0, // MAP_UR_APPL_PROC_CANCEL=1112
0, // MAP_UR_PROC_ERROR=1113
1,
1,
1,
1,
1,
1,
0, // MAP_PROVIDER_REASON_BASE, MAP_ABNORMAL_DIALOGUE = 1120
0, // MAP_PROVIDER_MALFUNCTION = 1121
0, // MAP_TRANSACTION_RELEASED = 1122
0, // MAP_RESOURCE_LIM_T = 1123
0, // MAP_MAINTENANCE_ACTIVITY = 1124
0, // MAP_VERSION_INCOMPATIBILITY = 1125
0, // MAP_DIALOGUE_TIMEOUT = 1126
1, // reserved
0, // MAP_REFUSE_REASON_BASE, MAP_REFUSE_REASON_NO_REASON=1128
1, // MAP_REFUSE_REASON_INV_DEST_REF=1129
1, // MAP_REFUSE_REASON_INV_ORIG_REF=1130
1, // MAP_REFUSE_REASON_APP_CONTEXT_NOT_SUPP=1131
1, // MAP_REFUSE_REASON_NODE_NOT_REACHABLE=1132
1, // MAP_REFUSE_REASON_VERS_INCOMPATIBLE=1133
0, // RESCHEDULEDNOW=1134 not error, message was rescheduled for immediate delivery
0, // MAP_PROVIDER_ERR_BASE=1135
0, // MAP_DUPLICATED_INVOKE_ID=1136
1, // MAP_NOT_SUPPORTED_SERVICE=1137
0, // MAP_MISTYPED_PARAMETER=1138
0, // MAP_RESOURCE_LIMITATION=1139
0, // MAP_INITIATING_RELEASE=1140;
1, // MAP_UNEXPECTED_RESPONSE_FROM_PEER=1141
0, // MAP_SERVICE_COMPLETION_FAILURE=1142
0, // MAP_NO_RESPONSE_FROM_PEER=1143
0, // MAP_INVALID_RESPONSE_RECEIVED=1144
0, // MAP_UNEXPECTED_TRANSACTION_ID=1145
1, // MAP_UNRECOGNIZED_COMPONENT=1146
1, // MAP_MISTYPED_COMPONENT=1147
1, // MAP_BADSTRUCTURED_COMPONENT=1148
1, // MAP_UNRECOGNIZED_LINKED_ID=1149
1, // MAP_LINKED_RESPONSE_UNEXPECTED=1150
1, // MAP_UNEXPECTED_LINKED_OPERATION=1151
0, // MAP_ERR_BASE=1152
1, // UNKSUBSCRIBER=1153 1 - Unknown subscriber
1, // INVDESTREF=1154
1, // INVORIGREF=1155
1, // reserved
0, // UNDEFSUBSCRIBER=1157 2 - Unidentified subscriber
0, // ABSENTSUBSCRIBERSM=1158 6 - Absent subscriber SM
1, // reserved
0, // ROAMINGNOTALWD=1160
1, // ILLEGALSUBSCRIBER=1161 9 - Illegal subscriber
1, // reserved
0, // TELSBVCNOTPROVIS=1163
1, // INVEQUIPMENT=1164 12 - Illegal Equipment !
0, // CALLBARRED=1165 13 - Call Barred
1, // reserved
0, // CUGREJECT=1167 15 - CUG reject
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // FACILITYNOTSUPP=1173 21 - Facility Not Supported
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
0, // ABSENTSUBSCR=1179 27 - Absent Subscriber
1, // reserved
1, // reserved
1, // reserved
0, // SUBSCRBUSYMT=1183 Subscriber Busy for MT SMS.
0, // SMDELIFERYFAILURE=1184; //SM Delivery Failure
0, // MSGWLISTFULL=1185
0, // SYSFAILURE=1186 34 - System Failure
1, // DATAMISSING=1187 35 - Data Missing !
0, // UNEXPDATA=1188 36 - Unexpected Data value
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // 1217 USSDSESSIONTERMABN
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // UNKALPHABET=1223 71 - Unknown alphabet
0, // USSDBUSY=1224 72 - USSD busy
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
0, // INVPRBRESLIM=1258 106 - Invoke problem. Resource limitation
0, // INVPRBINITREL=1259 107 - Invoke problem. Initiating release
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1, // reserved
1 // reserved
};

/* function return 0 if error is temp, and 1 if perm */
static inline bool isErrorPermanent( int err ) {
  if( err > 0x4ff ) return 1;
  return errorStatus[err];
}

}//Status
}//system
}//smsc

#endif
