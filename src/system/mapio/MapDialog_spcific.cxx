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
#define MAP_SMSC7BIT_ENCODING 0xf0
#define MAP_LATIN1_ENCODING 0x3
#define MAP_8BIT_ENCODING 0x4
#define MAP_UCS2_ENCODING 0x8

//#define MAKE_ERRORCODE(klass,code) (klass)
#define MAKE_ERRORCODE(klass,code) MAKE_COMMAND_STATUS(klass,code)
#define MAP_NETWORKERROR 1

struct MicroString{
  unsigned len;
  char bytes[256];
};

extern unsigned char lll_7bit_2_8bit[128];
extern unsigned char lll_8bit_2_7bit[256];

//list<unsigned> DialogMapContainer::dialogId_pool;

inline char GetChar(const unsigned char*& ptr,unsigned& shift){
  //__trace2__("MAP: 7bit: shift %d *ptr 0x%x",shift,*ptr);
  char val = (*ptr >> shift)&0x7f;
  if ( shift > 1 )
    val |= (*(ptr+1) << (8-shift))&0x7f;
  shift += 7;
  if ( shift >= 8 ) 
  {
    shift&=0x7;
    ++ptr;
  }
  //__trace2__("MAP: 7bit : %x",val);
  return val;
}

class VeryLongText{};

inline void PutChar(unsigned char*& ptr,unsigned& shift,unsigned char val8bit,unsigned char* ptr_end){
  if ( ptr >= ptr_end ) throw VeryLongText();
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
  __require__(chars<=255);
  unsigned shift = offset;
  for ( unsigned i=0; i< chars; ++i ){
    text->bytes[i] = GetChar(bit7buf,shift);
  }
  text->len = chars;
  text->bytes[chars] = 0;
#if !defined DISABLE_TRACING
  __trace2__("MAP::7bit(%d)->SMSC7Bit: %s",chars,text->bytes);
  {
    char b[255*4];
    unsigned k;
    unsigned i;
    for ( i=0,k=0; i<text->len;++i){
      k += sprintf(b+k,"%x ",text->bytes[i]);
    }
    __trace2__("MAP::latin1(hex): %s",b);
  }
#endif
}

inline unsigned ConvertText27bit(
  const unsigned char* text, unsigned chars, unsigned char* bit7buf,unsigned* elen,
  unsigned offset=0)
{
  __trace2__("MAP::ConvertText27bit: text %s",text);
  if ( chars > 160 ){
    __trace2__("MAP::ConvertText27bit: text length(%d) > 160",chars);
    throw runtime_error("text length > 160");
  }
  unsigned char* base = bit7buf;
  unsigned char* bit7buf_end = base+140;
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
#if !defined DISABLE_TRACING
  {
    char b[chars+1];
    memcpy(b,text,chars);
    b[chars] = 0;
    __trace2__("MAP::latin1->7bit: %s",b);
  }
  {
    char b[255*4];
    unsigned k;
    unsigned i;
    for ( i=0,k=0; i<chars;++i){
      k += sprintf(b+k,"%x ",text[i]);
    }
    __trace2__("MAP::7bit(hex): %s",b);
  }
#endif
  unsigned _7bit_len = bit7buf-base+(shift?1:0);
  __trace2__("MAP::7bit buffer length: %d",_7bit_len);
  return _7bit_len;
}

