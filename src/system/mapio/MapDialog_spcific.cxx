#include "MapIoTask.h"
#include "../../sms/sms.h"
#include "../../smeman/smsccmd.h"
#include <memory>
#include <list>
#include <time.h>
#include <stdexcept>

using namespace std;
using namespace smsc::sms;
using namespace smsc::smeman;

#define TP_VP_NONE  0
#define TP_VP_REL   1
#define TP_VP_ENCH  2
#define TP_VP_ABS   3

#define MAP_OCTET7BIT_ENCODING 0x0
#define MAP_SMSC7BIT_ENCODING  smsc::smpp::DataCoding::SMSC7BIT
#define MAP_LATIN1_ENCODING    smsc::smpp::DataCoding::LATIN1
#define MAP_8BIT_ENCODING 0x4
#define MAP_UCS2_ENCODING 0x8

//#define MAKE_ERRORCODE(klass,code) (klass)
//#define MAKE_ERRORCODE(klass,code) MAKE_COMMAND_STATUS(klass,code)
#define MAP_NETWORKERROR 1

struct MicroString{
  unsigned len;
  char bytes[256];
};

extern unsigned char lll_7bit_2_8bit[128];
extern unsigned char lll_8bit_2_7bit[256];

//list<unsigned> DialogMapContainer::dialogId_pool;

inline char GetChar(const unsigned char*& ptr,unsigned& shift){
  //__map_trace2__("7bit: shift %d *ptr 0x%x",shift,*ptr);
  char val = (*ptr >> shift)&0x7f;
  if ( shift > 1 )
    val |= (*(ptr+1) << (8-shift))&0x7f;
  shift += 7;
  if ( shift >= 8 )
  {
    shift&=0x7;
    ++ptr;
  }
  //__map_trace2__("7bit : %x",val);
  return val;
}

class VeryLongText: runtime_error {
public:
  VeryLongText(): runtime_error("very long text") {}
};

inline void PutChar(unsigned char*& ptr,unsigned& shift,unsigned char val8bit,unsigned char* ptr_end){
  if ( ptr >= ptr_end || (shift > 1 && (ptr+1) >= ptr_end)) throw VeryLongText();
  unsigned char val = val8bit;
  *ptr = *ptr | (val << shift);
  if ( shift > 1 )
    *(ptr+1) = *(ptr+1) | (val >> (8-shift));
  shift += 7;
  if ( shift >= 8 )
  {
    shift&=0x7;
    ++ptr;
  }
}


inline void Convert7BitToSMSC7Bit(
  const unsigned char* bit7buf, unsigned chars,
  MicroString* text,unsigned offset=0)
{
  if (chars>255) throw runtime_error("Convert7BitToSMSC7Bit invalid chars value");
  unsigned shift = offset;
  for ( unsigned i=0; i< chars; ++i ){
    text->bytes[i] = GetChar(bit7buf,shift);
  }
  text->len = chars;
  text->bytes[chars] = 0;
}

inline void Convert7BitToText(
  const unsigned char* bit7buf, unsigned chars,
  MicroString* text,unsigned offset=0)
{
  if (chars>255) throw runtime_error("Convert7BitToSMSC7Bit invalid chars value");
  unsigned shift = offset;
  bool     escape = false;
  int      symbols = 0;
  for ( unsigned i=0; i< chars; ++i ){
    unsigned char b = GetChar(bit7buf,shift)&0x7f;
    if( escape ) {
      switch(b){
        case 0x0a: // page break
          b = '\f'; break;
        case 0x14: //
          b = '^'; break;
        case 0x1b: // ??? national
          b = '?'; break;
        case 0x40:
          b = '|'; break;
        case 0x28: // {
          b = '{'; break;
        case 0x29: // }
          b = '}'; break;
        case 0x3c: // [
          b = '['; break;
        case 0x3d: // ~
          b = '~'; break;
        case 0x3e: // ]
          b = ']'; break;
        case 0x2f:
          b = '\\'; break;
        default:
          b = '?'; break;
      }
      escape = false;
    }
    if( b == 0x1b ) { // escape symbol
      escape = true;
      continue;
    }
    text->bytes[symbols++] = lll_7bit_2_8bit[b];
  }
  text->len = symbols;
  text->bytes[symbols] = 0;
}

