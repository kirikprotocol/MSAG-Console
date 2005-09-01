#include "util.hpp"
#include <alloca.h>
namespace smsc{
namespace mtsmsme{
namespace processor{
namespace util{
using namespace std;
std::string getAddressDescription(UCHAR_T len, UCHAR_T* buf)
{
  string res="";
  if (len == 0 || !buf) return res;

  int i=0; //position
  UCHAR_T ai; //address indicator
  ai = buf[i++];

  if (ai & 0x40)
    res+="SSNRC";
  else
    res+="GTRC";

  if (ai & 0x01) // spc included
  {
    //skip for now
    i++;i++;
  }
  if (ai & 0x02) { //ssn included
    res+=" SSN=";
    char tmp[4];int k;
    k = sprintf(tmp,"%d",buf[i++]); tmp[k] = 0;
    res+=tmp;
  }
  switch((ai >> 2) & 0x0F) {
    case 0x04:
    {
      res+=" GT=(";
      char tmp[4];int k;
      k = sprintf(tmp,"%d",buf[i++]); tmp[k] = 0;
      res+="TT=";res+=tmp;
      UCHAR_T npi = buf[i++];
      UCHAR_T odd = npi & 0x01;
      npi = (npi >> 4) & 0x0F;
      k = sprintf(tmp,"%d",npi); tmp[k] = 0;
      res+=",NP=";res+=tmp;
      UCHAR_T na = buf[i++];
      na &=0x7F;
      k = sprintf(tmp,"%d",na); tmp[k] = 0;
      res+=",NA=";res+=tmp;
      char ai[32];
      unpack_addr(ai,buf+i, (len - i)*2 - odd);
      res+=",AI=";res+=ai;res+=")";
      break;
    }
    default: res+=" don't know how to decode";
  }
  return res;
/*
//  vector<char> addr(called->noOfAddrSign + 1);
//  unpack_addr(&addr[0], called->addrSign_p, called->noOfAddrSign);
//  addr.insert(addr.begin(),'.');
//  addr.insert(addr.begin(),called->numberPlan + '0');
//  addr.insert(addr.begin(),'.');
//  addr.insert(addr.begin(),called->natureOfAddr + '0');
//  addr.insert(addr.begin(),'.');
  vector<char> addr;
  addr.reserve(called->noOfAddrSign + 1);
  addr.insert(addr.end(),called,called+sizeof(called)-1);
  addr.push_back('.');
  addr.push_back(called->natureOfAddr + '0');
  addr.push_back('.');
  addr.push_back(called->numberPlan + '0');
  addr.push_back('.');
  unpack_addr(addr, called->addrSign_p, called->noOfAddrSign);
  return string(&addr[0]);
*/
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

/*
void unpack_addr(vector<char>& dst, UCHAR_T* src, int len)
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
    char cdst = 0;
    switch(sign)
    {
      case 0: case 1: case 2: case 3: case 4: case 5:
      case 6: case 7: case 8: case 9: cdst = sign + '0'; break;
      case 11: cdst = '*'; break;
      case 12: cdst = '#'; break;
      case 15: cdst =  0 ; break;
      default: cdst = '?'; break;
    }
    dst.push_back(cdst);
  }
  dst.push_back(0);
}
*/
const char* getReturnOptionDescription(UCHAR_T opt) {
  switch(opt) {
    case EINSS7_I96SCCP_RET_OPT_OFF: return "\"Discard message if fail\"";
    case EINSS7_I96SCCP_RET_OPT_ON: return "\"Return message if fail\"";
    default: return "\"UNKNOWN return option\"";
  }
}
const char* getSequenceControlDescription(UCHAR_T ctrl) {
  switch(ctrl) {
    case EINSS7_I96SCCP_SEQ_CTRL_OFF: return "\"PROTO CLASS=0\"";
    case EINSS7_I96SCCP_SEQ_CTRL_ON: return "\"PROTO CLASS=1\"";
    default: return "\"UNKNOWN protocol class\"";
  }
}
const char* getUserState(UCHAR_T state) {
  switch(state)
  {
    case 0x00: return "\"User in service at SPC with congestion level 0.\"";
    case 0x01: return "\"User unavailable.\"";
    case 0x02: return "\"User in service at SPC with congestion level 1.\"";
    case 0x03: return "\"User in service at SPC with congestion level 2.\"";
    case 0x04: return "\"User in service at SPC with congestion level 3.\"";
    case 0x05: return "\"User in service at SCCP with congestion level 1.\"";
    case 0x06: return "\"User in service at SCCP with congestion level 2.\"";
    case 0x07: return "\"User in service at SCCP with congestion level 3.\"";
    case 0x08: return "\"User in service at SCCP with congestion level 4.\"";
    case 0x09: return "\"User in service at SCCP with congestion level 5.\"";
    case 0x10: return "\"User in service at SCCP with congestion level 6.\"";
    case 0x11: return "\"User in service at SCCP with congestion level 7.\"";
    case 0x12: return "\"User in service at SCCP with congestion level 8.\"";
    default: return "\"unknown state value\"";
  }
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

const char* getSccpBindStatusDescription(UCHAR_T result)
{
  switch (result)
  {
    case EINSS7_I96SCCP_NB_SUCCESS: return "\"Bind success\"";
    case EINSS7_I96SCCP_SSN_ALREADY_IN_USE: return "\"Subsystem number in use\"";
    case EINSS7_I96SCCP_PROTOCOL_ERROR: return "\"Protocol error\"";
    case EINSS7_I96SCCP_NB_RES_UNAVAIL: return "\"Resources unavailable\"";
    case EINSS7_I96SCCP_SSN_NOT_ALLOWED: return "\"Invalid subsystem number\"";
    case EINSS7_I96SCCP_SCCP_NOT_READY: return "\"SCCP Not available\"";
    default: return "UNKNOWN STATUS ERROR";
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

}//namespace util
}//namespace processor
}//namespace mtsms
}//namespace smsc
