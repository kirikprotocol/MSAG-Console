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
static const int INVDCS=260; //Invalid Data Coding Scheme Specified DCS is invalid or MC does not support it.

static const int SMSC_ERR_BASE=1024;
static const int NOROUTE=1025; //No route defined for given source and destination addresses
static const int EXPIRED=1026; //Message expired
static const int DELIVERYTIMEDOUT=1027; //Delivery attempt timed out
static const int SMENOTCONNECTED=1028; //Sme is not connected
static const int BLOCKEDMSC=1029; //Msc is blocked

static const int MAP_PROVIDER_ERR_BASE=1135;
static const int MAP_DUPLICATED_INVOKE_ID=1136;
static const int MAP_NOT_SUPPORTED_SERVICE=1137;
static const int MAP_MISTYPED_PARAMETER=1138;
static const int MAP_RESOURCE_LIMITATION=1139;
static const int MAP_INITIATING_RELEASE=1140;
static const int MAP_UNEXPECTED_RESPONSE_FROM_PEER=1141;
static const int MAP_SERVICE_COMPLETION_FAILURE=1142;
static const int MAP_NO_RESPONSE_FROM_PEER=1143;
static const int MAP_INVALID_RESPONSE_RECEIVED=1144;
static const int MAP_UNEXPECTED_TRANSACTION_ID=1145;
static const int MAP_UNRECOGNIZED_COMPONENT=1146;
static const int MAP_MISTYPED_COMPONENT=1147;
static const int MAP_BADSTRUCTURED_COMPONENT=1148;
static const int MAP_UNRECOGNIZED_LINKED_ID=1149;
static const int MAP_LINKED_RESPONSE_UNEXPECTED=1150;
static const int MAP_UNEXPECTED_LINKED_OPERATION=1151;

static const int MAP_ERR_BASE=1152;
static const int UNKSUBSCRIBER=1153; //Unknown subscriber
static const int UNDEFSUBSCRIBER=1157; //Unidentified subscriber
static const int ILLEGALSUBSCRIBER=1161; //Illegal subscriber
static const int INVEQUIPMENT=1164; //Illegal Equipment
static const int CALLBARRED=1165; //Call Barred
static const int FACILITYNOTSUPP=1173; //Facility Not Supported
static const int ABSENTSUBSCR=1179; //Absent Subscriber
static const int SUBSCRBUSYMT=1183; //Subscriber Busy for MT SMS.
static const int SMDELIFERYFAILURE=1184; //SM Delivery Failure
static const int SYSFAILURE=1186; //System Failure
static const int DATAMISSING=1187; //Data Missing
static const int UNEXPDATA=1188; //Unexpected Data value

};//Status
};//system
};//smsc

#endif