inline unsigned ConvertText27bit(
  const unsigned char* text, unsigned chars, unsigned char* bit7buf,unsigned* elen,
  unsigned offset,unsigned buflen)
{
  /*
  if ( chars > 160 ){
    __map_warn2__("ConvertText27bit: text length(%d) > 160",chars);
    throw runtime_error("text length > 160");
  }
  */
  unsigned char* base = bit7buf;
  unsigned char* bit7buf_end = base+buflen;
  unsigned shift = offset;
  (*elen) = 0;
  for ( unsigned i=0; i< chars; ++i ){
#define __pchar(x) PutChar(bit7buf,shift,x,bit7buf_end)
#define __escape(x) __pchar(0x1b); __pchar(x); (*elen) += 2;
    switch(text[i]){
    case '^': __escape(0x14); break;
    case '\f':__escape(0x0a); break;
    case '|': __escape(0x40); break;
    case '{': __escape(0x28); break;
    case '}': __escape(0x29); break;
    case '[': __escape(0x3c); break;
    case ']': __escape(0x3e); break;
    case '~': __escape(0x3d); break;
    case '\\':__escape(0x2f); break;
    default:
      PutChar(bit7buf,shift,lll_8bit_2_7bit[text[i]],bit7buf_end);
      (*elen) += 1;
    }
#undef __pchar
#undef __escape
  }
  unsigned _7bit_len = (unsigned)(bit7buf-base+(shift?1:0));
  return _7bit_len;
}

inline unsigned ConvertSMSC7bit27bit(
  const unsigned char* text, unsigned chars, unsigned char* bit7buf,
  unsigned offset,unsigned bufsize)
{
  unsigned char* base = bit7buf;
  unsigned char* bit7buf_end = base+bufsize;
  unsigned shift = offset;
  for ( unsigned i=0; i< chars; ++i ){
    PutChar(bit7buf,shift,text[i],bit7buf_end);
  }
  unsigned _7bit_len = (unsigned)(bit7buf-base+(shift?1:0));
  return _7bit_len;
}

inline bool provErrCodeFatal( ET96MAP_PROV_ERR_T p ) {
  return
  (p == 0x02 || // unsupported service
   p == 0x03 || // mystyped parametor
   p == 0x06 || // unexcpected responnse from peer
   p == 0x09 || // invalid responce recived
  (p > 0x0a && p <= 0x10)); // unxpected component end other
}

/*struct SMS_SUMBMIT_FORMAT_HEADER{
  union{
    struct{
      unsigned char reply_path:1;
      unsigned char udhi:1;
      unsigned char srr:1;
      unsigned char tp_vp:2;
      unsigned char reject_dupl:1;
      unsigned char mg_type_ind:2;
    }head;
    unsigned char _val_01;
  }u;
  unsigned char mr;
};
*/

struct MAP_SMS_ADDRESS{
  unsigned char len;

//  union{
//    struct{
//      unsigned char reserved_1:1;
//      unsigned char ton:3;
//      unsigned char npi:4;
//    }st;
    unsigned char tonnpi;
//  };
  unsigned char val[10];
};

struct MAP_TIMESTAMP{
  uint8_t year;
  uint8_t mon;
  uint8_t day;
  uint8_t hour;
  uint8_t min;
  uint8_t sec;
  uint8_t tz;
};

inline void mkSS7GTAddress( ET96MAP_SS7_ADDR_T *addr, const ET96MAP_ADDRESS_T *saddr, ET96MAP_LOCAL_SSN_T ssn) {
  addr->ss7AddrLen = 5+(saddr->addressLength+1)/2;
  addr->ss7Addr[0] = 0x12; // SSN & GT
  addr->ss7Addr[1] = ssn;
  addr->ss7Addr[2] = 0;
  addr->ss7Addr[3] = (saddr->typeOfAddress<<4)|(saddr->addressLength%2==0?0x02:0x01); // NP & GT coding
  addr->ss7Addr[4] = 0x04; // | (saddr->addressLength%2==0?0x80:0x00); high bit always 0 see 15517-CAA901437, 3.3.8
  memcpy( addr->ss7Addr+5, saddr->address, (saddr->addressLength+1)/2 );
  if( saddr->addressLength%2!=0 ) {
    addr->ss7Addr[5+(saddr->addressLength+1)/2-1] &= 0x0f;
  }
}

