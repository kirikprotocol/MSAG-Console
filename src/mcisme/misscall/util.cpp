#include "logger/Logger.h"
#include "util.hpp"
#include <vector>

namespace smsc{
namespace misscall{
namespace util{

using smsc::logger::Logger;
using namespace std;
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

std::string getCallingNumberDescription(EINSS7_I97_CALLINGNUMB_T* calling)
{
  if (!calling) return "";
  vector<char> addr(calling->noOfAddrSign + 1);
  unpack_addr(&addr[0], calling->addrSign_p, calling->noOfAddrSign);
  addr.insert(addr.begin(),calling->natureOfAddr + '0');
  addr.insert(addr.begin(),'.');
  addr.insert(addr.begin(),calling->numberPlan + '0');
  addr.insert(addr.begin(),'.');
  char str[]="Calling=";
  char *end=str+sizeof(str)-1;
  addr.insert(addr.begin(),str,end);
  return &addr[0];
}
std::string getRedirectinNumberDescription(EINSS7_I97_REDIRECTINGNUMB_T* redirNumb)
{
  if (!redirNumb) return "";
  vector<char> addr(redirNumb->noOfAddrSign + 1);
  unpack_addr(&addr[0], redirNumb->addrSign_p, redirNumb->noOfAddrSign);
  char str[]="Redirecting=";
  char *end=str+sizeof(str)-1;
  addr.insert(addr.begin(),str,end);
  return &addr[0];
}
std::string getOriginalNumberDescription(EINSS7_I97_ORIGINALNUMB_T* origNumb)
{
  if (!origNumb) return "";
  vector<char> addr(origNumb->noOfAddrSign + 1);
  unpack_addr(&addr[0], origNumb->addrSign_p, origNumb->noOfAddrSign);
  addr.insert(addr.begin(),origNumb->natureOfAddr + '0');
  addr.insert(addr.begin(),'.');
  addr.insert(addr.begin(),origNumb->numberPlan + '0');
  addr.insert(addr.begin(),'.');
  char str[]="Original=";
  char *end=str+sizeof(str)-1;
  addr.insert(addr.begin(),str,end);
  return &addr[0];
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
    case EINSS7_I97_REASON_UNKNOWN : res +=  "Unknown/not available";break;
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
    case EINSS7_I97_REASON_UNKNOWN  : res +=  "Unknown/not available";break;
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
             "CID=%ld HSN=%d SPN=%d TS=%d",
             isupHead_sp->connectionId,
             isupHead_sp->hsn,
             isupHead_sp->span,
             isupHead_sp->timeslot);
  }
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
const char* getIsupBindStatusDescription(UCHAR_T result)
{
  switch (result)
  {
    case EINSS7_I97_BIND_OK: return "Call Control connected";
    case EINSS7_I97_ANOTHER_CC_BOUND: return "Another Call Control was bound";
    case EINSS7_I97_SAME_CC_BOUND: return "Same Call Control already bound";
  }
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
}//namespace util
}//namespace misscall
}//namespace smsc
