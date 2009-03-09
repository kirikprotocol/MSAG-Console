static char const ident[] = "$Id$";
#include "mtsmsme/processor/util.hpp"
#include <alloca.h>
#include <vector>
namespace smsc{
namespace mtsmsme{
namespace processor{
namespace util{
using namespace std;
extern "C" int print2vec(const void *buffer, size_t size, void *app_key) {
  std::vector<unsigned char> *stream = (std::vector<unsigned char> *)app_key;
  unsigned char *buf = (unsigned char *)buffer;

  stream->insert(stream->end(),buf, buf + size);

  return 0;
}
unsigned packNumString2BCD(unsigned char* bcd, const char* str,
                                  unsigned slen, bool filler)
{
    unsigned bcdLen = (slen + 1)/2;

    for (unsigned i = 0; i < slen; ++i) {
    if (i & 0x01) // i % 2
        bcd[i/2] |= ((str[i]-'0') << 4);	// fill high semioctet
    else
        bcd[i/2] = (str[i]-'0') & 0x0F;	// fill low semioctet
    }
    if ((slen % 2) && filler)
    bcd[bcdLen - 1] |= 0xF0;		// add filler to high semioctet

    return bcdLen;
}
unsigned packNumString2BCD91(unsigned char* dst, const char* str,
                                  unsigned slen, bool filler)
{
    unsigned bcdLen = (slen + 1)/2;
    dst[0] = 0x91;
    unsigned char* bcd = dst + 1;

    for (unsigned i = 0; i < slen; ++i) {
    if (i & 0x01) // i % 2
        bcd[i/2] |= ((str[i]-'0') << 4);	// fill high semioctet
    else
        bcd[i/2] = (str[i]-'0') & 0x0F;	// fill low semioctet
    }
    if ((slen % 2) && filler)
    bcd[bcdLen - 1] |= 0xF0;		// add filler to high semioctet

    return bcdLen + 1;
}
unsigned unpackBCD912NumString(char* dst, const unsigned char* src, unsigned srclen)
{
    unsigned i, k = 0;
    for (i = 1; i < srclen; i++) { // starting from index 1 to skip TON,NPI
        dst[k++] = '0' + (char)(src[i] & 0x0F); // low semioctet
        // high semioctet, check for possible filler
        if ((dst[k] = (char)((src[i] >> 4) & 0x0F)) == 0x0F)
            break;
        dst[k++] += '0';
    }
    dst[k] = 0;
    return k;
}
unsigned packSCCPAddress(unsigned char* dst, unsigned char npi, const char *saddr, unsigned char ssn)
{
    unsigned len = strlen(saddr);
    unsigned addrLen = 5 + (len + 1)/2;    // length in octets
    dst[0]  = 0x12;                        // GlobTitle(0100) & SSN indicator
    dst[1]  = ssn;                         // SSN
    dst[2]  = 0;                           // Translation Type (not used)
    dst[3]  = npi << 4 ;                   // NumPlan(ISDN) &
    dst[3] |= (!(len%2) ? 0x02 : 0x01);    // encScheme(BCD odd/even)
    dst[4]  = 0x04;                        // Nature of address (international)

    //NOTE: SCCP address uses filler '0000'B !!!
    packNumString2BCD(dst + 5, saddr, len, false);
    return len ? addrLen : 0;
}
static void unpack_addr(char* dst, uint8_t* src, int len)
{
  uint8_t sign;
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

bool modifyssn(uint8_t* src, uint8_t len, const char* pattern, uint8_t newssn, bool uncoditional)
{
  if( !len || !src) return false;
  int i=0; //position
  uint8_t ai; //address indicator
  uint8_t* ssn_ptr = 0;
  ai = src[i++];
  if (ai & 0x01) // spc included
  {
    //skip for now
    i++;i++;
  }
  if (ai & 0x02) { //ssn included
    ssn_ptr = &src[i++];
  }
  {
    i++;//skip TT, traslation type
    uint8_t npi = src[i++];
    uint8_t odd = npi & 0x01;
    i++;//skip TON

    char ad[32];
    unpack_addr(ad,src+i, (len - i)*2 - odd);
    if (strcmp(ad,pattern) == 0 || uncoditional)
    {
      *ssn_ptr = newssn;
      return true;
    }
  }
  return false;
}

std::string getAddressDescription(uint8_t len, uint8_t* buf)
{
  string res="";
  if (len == 0 || !buf) return res;

  int i=0; //position
  uint8_t ai; //address indicator
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
      uint8_t npi = buf[i++];
      uint8_t odd = npi & 0x01;
      npi = (npi >> 4) & 0x0F;
      k = sprintf(tmp,"%d",npi); tmp[k] = 0;
      res+=",NP=";res+=tmp;
      uint8_t na = buf[i++];
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

const char* getUserState(uint8_t state) {
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
std::string getTypeOfServiceDescription(uint8_t typeOfService) {
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

const char* getStackStatusDescription(uint8_t mmState)
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

const char* getResultDescription(uint8_t result)
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

const char* getReturnCodeDescription(uint16_t code)
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

std::string dump(uint16_t len, uint8_t* udp) {
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