inline void mkSS7GTAddress( ET96MAP_SS7_ADDR_T *addr, const ET96MAP_LOCATION_INFO_T *saddr, ET96MAP_LOCAL_SSN_T ssn) {
  addr->ss7AddrLen = 5+(saddr->addressLength+1)/2;
  addr->ss7Addr[0] = 0x12; // SSN & GT
  addr->ss7Addr[1] = ssn;
  addr->ss7Addr[2] = 0;
  addr->ss7Addr[3] = (saddr->typeOfAddress<<4)|(saddr->addressLength%2==0?0x02:0x01); // NP & GT coding
  addr->ss7Addr[4] = 0x04; // | (saddr->addressLength%2==0?0x80:0x00); high bit always 0 see 15517-CAA901437, 3.3.8
  memcpy( addr->ss7Addr+5, saddr->address, (saddr->addressLength+1)/2 );
  if( saddr->addressLength%2!=0 ) {
    addr->ss7Addr[5+(saddr->addressLength+1)/2-1] &= 0x0f;
  }
}

inline void SS7ToText(  const ET96MAP_SS7_ADDR_T *addr , char* text ){
  unsigned k = 0;
  for ( int i=0; i < addr->ss7AddrLen; ++i ){
    k += sprintf(text+k,"%02x|",addr->ss7Addr[i]);
  }
  text[k] = 0;
}


inline void mkMapAddress( ET96MAP_ADDRESS_T *addr, const char *saddr, unsigned len) {
  unsigned i;
  int sz = (len+1)/2;
  addr->addressLength = len;
  addr->typeOfAddress = 0x91; // InterNational, ISDN
  for( i = 0; i < len; i++ ) {
    int bi = i/2;
    if( i%2 == 1 ) {
      addr->address[bi] |= ((saddr[i]-'0')<<4); // fill high octet
    } else {
      addr->address[bi] = (saddr[i]-'0')&0x0F; // fill low octet
    }
  }
  if( len%2 != 0 ) {
    addr->address[sz-1] |= 0xF0;
  }
}

inline void mkIMSIOrMSISDNFromIMSI( ET96MAP_IMSI_OR_MSISDN_T *addr, const string &s_imsi ) {
  size_t len = s_imsi.length();
  size_t sz = (len+1)/2;
  addr->imsiOrMsisdnLen = (UCHAR_T)(sz+1);
  addr->imsiOrMsisdn[0]=0x96;
  const char *value = s_imsi.c_str();
  for( size_t i = 0; i < len; i++ ) {
    UCHAR_T bi = (UCHAR_T)i/2;
    if( i%2 == 1 ) {
      addr->imsiOrMsisdn[1+bi] |= ((value[i]-'0')<<4); // fill high octet
    } else {
      addr->imsiOrMsisdn[1+bi] = (value[i]-'0')&0x0F; // fill low octet
    }
  }
  if( len%2 != 0 ) {
    addr->imsiOrMsisdn[sz] |= 0xF0;
  }
}

inline void mkIMSIOrMSISDNFromAddress( ET96MAP_IMSI_OR_MSISDN_T *addr, Address &saddr ) {
  int len = saddr.length;
  int sz = (len+1)/2;
  addr->imsiOrMsisdnLen = 1+sz;
  addr->imsiOrMsisdn[0] = 0x80|(saddr.type<<4)|saddr.plan; // InterNational, ISDN
  for( int i = 0; i < len; i++ ) {
    int bi = 1+i/2;
    if( i%2 == 1 ) {
      addr->imsiOrMsisdn[bi] |= ((saddr.value[i]-'0')<<4); // fill high octet
    } else {
      addr->imsiOrMsisdn[bi] = (saddr.value[i]-'0')&0x0F; // fill low octet
    }
  }
  if( len%2 != 0 ) {
    addr->imsiOrMsisdn[sz] |= 0xF0;
  }
}

inline void mkMapAddress( ET96MAP_ADDRESS_T *addr, const Address &saddr) {
  int len = saddr.length;
  int sz = (len+1)/2;
  addr->addressLength = saddr.length;
  addr->typeOfAddress = 0x80|(saddr.type<<4)|saddr.plan; // InterNational, ISDN
  for( int i = 0; i < len; i++ ) {
    int bi = i/2;
    if( i%2 == 1 ) {
      addr->address[bi] |= ((saddr.value[i]-'0')<<4); // fill high octet
    } else {
      addr->address[bi] = (saddr.value[i]-'0')&0x0F; // fill low octet
    }
  }
  if( len%2 != 0 ) {
    addr->address[sz-1] |= 0xF0;
  }
}

inline void mkRP_DA_Address( ET96MAP_SM_RP_DA_T *addr, const char *saddr, unsigned len, char type) {
  unsigned i;
  int sz = (len+1)/2;
  addr->addrLen = sz+1;
  addr->typeOfAddress = type;
  addr->addr[0] = 0x91; // InterNational, ISDN
  for( i = 0; i < len; i++ ) {
    int bi = i/2;
    if( i%2 == 1 ) {
      addr->addr[bi+1] |= ((saddr[i]-'0')<<4); // fill high octet
    } else {
      addr->addr[bi+1] = (saddr[i]-'0')&0x0F; // fill low octet
    }
  }
  if( len%2 != 0 ) {
    addr->addr[sz] |= 0xF0;
  }
}