inline unsigned ConvertSMSC7bit27bit(
  const unsigned char* text, unsigned chars, unsigned char* bit7buf,
  unsigned offset=0)
{
  if ( chars > 160 ){
    __trace2__("MAP::ConvertSMSC7bit27bit: text length(%d) > 160",chars);
    throw runtime_error("text length > 160");
  }
  unsigned char* base = bit7buf;
  unsigned char* bit7buf_end = base+140;
  unsigned shift = offset;
  for ( unsigned i=0; i< chars; ++i ){
    PutChar(bit7buf,shift,text[i],bit7buf_end);
  }
#if !defined DISABLE_TRACING
  {
    char b[chars+1];
    memcpy(b,text,chars);
    b[chars] = 0;
    __trace2__("MAP::SMSC7bit->7bit: %s",b);
  }
  {
    char b[255*4];
    unsigned k;
    unsigned i;
    for ( i=0,k=0; i<chars;++i){
      k += sprintf(b+k,"%x ",text[i]);
    }
    __trace2__("MAP::7bit(hex): %s",b);
  }
#endif
  unsigned _7bit_len = bit7buf-base+(shift?1:0);
  __trace2__("MAP::7bit buffer length: %d",_7bit_len);
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

#pragma pack(1)

struct SMS_SUMBMIT_FORMAT_HEADER{ 
  union{                          
    struct{                       
      unsigned reply_path:1;      
      unsigned udhi:1;            
      unsigned srr:1;             
      unsigned tp_vp:2;           
      unsigned reject_dupl:1;     
      unsigned mg_type_ind:2;     
    };                            
    unsigned char _val_01;        
  };                              
  unsigned char mr;               
};                                
                                  
struct MAP_SMS_ADDRESS{
  unsigned char len;
  //unsigned char tonpi;
  union{
    struct{
      unsigned reserved_1:1;
      unsigned ton:3;
      unsigned npi:4;
    }st;
    unsigned char tonpi;
  };
  unsigned char val[10];
};

struct MAP_TIMESTAMP{
  struct{
    unsigned second:4;
    unsigned first:4;
  }year;
  struct{
    unsigned second:4;
    unsigned first:4;
  }mon;
  struct{
    unsigned second:4;
    unsigned first:4;
  }day;
  struct{
    unsigned second:4;
    unsigned first:4;
  }hour;
  struct{
    unsigned second:4;
    unsigned first:4;
  }min;
  struct{
    unsigned second:4;
    unsigned first:4;
  }sec;
  unsigned char tz;
};

#pragma pack()

inline void ConvAddrMap2Smc(const MAP_SMS_ADDRESS* ma,Address* sa){
  sa->setTypeOfNumber(ma->st.ton);
  sa->setNumberingPlan(ma->st.npi);
  if ( ma->len != 0 ){
    char sa_val[21] = {0,};
    int i = 0;
    for ( ;i<ma->len;){
      sa_val[i]=(ma->val[(i>>1)]&0x0f)+0x30;
      ++i;
      if ( i < ma->len ){
        sa_val[i] = (ma->val[(i>>1)]>>4)+0x30;
        ++i;
      }else break;
    }
    sa->setValue(i,sa_val);
    {
      char b[256] = {0,};
      memcpy(b,sa_val,ma->len);
      __trace2__("MAP::ConvAddrMap2Smc::adr value(%d) %s",ma->len,b);
    }
  }else{
    throw runtime_error("MAP::ConvAddrMap2Smc  MAP_SMS_ADDRESS length should be greater than 0");
  }
}

inline void mkSS7GTAddress( ET96MAP_SS7_ADDR_T *addr, const ET96MAP_ADDRESS_T *saddr, ET96MAP_LOCAL_SSN_T ssn) {
  addr->ss7AddrLen = 5+(saddr->addressLength+1)/2;
  addr->ss7Addr[0] = 0x12; // SSN & GT
  addr->ss7Addr[1] = ssn;
  addr->ss7Addr[2] = 0;
  addr->ss7Addr[3] = (saddr->typeOfAddress<<4)|(saddr->addressLength%2==0?0x02:0x01); // NP & GT coding
  addr->ss7Addr[4] = 0x04 | (saddr->addressLength%2==0?0x80:0x00); //
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

inline void ConvAddrMSISDN2Smc(const ET96MAP_SM_RP_OA_T* ma,Address* sa){
  sa->setTypeOfNumber((ma->addr[0]>>4)&0x7);
  sa->setNumberingPlan(ma->addr[0]&0xf);
  if ( ma->addrLen != 0 ){
    char sa_val[21] = {0,};
    int i = 0;
    for ( i=0; i<(ma->addrLen-1)*2;){
      if ( (ma->addr[(i>>1)+1]&0x0f) == 0xf ) break;
      sa_val[i]=(ma->addr[(i>>1)+1]&0x0f)+0x30;
      ++i;
      if ( i<(ma->addrLen-1)*2 ){
        if ( (ma->addr[(i>>1)+1]>>4) == 0xf ) break;
        sa_val[i] = (ma->addr[(i>>1)+1]>>4)+0x30;
        ++i;
      }else break;
    }
    {
      char b[256] = {0,};
      memcpy(b,sa_val,i);
      __trace2__("MAP::ConvAddrMSISDN2Smc::adr value(%d) %s",(ma->addrLen-1)*2,b);
    }
    sa->setValue(i,sa_val);
  }else{
    throw runtime_error("MAP::ConvAddrMap2Smc  ET96MAP_SM_RP_OA_T length should be greater than 0");
  }
}

inline void ConvAddrMSISDN2Smc(const ET96MAP_ADDRESS_T* ma,Address* sa)
{
  sa->setTypeOfNumber((ma->addr[0]>>4)&0x7);
  sa->setNumberingPlan(ma->addr[0]&0xf);
  if ( ma->addrLen != 0 ){
    char sa_val[21] = {0,};
    int i = 0;
    for ( i=0; i<(ma->addrLen-1)*2;){
      if ( (ma->addr[(i>>1)+1]&0x0f) == 0xf ) break;
      sa_val[i]=(ma->addr[(i>>1)+1]&0x0f)+0x30;
      ++i;
      if ( i<(ma->addrLen-1)*2 ){
        if ( (ma->addr[(i>>1)+1]>>4) == 0xf ) break;
        sa_val[i] = (ma->addr[(i>>1)+1]>>4)+0x30;
        ++i;
      }else break;
    }
    {
      char b[256] = {0,};
      memcpy(b,sa_val,i);
      __trace2__("MAP::ConvAddrMSISDN2Smc::adr value(%d) %s",(ma->addrLen-1)*2,b);
    }
    sa->setValue(i,sa_val);
  }else{
    throw runtime_error("MAP::ConvAddrMap2Smc  ET96MAP_SM_RP_OA_T length should be greater than 0");
  }
}

extern void CloseDialog(ET96MAP_LOCAL_SSN_T lssn,ET96MAP_DIALOGUE_ID_T dialogId);
extern void CloseAndRemoveDialog(ET96MAP_LOCAL_SSN_T lssn,ET96MAP_DIALOGUE_ID_T dialogId);
extern void AbortAndRemoveDialog(ET96MAP_LOCAL_SSN_T lssn,ET96MAP_DIALOGUE_ID_T dialogId);

