#include <alloca.h>
#include <vector>
#include "logger/Logger.h"
#include "util.hpp"

namespace smsc{
namespace misscall{
namespace util{

using smsc::logger::Logger;
using namespace std;
std::string dump(USHORT_T len, UCHAR_T* udp) {
  //char *text = new char[len*3+1];
  char* text = (char*)alloca(len*3+1);
  int k = 0;
  for ( int i=0; i<len; i++){
    k+=sprintf(text+k,"%02X ",udp[i]);
  }
  text[k]=0;
  return string(text);
}
std::string getExtraOptionsDescription(EINSS7_I97_OPTPARAMS_T *ext)
{
  if(!ext) return "";

  string res = "ExtOptions=(";

  for(int i = 0; i < ext->noOfParams; i++)
  {
    UCHAR_T* param = ext->params[i];
    if (param)
    {
      char buf[4]; int k;
      int len = param[1] + 2;

      for (int j = 0; j < len; j++)
      {
        k = sprintf(buf,"%02X ",param[j]);
        buf[k] = 0;
        res +=buf;
      }
      if(ext->noOfParams-i-1 > 0) res +=",";
    }
  }
  res +=")";
  return res;
}
void pack_addr(UCHAR_T* dst, const char* src, int len)
{
  for( int i = 0; i < len; i++ )
  {
    if( i & 1 )
    {
      dst[i/2] |= ((src[i]-'0')<<4); // fill high semioctet
    }
    else
    {
      dst[i/2] = (src[i]-'0')&0x0f; // fill low semioctet
    }
  }
  if( len & 1 ) {
    dst[(len+1)/2-1] &= 0x0F;
  }
}
std::string getCalledNumberDescription(EINSS7_I97_CALLEDNUMB_T* called)
{
  if (!called) return "";
  vector<char> addr(called->noOfAddrSign + 1);
  unpack_addr(&addr[0], called->addrSign_p, called->noOfAddrSign);
  addr.insert(addr.begin(),'.');
  addr.insert(addr.begin(),called->numberPlan + '0');
  addr.insert(addr.begin(),'.');
  addr.insert(addr.begin(),called->natureOfAddr + '0');
  addr.insert(addr.begin(),'.');
  char str[]="Called=";
  char *end=str+sizeof(str)-1;
  addr.insert(addr.begin(),str,end);
  return &addr[0];
}

std::string getCallingNumberDescription(EINSS7_I97_CALLINGNUMB_T* calling)
{
  if (!calling) return "";
  string res="Calling=(";
  res += "PRES=";
  switch (calling->presentationRestr) {
    case EINSS7_I97_PRES_ALLOWED   : res += "ALLOWED";break;
    case EINSS7_I97_PRES_RESTR     : res += "RESTRICTED";break;
    case EINSS7_I97_ADDR_NOT_AVAIL : res += "NOT AVAIL";break;
    default                        : res += "UNKNOWN";break;
  }
  res += ",SCREEN=";
  switch (calling->screening) {
    case EINSS7_I97_USER_PROV : res += "USER"; break;
    case EINSS7_I97_NETW_PROV : res += "NET"; break;
    default                   : res += "UNKNOWN"; break;
  }
  switch (calling->numbIncompl) {
    case EINSS7_I97_NUMB_COMPL   : res += ",COMPLETE"; break;
    case EINSS7_I97_NUMB_INCOMPL : res += ",INCOMPLETE"; break;
  }
  char addr[32] = {0,}; int k;
  k = sprintf(addr,".%d.%d.",calling->natureOfAddr,calling->numberPlan); addr[k] = 0;
  unpack_addr(addr+k, calling->addrSign_p, calling->noOfAddrSign);
  res += ",AI=";res += addr; res += ")";
  return res;
}

std::string getRedirectingNumberDescription(EINSS7_I97_REDIRECTINGNUMB_T* redirNumb)
{
  if (!redirNumb) return "";
  vector<char> addr(redirNumb->noOfAddrSign + 1);
  unpack_addr(&addr[0], redirNumb->addrSign_p, redirNumb->noOfAddrSign);
  addr.insert(addr.begin(),'.');
  addr.insert(addr.begin(),redirNumb->numberPlan + '0');
  addr.insert(addr.begin(),'.');
  addr.insert(addr.begin(),redirNumb->natureOfAddr + '0');
  addr.insert(addr.begin(),'.');
  char str[]="RedirectingNumber=";
  char *end=str+sizeof(str)-1;
  addr.insert(addr.begin(),str,end);
  return &addr[0];
}
std::string getRedirectionNumberDescription(EINSS7_I97_REDIRECTIONNUMB_T* redirectionNumb_sp)
{
  if (!redirectionNumb_sp) return "";
  vector<char> addr(redirectionNumb_sp->noOfAddrSign + 1);
  unpack_addr(&addr[0], redirectionNumb_sp->addrSign_p, redirectionNumb_sp->noOfAddrSign);
  char str[]="RedirectionNumber=";
  char *end=str+sizeof(str)-1;
  addr.insert(addr.begin(),str,end);
  return &addr[0];
}

std::string getOriginalNumberDescription(EINSS7_I97_ORIGINALNUMB_T* origNumb)
{
  if (!origNumb) return "";
  vector<char> addr(origNumb->noOfAddrSign + 1);
  unpack_addr(&addr[0], origNumb->addrSign_p, origNumb->noOfAddrSign);
  addr.insert(addr.begin(),'.');
  addr.insert(addr.begin(),origNumb->numberPlan + '0');
  addr.insert(addr.begin(),'.');
  addr.insert(addr.begin(),origNumb->natureOfAddr + '0');
  addr.insert(addr.begin(),'.');
  char str[]="Original=";
  char *end=str+sizeof(str)-1;
  addr.insert(addr.begin(),str,end);
  return &addr[0];
}

std::string getTypeOfServiceDescription(UCHAR_T typeOfService) {
  switch(typeOfService) {
    case 0:  return "End Of Service Request (EOSR)";
    case 1:  return "SysLog Info Service (SLIS)";
    case 2:  return "Reserved for future use";
    case 3:  return "Service Request Failed (SRF)";
    case 4:  return "Reserved for future use";
    case 5:  return "Reserved for future use";
    case 6:  return "Configuration Query (CONQ)";
    case 7:  return "Configuration Index Query (CIQ)";
    case 8:  return "System Trace Query (SYSTRCQ)";
    case 9:  return "Stack State Query (SSQ)";
    case 10: return "Set Config Info Query (SCIQ)";
    case 11: return "Get Config Info Query (GCIQ)";
    case 12: return "Read Binary Data (RBD)";
    case 13: return "Write Binary Data (WBD)";
  }
}

std::string getRedirectionInfoDescription(EINSS7_I97_REDIRECTIONINFO_T* redirectionInfo)
{
  if (!redirectionInfo) return "";
  string res = "Redirection indicator=";
  switch(redirectionInfo->redirecting)
  {
    case EINSS7_I97_NO_RED            : res += "No redirection";break;
    case EINSS7_I97_CALL_RER          : res += "Call rerouted";break;
    case EINSS7_I97_CALL_RER_RESTR    : res += "Call rerouted, all redirection information presentation restricted";break;
    case EINSS7_I97_CALL_FORW         : res += "Call forwarded (diverted)";break;
    case EINSS7_I97_CALL_FORW_RESTR   : res += "Call forwarded (diverted), all redirection information presentation restricted";break;
    case EINSS7_I97_CALL_RER_RED_RES  : res += "Call rerouted, redirection number presentation restricted";break;
    case EINSS7_I97_CALL_FORW_RED_RES : res += "Call forwarded (diverted), redirection number presentation restricted";break;
  }
  res += " OrigReason=";
  switch(redirectionInfo->origReason)
  {
    case EINSS7_I97_REASON_UNKNOWN : res += "Unknown/not available";break;
    case EINSS7_I97_USER_BUSY      : res += "User busy";break;
    case EINSS7_I97_NO_REPLY       : res += "No reply";break;
    case EINSS7_I97_UNCOND         : res += "Unconditional";break;
  }
  res += " RedirectCount=";
  char buf[4];
  int k;
  k = sprintf(buf,"%d",redirectionInfo->counter);
  buf[k] = 0;
  res += buf;
  res += " LastReason=";
  switch(redirectionInfo->lastReason)
  {
    case EINSS7_I97_REASON_UNKNOWN  : res += "Unknown/not available";break;
    case EINSS7_I97_USER_BUSY       : res += "User busy";break;
    case EINSS7_I97_NO_REPLY        : res += "No reply";break;
    case EINSS7_I97_UNCOND          : res += "Unconditional";break;
    case EINSS7_I97_DEF_ALERT       : res += "Deflection during alerting";break;
    case EINSS7_I97_DEF_RESP        : res += "Deflection immediate response";break;
    case EINSS7_I97_MOB_NOT_REACHED : res += "Mobile subscriber not reachable";break;
  }
  return res;
}

void unpack_addr(char* dst, UCHAR_T* src, int len)
{
  UCHAR_T sign;
  for (int i = 0; i < len; i++)
  {
    if (i & 1)
    {
      sign = (src[i/2] >> 4) & 0x0f;
    }
    else
    {
      sign = src[i/2] & 0x0f;
    }
    switch(sign)
    {
      case 0: case 1: case 2: case 3: case 4: case 5:
      case 6: case 7: case 8: case 9: dst[i] = sign + '0'; break;
      case 11: dst[i] = '*'; break;
      case 12: dst[i] = '#'; break;
      case 15: dst[i] =  0 ; break;
      default: dst[i] = '?'; break;
    }
  }
  dst[len] = 0;
}
std::string getHeadDescription(EINSS7_I97_ISUPHEAD_T* isupHead_sp)
{
  char buf[32];
  buf[0] = 0;
  if (isupHead_sp)
  {
    snprintf(buf,
             sizeof(buf),
             "CID=%d HSN=%d SPN=%d TS=%d",
             isupHead_sp->connectionId,
             isupHead_sp->hsn,
             isupHead_sp->span,
             isupHead_sp->timeslot);
  }
  return buf;
}
std::string getInstanceDescription(EINSS7INSTANCE_T isupInstanceId)
{
  char buf[32];
  buf[0] = 0;
  snprintf(buf,sizeof(buf),"BEID=%d",isupInstanceId);
  return buf;
}
const char* getResultDescription(UCHAR_T result)
{
  switch(result) {
    case 0: return "Success";
    case 1: return "File not found";
    case 2: return "File could not be opened";
    case 3: return "Order could not be performed";
    case 4: return "Bad configuration";
    case 5: return "Statistics ID not implemented";
    case 6: return "Order ID not implemented";
    case 7: return "Alarm ID not implemented";
    case 8: return "Format error";
    case 9: return "Resources Unavailable";
    case 10: return "SSN has not made a bind";
    case 11: return "Illegal state";
    case 12: return "MM user is already bound";
  }
}
const char* getInstanceTypeDescription(EINSS7OAMAPI_INSTANCETYPE_T type)
{
  switch (type) {
    case EINSS7OAMAPI_BE: return "BE";
    case EINSS7OAMAPI_FE:return "FE";
    case EINSS7OAMAPI_NMP: return "NMP";
    case EINSS7OAMAPI_ALL_BE: return "ALL_BE";
    case EINSS7OAMAPI_ALL_FE: return "ALL_FE";
    case EINSS7OAMAPI_ALL: return "ALL";
    case EINSS7OAMAPI_SM: return "ECM";
    case EINSS7OAMAPI_ALL_SM: return "ALL_ECM";
    case EINSS7OAMAPI_OAM: return "OAM";
    default: return "UNKNOWN";
  }
}
const char* getStackStatusDescription(UCHAR_T mmState)
{
  switch (mmState) {
    case 0: return "IDLE";
    case 1: return "INITIALIZING";
    case 2: return "INITIALIZED";
    case 3: return "STARTING";
    case 4: return "RUNNING";
    case 5: return "TERMINATING";
    case 6: return "TERMINATED";
    case 7: return "PASSIVE";
    case 8: return "HA-TRAFFIC-BLOCKED";
  }
}
const char* getCongLevelDescription(UCHAR_T congLevel)
{
  switch(congLevel)
  {
    case EINSS7_I97_NO_CONG: return "Available";
    case EINSS7_I97_CONG_1: return "Level 1";
    case EINSS7_I97_CONG_2: return "Level 2";
    case EINSS7_I97_CONG_3: return "Level 3";
    case EINSS7_I97_UNAVAIL_PAUSE: return "Unavailable, due to MTP_PAUSE";
    case EINSS7_I97_UNAVAIL_INACC: return "Unavailable, due to \"inaccessible remote user\"";
    case EINSS7_I97_UNAVAIL_UNEQUIP: return "Unavailable, due to \"unequipped remote user\"";
  }
}
std::string getAutoCongLevelDescription(UCHAR_T* autoCongestLevel)
{
  if(!autoCongestLevel) return "";
  string res = "Auto Congestion level=(";
  switch(*autoCongestLevel)
  {
    case EINSS7_I97_ACC_CONG_1 : res += "level 1 exceeded";break;
    case EINSS7_I97_ACC_CONG_2 : res += "level 2 exceeded";break;
    default                    : res += "UNKNOWN"; break;
  }
  res +=")";
  return res;
}

const char* getIsupBindStatusDescription(UCHAR_T result)
{
  switch (result)
  {
    case EINSS7_I97_BIND_OK: return "Call Control connected";
    case EINSS7_I97_ANOTHER_CC_BOUND: return "Another Call Control was bound";
    case EINSS7_I97_SAME_CC_BOUND: return "Same Call Control already bound";
  }
}
std::string getResponseIndicatorDescription(UCHAR_T responseInd)
{
  string res = "Response indicator=(";
  switch(responseInd & 0x01)
  {
    case 0x00  : res += "NOT required";break;
    case 0x01  : res += "required";break;
  }
  res += ")";
  return res;
}
std::string getSourceIndicatorDescription(UCHAR_T sourceInd)
{
  string res = "Source indicator=(";
  switch(sourceInd)
  {
    case EINSS7_I97_SOURCE_NET : res += "Network initiated release";break;
    case EINSS7_I97_SOURCE_LOC : res += "ISUP module initiated release";break;
  }
  res += ")";
  return res;
}

std::string getCauseDescription(EINSS7_I97_CAUSE_T *cause_sp)
{
  if (!cause_sp) return "";
  string res = "Cause indicator=(";
  switch(cause_sp->location)
  {
    case EINSS7_I97_ISUP_USER     : res += "location=User";break;
    case EINSS7_I97_PRIV_NETW_LOC : res += "location=Private network serving local user";break;
    case EINSS7_I97_PUBL_NETW_LOC : res += "location=Public network serving local user";break;
    case EINSS7_I97_TRANS_NETW    : res += "location=Transit network";break;
    case EINSS7_I97_PUBL_NETW_REM : res += "location=Public network serving remote user";break;
    case EINSS7_I97_PRIV_NETW_REM : res += "location=Private network serving remote user";break;
    case EINSS7_I97_INTERNAT_NETW : res += "location=International network";break;
    case EINSS7_I97_BEYOND_INTERW : res += "location=Beyond an interworking point";break;
    default                       : res += "location=NOT STANDARDAZED";break;
  }
  switch(cause_sp->codingStd)
  {
    case 0x00 : res += " codingStandard=ITU-T standard";break;
    case 0x01 : res += " codingStandard=ISO/IEC standard";break;
    case 0x02 : res += " codingStandard=national standard";break;
    case 0x03 : res += " codingStandard=local use";break;
    default   : res += " codingStandard=XXX";break;
  }
  if (cause_sp->codingStd == 0x03)
  {
    switch (cause_sp->causeValue)
    {
      case EINSS7_I97_LC_HARD_BLOCK : res += " cause=Circuit hardware blocked";break;
      case EINSS7_I97_LC_DUAL_SEIZ  : res += " cause=Dual seizure";break;
      case EINSS7_I97_LC_INV_RG     : res += " cause=Invalid Resource Group";break;
      case EINSS7_I97_LC_INV_HSN    : res += " cause=Invalid Hardware Selection Number";break;
      case EINSS7_I97_LC_INV_SPAN   : res += " cause=Invalid Span";break;
      case EINSS7_I97_LC_INV_TS     : res += " cause=Invalid Timeslot (circuit)";break;
      case EINSS7_I97_LC_USR_ERR    : res += " cause=User Application protocol error";break;
      case EINSS7_I97_LC_NTW_ERR    : res += " cause=Network protocol error";break;
      case EINSS7_I97_LC_NO_MEM     : res += " cause=Out of memory (temporarily)";break;
      case EINSS7_I97_LC_NO_CIRC    : res += " cause=No circuit available in RG or HSN/SPN";break;
      case EINSS7_I97_LC_TS_UNAVAIL : res += " cause=Specified circuit unavailable";break;
      case EINSS7_I97_LC_T6_REL     : res += " cause=Suspended call released on T6 expiry";break;
      case EINSS7_I97_LC_T7_REL     : res += " cause=No ACM received (T7 expired)";break;
      case EINSS7_I97_LC_T9_REL     : res += " cause=No ANM received (T9 expired)";break;
      case EINSS7_I97_LC_RESET      : res += " cause=Circuit reset by network";break;
      case EINSS7_I97_LC_UCIC_TWICE : res += " cause=UCIC has been received twice for this setup. No more reattempts";break;
      case EINSS7_I97_LC_CONG       : res += " cause=Destination congested";break;
      case EINSS7_I97_LC_LOC_RESET  : res += " cause=Circuit locally reset";break;
      default                       : res += " cause=RESERVED";break;
    }
  }
  if (cause_sp->codingStd == 0x00)
  {
    switch (cause_sp->causeValue)
    {
      case 0x01 : res += " cause=normal[unallocated number]";break;
      case 0x02 : res += " cause=normal[no route to specified transit network]";break;
      case 0x03 : res += " cause=normal[no route to destination]";break;
      case 0x04 : res += " cause=normal[send special information tone]";break;
      case 0x05 : res += " cause=normal[misdialled trunk prefix]";break;
      case 0x08 : res += " cause=normal[preemption]";break;
      case 0x09 : res += " cause=normal[preemption - circuit reserved for reuse]";break;
      case 0x10 : res += " cause=normal[normal clearing]";break;
      case 0x11 : res += " cause=normal[called user busy]";break;
      case 0x12 : res += " cause=normal[no user responding]";break;
      case 0x13 : res += " cause=normal[no answer from user]";break;
      case 0x14 : res += " cause=normal[subscriber absent]";break;
      case 0x15 : res += " cause=normal[call rejected]";break;
      case 0x16 : res += " cause=normal[number changed]";break;
      case 0x1B : res += " cause=normal[destination out of order]";break;
      case 0x1C : res += " cause=normal[address incomplete]";break;
      case 0x1D : res += " cause=normal[facility rejected]";break;
      case 0x1F : res += " cause=normal[unspecified]";break;
      case 0x22 : res += " cause=service/option not available[no circuit available]";break;
      case 0x26 : res += " cause=service/option not available[network out of order]";break;
      case 0x29 : res += " cause=service/option not available[temporary failure]";break;
      case 0x2A : res += " cause=service/option not available[switching equipment congestion]";break;
      case 0x2B : res += " cause=service/option not available[access information discarded]";break;
      case 0x2C : res += " cause=service/option not available[requested circuit not available]";break;
      case 0x2E : res += " cause=service/option not available[precedence call blocked]";break;
      case 0x2F : res += " cause=service/option not available[resource unavailable - unspecified]";break;
      case 0x32 : res += " cause=service/option not available[requested facility not subscribed]";break;
      case 0x37 : res += " cause=service/option not available[incoming calls barred within CUG]";break;
      case 0x39 : res += " cause=service/option not available[bearer capability not authorized]";break;
      case 0x3A : res += " cause=service/option not available[bearer capability not presently available]";break;
      case 0x3E : res += " cause=service/option not available[inconsistency in designated outgoing access information and subscriber class]";break;
      case 0x3F : res += " cause=service/option not available[service/option not available - unspecified]";break;
      case 0x41 : res += " cause=service/option not implemented[bearer capability not implemented]";break;
      case 0x45 : res += " cause=service/option not implemented[requested facility not implemented]";break;
      case 0x46 : res += " cause=service/option not implemented[only restricted digital information bearer capability is available]";break;
      case 0x4F : res += " cause=service/option not implemented[unspecified]";break;
      case 0x57 : res += " cause=invalid message[called user not member of CUG]";break;
      case 0x58 : res += " cause=invalid message[incompatible destination]";break;
      case 0x5A : res += " cause=invalid message[non-existent CUG]";break;
      case 0x5B : res += " cause=invalid message[invalid transit network selection]";break;
      case 0x5F : res += " cause=invalid message[unspecified]";break;
      case 0x61 : res += " cause=protocol error[message type non-existent or not implemented]";break;
      case 0x63 : res += " cause=protocol error[parameter non-existent or not implemented - discarded]";break;
      case 0x66 : res += " cause=protocol error[recovery on timer expiry]";break;
      case 0x67 : res += " cause=protocol error[parameter non-existent or not implemented - passed on]";break;
      case 0x6E : res += " cause=protocol error[message with unrecognized parameter - discarded]";break;
      case 0x6F : res += " cause=protocol error[unspecified]";break;
      case 0x7F : res += " cause=interworking[unspecified]";break;
      default   : res += " cause=UNKNOWN";break;
    }
  }
  if (cause_sp->lengthOfDiagnostics > 0)
  {
      res += " dignostic is present";
  }
  res += ")";
  return res;
}
const char* getModuleName(USHORT_T moduleId)
{
  switch(moduleId)
  {
    case SS7_BASE_ID: return "SS7_BASE_ID";
    case MOBILE_ID: return "MOBILE_ID";
    case PLMN_ID: return "PLMN_ID";
    case MAP_ID: return "MAP_ID";
    case TCAP_ID: return "TCAP_ID";
    case SCCP_ID: return "SCCP_ID";
    case MTPL3_ID: return "MTPL3_ID";
    case MGMT_ID: return "MGMT_ID";
    case TESTREADER_ID: return "TESTREADER_ID";
    case TESTLOGGER_ID: return "TESTLOGGER_ID";
    case TUP_ID: return "TUP_ID";
    case DUP_ID: return "DUP_ID";
    case ISUP_ID: return "ISUP_ID";
    case MTPL2_ID: return "MTPL2_ID";
    case MTPL2R1_ID: return "MTPL2R1_ID";
    case MTPL2R2_ID: return "MTPL2R2_ID";
    case SCCP_INIT_ID: return "SCCP_INIT_ID";
    case TUPUP_ID: return "TUPUP_ID";
    case ISUPUP_ID: return "ISUPUP_ID";
    case DUPUP_ID: return "DUPUP_ID";
    case BSSAP_ID: return "BSSAP_ID";
    case INAP_ID: return "INAP_ID";
    case INAPUP_ID: return "INAPUP_ID";
    case XMGMT_ID: return "XMGMT_ID";
    case CC_ID: return "CC_ID";
    case IWE_ID: return "IWE_ID";
    case OM_ID: return "OM_ID";
    case MMI_ID: return "MMI_ID";
    case MAPUP_ID: return "MAPUP_ID";
    case ISTUP_ID: return "ISTUP_ID";
    case IS41_ID: return "IS41_ID";
    case XMGMT2_ID: return "XMGMT2_ID";
    case USER01_ID: return "USER01_ID";
    case USER02_ID: return "USER02_ID";
    case USER03_ID: return "USER03_ID";
    case USER04_ID: return "USER04_ID";
    case USER05_ID: return "USER05_ID";
    case USER06_ID: return "USER06_ID";
    case USER07_ID: return "USER07_ID";
    case USER08_ID: return "USER08_ID";
    case USER09_ID: return "USER09_ID";
    case USER10_ID: return "USER10_ID";
    case USER11_ID: return "USER11_ID";
    case USER12_ID: return "USER12_ID";
    case USER13_ID: return "USER13_ID";
    case USER14_ID: return "USER14_ID";
    case USER15_ID: return "USER15_ID";
    case USER16_ID: return "USER16_ID";
    case USER17_ID: return "USER17_ID";
    case USER18_ID: return "USER18_ID";
    case USER19_ID: return "USER19_ID";
    case USER20_ID: return "USER20_ID";
    case MTPL3T_ID: return "MTPL3T_ID";
    case MTPL3C_ID: return "MTPL3C_ID";
    case VNMS_ID: return "VNMS_ID";
    case EMAP_ID: return "EMAP_ID";
    case VAP_ID: return "VAP_ID";
    case VAPUP_ID: return "VAPUP_ID";
    case IS41UP_ID: return "IS41UP_ID";
    case FEIF01_ID: return "FEIF01_ID";
    case FEIF02_ID: return "FEIF02_ID";
    case FEIF03_ID: return "FEIF03_ID";
    case FEIF04_ID: return "FEIF04_ID";
    case FEIF05_ID: return "FEIF05_ID";
    case FEIF06_ID: return "FEIF06_ID";
    case FEIF07_ID: return "FEIF07_ID";
    case FEIF08_ID: return "FEIF08_ID";
    case FEIF09_ID: return "FEIF09_ID";
    case FEIF10_ID: return "FEIF10_ID";
    case L201_ID: return "L201_ID";
    case L202_ID: return "L202_ID";
    case L203_ID: return "L203_ID";
    case L204_ID: return "L204_ID";
    case L205_ID: return "L205_ID";
    case L206_ID: return "L206_ID";
    case L207_ID: return "L207_ID";
    case L208_ID: return "L208_ID";
    case L209_ID: return "L209_ID";
    case L210_ID: return "L210_ID";
    case APC7_ID: return "APC7_ID";
    case HAMON0_ID: return "HAMON0_ID";
    case HAMON1_ID: return "HAMON1_ID";
    case HAMON2_ID: return "HAMON2_ID";
    case HAMON3_ID: return "HAMON3_ID";
    case HAMON4_ID: return "HAMON4_ID";
    case HAMON5_ID: return "HAMON5_ID";
    case HAMON6_ID: return "HAMON6_ID";
    case HAMON7_ID: return "HAMON7_ID";
    case HAMON8_ID: return "HAMON8_ID";
    case HAMON9_ID: return "HAMON9_ID";
    case HAMON_ID: return "HAMON_ID";
    case HACTRL_ID: return "HACTRL_ID";
    case PSMON_ID: return "PSMON_ID";
    case SS7CTRL_ID: return "SS7CTRL_ID";
    case TCAP01_ID: return "TCAP01_ID";
    case INAP01_ID: return "INAP01_ID";
    case ANSITCAP_ID: return "ANSITCAP_ID";
    case ANSITCAP01_ID: return "ANSITCAP01_ID";
    case ETSIMAP_ID: return "ETSIMAP_ID";
    case SS7IPLOCAL_ID: return "SS7IPLOCAL_ID";
    case SS7IP1_ID: return "SS7IP1_ID";
    case SS7IP2_ID: return "SS7IP2_ID";
    case SS7IP3_ID: return "SS7IP3_ID";
    case SS7IP4_ID: return "SS7IP4_ID";
    case SS7IP5_ID: return "SS7IP5_ID";
    case CP_ID: return "CP_ID";
    case DAEMON_ID: return "DAEMON_ID";
    case SNMPAGENT_ID: return "SNMPAGENT_ID";
    case BEFEIFFAST_ID: return "BEFEIFFAST_ID";
    case ISRL201_ID: return "ISRL201_ID";
    case ISRL202_ID: return "ISRL202_ID";
    case ISRL203_ID: return "ISRL203_ID";
    case ISRL204_ID: return "ISRL204_ID";
    case ISRL205_ID: return "ISRL205_ID";
    case ISRL206_ID: return "ISRL206_ID";
    case ISRL207_ID: return "ISRL207_ID";
    case ISRL208_ID: return "ISRL208_ID";
    case LIC_ID: return "LIC_ID";
    case DECODER_ID: return "DECODER_ID";
    case MISC01_ID: return "MISC01_ID";
    case MISC02_ID: return "MISC02_ID";
    case MISC03_ID: return "MISC03_ID";
    case MISC04_ID: return "MISC04_ID";
    case MISC05_ID: return "MISC05_ID";
    case MISC06_ID: return "MISC06_ID";
    case MISC07_ID: return "MISC07_ID";
    case MISC08_ID: return "MISC08_ID";
    case MISC09_ID: return "MISC09_ID";
    case MISC10_ID: return "MISC10_ID";
    case A41MAP_ID: return "A41MAP_ID";
    case A41MAPUP_ID: return "A41MAPUP_ID";
    case VNMS_AL_ID: return "VNMS_AL_ID";
    case BSSAPLEUP_ID: return "BSSAPLEUP_ID";
    case MGMT1_ID: return "MGMT1_ID";
    case MGMT2_ID: return "MGMT2_ID";
    default: return "UNKNOWN MODULE";
  }
}
const char* getOamApiResultDescription(USHORT_T retCode)
{
  switch(retCode)
  {
    case EINSS7OAMAPI_RESULT_OK: return "Success";
    case EINSS7OAMAPI_RESULT_PRIMITIVE: return "Not an OAM IF primitive";
    case EINSS7OAMAPI_RESULT_PARSE_LENGTH: return "Length of primitive is too short";
    case EINSS7OAMAPI_RESULT_PARSE_RANGE: return "The range of a parameter in the primitive is out of bounds";
    case EINSS7OAMAPI_RESULT_NO_CALL_BACK: return "There is no call back function available";
    case EINSS7OAMAPI_RESULT_UNABLE_TO_ALLOCATE_MEM: return "Memory allocation failed";
    case EINSS7OAMAPI_NOT_INITIALIZED: return "OAM API was not initialized when trying to use Req or Resp function";
    case EINSS7OAMAPI_MODULE_DATA_ERROR: return "The module data provided to a Req/Resp function was invalid";
    case EINSS7OAMAPI_CONFIG_DATA_ERROR: return "The config data provided to a Req/Resp function was invalid";
    case EINSS7OAMAPI_MISSING_SERVICE_REQ: return "Service Request not provided to a Req/Resp function";
    case EINSS7OAMAPI_UNKNOWN_SERVICE_REQ: return "The Service Request provided to a Req/Resp function was invalid";
    case EINSS7OAMAPI_MISSING_INFOID: return "InfoId not provided to a Req/Resp function";
    case EINSS7OAMAPI_UNKNOWN_INFOID: return "The InfoId provided to a Req/Resp function was invalid";
    default: return "UNKNOWN RETURN CODE";
  }
}
const char* getReturnCodeDescription(USHORT_T code)
{
  switch(code)
  {
    case 1000: return "MSG TIMEOUT";
    case 1001: return "MSG OUT OF MEMORY";
    case 1002: return "MSG ID NOT FOUND";
    case 1003: return "MSG QUEUE NOT OPEN";
    case 1004: return "MSG NOT OPEN";
    case 1005: return "MSG SIZE ERROR";
    case 1006: return "MSG INTERNAL QUEUE FULL";
    case 1007: return "MSG ERR";
    case 1008: return "MSG SYSTEM INTERRUPT";
    case 1009: return "MSG NOT CONNECTED";
    case 1010: return "MSG NOT ACCEPTED";
    case 1011: return "MSG BROKEN CONNECTION";
    case 1012: return "MSG NOT MPOWNER";
    case 1013: return "MSG BAD FD";
    case 1014: return "MSG ARG VAL";
    case 1015: return "MSG APPL EVENT";
    case 1016: return "MSG OPEN FAILED";
    case 1017: return "MSG WOULD BLOCK";
    case 1018: return "MSG GETBUF FAIL";
    case 1019: return "MSG RELBUF FAIL";
    case 1024: return "MSG INSTANCE NOT FOUND";
    case 1020: return "TIME INIT FAILED";
    case 1021: return "TOO MANY TIMERS";
    case 1022: return "NAME NOT FOUND";
    case 1023: return "MSG ID NOT IN CONFIG FILE";
    case 1030: return "XMEM INVALID USERID";
    case 1031: return "XMEM OUT OF MEMORY";
    case 1032: return "XMEM INVALID POINTER";
    case 1033: return "XMEM INIT DONE";
    case 1034: return "XMEM INVALID PARAMETER";
    case 1035: return "XMEM INVALID SIZE";
    case 1036: return "XMEM CORRUPT MEM";
    case 1040: return "THREAD MI PROBLEM";
    case 1041: return "THREAD KC PROBLEM";
    case 1042: return "THREAD CI PROBLEM";
    case 1043: return "MPOWNER CLOSED";
    case 1044: return "TIME NOT INIT";
    case 1045: return "NO TOKEN";
    case 1060: return "INVALID SOCKET";
    case 1061: return "SOCKET ERROR";
    case 1062: return "INVALID EVENT";
    case 1063: return "EVENT ERROR";
    case 1070: return "MSG POOL ALREADY CREATED";
    case 1071: return "MSG POOL MAX NUMBER REACHED";
    case 1072: return "INVALID POINTER";
    case 1124: return "MSG MSGDELAY ERROR";
    case 1100: return "MSG UNKNOWN FILE";
    case 1101: return "MSG IPA UNKNOWN MODULE";
    case 1102: return "MSG IPA SYNTAX ERROR";
    case 1103: return "MSG IPA NOT MPOWNER";
    case 1104: return "MSG IPA MISSING ADDRESS";
    case 1105: return "MSG IPA MULTIPLE ENTRIES";
    case 1106: return "MSG INTERACT UNKNOWN MODULE";
    case 1107: return "MSG INTERACT SYNTAX ERROR";
    case 1108: return "MSG INTERACT MULTIPLE ENTRIES";
    case 1109: return "MSG INTERACT NOT MPOWNER";
    case 1110: return "MSG BUFSIZE ERROR";
    case 1111: return "MSG MAXENTRIES ERROR";
    case 1112: return "MSG CONFIG ERR";
    case 1113: return "LOG WRITEMODE ERROR";
    case 1114: return "LOG FILESIZE ERROR";
    case 1115: return "LOG FILEPATH ERROR";
    case 1116: return "LOG INTERNALBUFFER ERROR";
    case 1117: return "LOG FILEFORMAT ERROR";
    case 1118: return "MSG HBLOST ERROR";
    case 1119: return "MSG HBRATE ERROR";
    case 1120: return "MSG MSGNONBLOCK ERROR";
    case 1121: return "MSG SYSTIME ERROR";
    case 1122: return "LOG PROCPATH ERROR";
    case 1123: return "LOG FILEPROP ERROR";
    case 1125: return "MSG HBOFF NOT MPOWNER";
    case 1126: return "MSG HBOFF SYNTAX ERROR";
    case 1127: return "MSG UXDOM SYNTAX ERROR";
    case 1128: return "MSG UXDOM NOT AVAILABLE";
    case 1129: return "MSG UXDOM ARG ERROR";
    case 1130: return "LOG BACKFILEPROP ERROR";
    case 1131: return "LOG LD IPA ERROR";
    case 1132: return "LOG FLUSH ERROR";
    case 1133: return "LOGD SOCKET ERROR";
    case 1134: return "LOGD BIND ERROR";
    case 1135: return "LOG NOT INITIATED";
    case 1136: return "MSGINIT DONE";
    case 1137: return "NO THREAD CREATED";
    case 1138: return "STOP SOCKET ERROR";
    case 1139: return "SIGNAL ERROR";
    case 1140: return "MSG NO BUFFER";
    case 1141: return "MUTEX INIT FAILED";
    case 1142: return "COND INIT FAILED";
    case 1143: return "ANOTHER THREAD CLOSING CON";
    case 1144: return "AUTOSETUP ADDR";
    case 1145: return "NO IPC EXIST";
    case 1146: return "CREATE IPC ERROR";
    case 1147: return "EMPTY BUFFER";
    case 1148: return "READ SOCKET BUFFER ERROR";
    case 1149: return "WRITE SOCKET BUFFER ERROR";
    case 1150: return "SEND BUFFER ERROR";
    case 1151: return "MSG SEND FAIL";
    case 1152: return "MSGINIT NOT DONE";
    case 1153: return "NOT IMPLEMENTED";
    case 1154: return "TIMER NOT IN USE";
    case 1155: return "MSG CONNTYPE NOT MPOWNER";
    case 1156: return "LOGALARM SYNTAX ERROR";
    case 1157: return "UNIXSOCKPATH ERROR";
    case 1158: return "MSGNODELAY ERROR";
    case 1159: return "MSGTRACEON ERROR";
    case 1160: return "MSGCONNTYPE ERROR";
    case 1161: return "TESTMODULE ERROR";
    case 1162: return "NO CPMANAGER ERROR";
    case 1163: return "CONNTIMEOUT ERROR";
    case 1164: return "SOCKETTIMEOUT ERROR";
    case 1165: return "FILEACCESSTYPE ERROR";
    case 1400: return "MSG ZBUF CREATE FAIL";
    case 1401: return "MSG ZBUF DELETE FAIL";
    case 1402: return "MSG ZBUF SEND FAIL";
    case 1450: return "CPMGMT NOT USED";
    case 1251: return "MSGREGBROKENCONNFAIL";
    case 1252: return "MSGREGISTERCALLBACKFAIL";
    case 1500: return "MSG OSE SYNTAX ERROR";
    case 1502: return "MSG APPL EVENT EXTERNAL";
    case 1503: return "CELLO BAD IPADDR";
    case 1504: return "CELLO UNSUPP FAMILY";
    case 1505: return "EXTSIGREGISTERCALLBACKFAIL";
    case 1506: return "MSG OSE UNKNOWN MODULE";
    default:   return "UNKNOWN CODE";
  }
}
}//namespace util
}//namespace misscall
}//namespace smsc