inline void mkRP_OA_Address( ET96MAP_SM_RP_OA_T *addr, const char *saddr, unsigned len, char type) {
  unsigned i;
  int sz = (len+1)/2;
  addr->addrLen = sz+1;
  addr->typeOfAddress = type;
  addr->addr[0] = 0x91; // InterNational, ISDN
  for( i = 0; i < len; i++ ) {
    int bi = i/2;
    if( i%2 == 1 ) {
      addr->addr[bi+1] |= ((saddr[i]-'0')<<4); // fill high octet
    } else {
      addr->addr[bi+1] = (saddr[i]-'0')&0x0F; // fill low octet
    }
  }
  if( len%2 != 0 ) {
    addr->addr[sz] |= 0xF0;
  }
}

inline void ConvAddrMSISDN2Smc(const ET96MAP_SM_RP_OA_T* ma,Address* sa){
  sa->setTypeOfNumber((ma->addr[0]>>4)&0x7);
  sa->setNumberingPlan(ma->addr[0]&0xf);
  if ( ma->addrLen > 1 ){
    char sa_val[21] = {0,};
    int i = 1, len=0;
    for( i = 1; i < ma->addrLen;i++) {
      sa_val[len++]=(ma->addr[i]&0x0f)+0x30;
      if ( (ma->addr[i]&0xf0) == 0xf0 ) break;
      sa_val[len++]=((ma->addr[i]&0xf0)>>4)+0x30;
    }
    sa->setValue(len,sa_val);
  }else{
    throw runtime_error("MAP::ConvAddrMSISDN2Smc  ET96MAP_SM_RP_OA_T length should be greater than 0");
  }
}

inline void ConvAddrMSISDN2Smc(const ET96MAP_ADDRESS_T* ma,Address* sa)
{
  sa->setTypeOfNumber((ma->typeOfAddress>>4)&0x7);
  sa->setNumberingPlan(ma->typeOfAddress&0xf);
  if ( ma->addressLength == 0 ) throw runtime_error("MAP::ConvAddrMSISDN2Smc  ET96MAP_ADDRESS_T length should be greater than 0");
  char sa_val[21] = {0,};
  int i = 0;
  for ( ;i<(ma->addressLength);i++){
    if( i % 2 == 0 ) {
      sa_val[i] = (ma->address[i/2]&0x0f)+0x30;
    } else {
      sa_val[i] = (ma->address[i/2]>>4)+0x30;
    }
    if(sa_val[i] > 0x39 ) throw runtime_error("MAP::ConvAddrMSISDN2Smc numeric address contains not digit.");
  }
  sa->setValue(i,sa_val);
}

inline void ConvAddrIMSI2Smc(const ET96MAP_IMSI_T* ma,Address* sa)
{
  sa->setTypeOfNumber(0);  // unknown
  sa->setNumberingPlan(9); // reserved but we will use it for imsi
  if ( ma->imsiLen == 0 ) throw runtime_error("MAP::ConvAddrIMSI2Smc  ET96MAP_IMSI_T length should be greater than 0");
  char sa_val[21] = {0,};
  char *sa_ptr = sa_val;
  int i = 0;
  for ( ;i<(ma->imsiLen);i++){
    *sa_ptr = (ma->imsi[i]&0x0f)+0x30;
    if( *sa_ptr > 0x39 ) throw runtime_error("MAP::ConvAddrIMSI2Smc numeric address contains not digit.");
    sa_ptr++;
    *sa_ptr = (ma->imsi[i]>>4)+0x30;
    if( *sa_ptr == 0x0f+0x30 ) *sa_ptr = 0;
    else {
      if( *sa_ptr > 0x39 ) throw runtime_error("MAP::ConvAddrIMSI2Smc numeric address contains not digit.");
      sa_ptr++;
    }
  }
  sa->setValue((uint8_t)(sa_ptr-sa_val),sa_val);
}

inline void ConvAddrIMSIorMSISDN2String(const ET96MAP_IMSI_OR_MSISDN_T* ma,String32 &s)
{
  int idx=0;
  if ( ma->imsiOrMsisdnLen == 0 ) return;
  for ( int i = 1;i<(ma->imsiOrMsisdnLen) && idx<31;i++){
    s[idx++]=((ma->imsiOrMsisdn[i]&0x0f)+0x30);
    if( (ma->imsiOrMsisdn[i]&0xF0) == 0xF0 ) break;
    s[idx++]= (((ma->imsiOrMsisdn[i]>>4)&0x0f)+0x30);
  }
  s[idx]=0;
}

extern void CloseDialog(ET96MAP_LOCAL_SSN_T lssn,ET96MAP_DIALOGUE_ID_T dialogId);
extern void CloseAndRemoveDialog(ET96MAP_LOCAL_SSN_T lssn,ET96MAP_DIALOGUE_ID_T dialogId);
extern void AbortAndRemoveDialog(ET96MAP_LOCAL_SSN_T lssn,ET96MAP_DIALOGUE_ID_T dialogId);

inline void ConvAddrMap2Smc(const MAP_SMS_ADDRESS* ma,Address* sa){
  //sa->setTypeOfNumber(ma->st.ton);
  int ton=(ma->tonnpi&0x70)>>4;
  sa->setTypeOfNumber(ton);
  int npi=ma->tonnpi&0x0f;
  //sa->setNumberingPlan(ma->st.npi);
  sa->setNumberingPlan(npi);
  if ( ton == 5 )
  {
    MicroString text;
    Convert7BitToText(ma->val,(ma->len*4)/7,&text,0); // len originally specified in usefull semioctets
    if ( text.len == 0 || text.bytes[0] == 0 ) throw runtime_error("MAP::ConvAddrMap2Smc: zero address");
    if( text.bytes[text.len-1]==0x0d ) {
      text.len--;
      text.bytes[text.len]=0;
    }
    if ( text.len == 0 ) throw runtime_error("MAP::ConvAddrMap2Smc: zero address");
    sa->setValue(text.len,text.bytes);
  }
  else
  if ( ma->len != 0 ){
    char sa_val[21] = {0,};
    int i = 0;
    for ( ;i<ma->len;){
      sa_val[i]=(ma->val[(i>>1)]&0x0f)+0x30;
      if(sa_val[i] > 0x39 ) throw runtime_error("MAP::ConvAddrMap2Smc numeric address contains not digit.");
      ++i;
      if ( i < ma->len ){
        sa_val[i] = (ma->val[(i>>1)]>>4)+0x30;
        if(sa_val[i] > 0x39 ) throw runtime_error("MAP::ConvAddrMap2Smc numeric address contains not digit.");
        ++i;
      }else break;
    }
    sa->setValue(i,sa_val);
  }else{
    throw runtime_error("MAP::ConvAddrMap2Smc  MAP_SMS_ADDRESS length should be greater than 0");
  }
}

uint8_t convertCBSDatacoding2SMSC( UCHAR_T dcs, UCHAR_T *udhPresent, UCHAR_T *msgClassMean, UCHAR_T *msgClass ) {
  *udhPresent = 0;
  *msgClassMean = 0;
  *msgClass = 0;
  int codingGroup = (dcs>>4) & 0x0f;
  int codingScheme = dcs & 0x0f;
  if( codingGroup == 0 || codingGroup == 2 || codingGroup == 3 ) {
    return smsc::smpp::DataCoding::SMSC7BIT;
  } else if( codingGroup == 1 ) {
    if( codingScheme == 0 ) return smsc::smpp::DataCoding::SMSC7BIT;
    else if(codingScheme == 1) return smsc::smpp::DataCoding::UCS2;
    else {
      __map_warn2__("Strange CBS datacoding 0x%02X", (int)dcs);
      return smsc::smpp::DataCoding::SMSC7BIT;
    }
  } else if( (codingGroup & 0x0C) == 4 ) { // 7,6 bits of dcs are 01 - general datacoding
    if( (codingGroup & 0x01) == 0x01 ) {
      *msgClassMean = 1;
      *msgClass = codingScheme&0x03;
    }
    return codingScheme&0xC;
  } else if (codingGroup == 9) { // 0x1001 - msg with udhi
    *udhPresent = 1;
    *msgClassMean = 1;
    *msgClass = codingScheme&0x03;
    return codingScheme&0xC;
  } else if(codingGroup == 0x0f) {// 0x1111 - dcs/msg handling but we will use it and for UCS2
    *msgClassMean = 1;
    *msgClass = codingScheme&0x03;
    return codingScheme&0xC;
  } else {
    __map_warn2__("Strange CBS datacoding 0x%02X", (int)dcs);
    return smsc::smpp::DataCoding::SMSC7BIT;
  }
}
