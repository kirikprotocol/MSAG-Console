#include "MapIoTask.h"
#include "sms/sms.h"
#include "smeman/smsccmd.h"
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
  //__trace2__("MAP: 7bit: shift %d *ptr 0x%x",shift,*ptr);
  if ( ptr >= ptr_end ) throw VeryLongText();
  unsigned char val = val8bit;
  //char val = (*ptr >> shift)&0x7f;
  *ptr = *ptr | (val << shift);
  if ( shift > 1 )
    //val |= (*(ptr+1) << (8-shift))&0x7f;
    *(ptr+1) = *(ptr+1) | (val >> (8-shift));
  shift += 7;
  if ( shift >= 8 ) 
  {
    shift&=0x7;
    ++ptr;
  }
  //__trace2__("MAP: 7bit : %x",val);
}

/*void Convert7BitToText(
  const unsigned char* bit7buf, unsigned chars,
  MicroString* text)
{
  __require__(chars<=255);
  unsigned shift = 0;
  for ( unsigned i=0; i< chars; ++i ){
    text->bytes[i] =lll_7bit_2_8bit[GetChar(bit7buf,shift)&0x7f];
  }
  text->len = chars;
  text->bytes[chars] = 0;
#if !defined DISABLE_TRACING
  __trace2__("MAP::7bit->latin1: %s",text->bytes);
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
}*/

void Convert7BitToSMSC7Bit(
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
  __trace2__("MAP::7bit->latin1: %s",text->bytes);
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

unsigned ConvertText27bit(
  const unsigned char* text, unsigned chars, unsigned char* bit7buf,unsigned* elen,
  unsigned offset=0)
{
  //__require__(chars<=255);
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

unsigned ConvertSMSC7bit27bit(
  const unsigned char* text, unsigned chars, unsigned char* bit7buf,
  unsigned offset=0)
{
  //__require__(chars<=255);
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

bool provErrCodeFatal( ET96MAP_PROV_ERR_T p ) {
  return 
  (p == 0x02 || // unsupported service
   p == 0x03 || // mystyped parametor
   p == 0x06 || // unexcpected responnse from peer
   p == 0x09 || // invalid responce recived
  (p > 0x0a && p <= 0x10)); // unxpected component end other
}

#pragma pack(1)

/*struct SMS_SUMBMIT_FORMAT_HEADER{
  union{
    struct{
      unsigned srr:1;
      unsigned udhi:1;
      unsigned reply_path:1;
      unsigned tp_vp:2;
      unsigned reject_dupl:1;
      unsigned mg_type_ind:2;
    };
    unsigned char _val_01;
  };
  unsigned char mr;
};

struct SMS_DELIVERY_FORMAT_HEADER{
  union{
    struct{
      unsigned reserved:2;
      unsigned srri:1;
      unsigned udhi:1;
      unsigned reply_path:1;
      unsigned mms:1;
      unsigned mg_type_ind:2;
    }s;
    unsigned char _val_01;
  }uu;
};*/

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


void ConvAddrMap2Smc(const MAP_SMS_ADDRESS* ma,Address* sa){
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
    //char c = 0;
    //sa->setValue(0,&c);
    throw runtime_error("MAP::ConvAddrMap2Smc  MAP_SMS_ADDRESS length should be greater than 0");
  }
}

void mkSS7GTAddress( ET96MAP_SS7_ADDR_T *addr, ET96MAP_ADDRESS_T *saddr, ET96MAP_LOCAL_SSN_T ssn) {
  //int i;
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
void mkMapAddress( ET96MAP_ADDRESS_T *addr, char *saddr, unsigned len) {
  unsigned i;
  int sz = (len+1)/2;
  addr->addressLength = len;
  addr->typeOfAddress = 0x91; // InterNational, ISDN
  for( i = 0; i < len; i++ ) {
    int bi = i/2;
    //int even = i%2;
    if( i%2 == 1 ) { 
      //even 
      addr->address[bi] |= ((saddr[i]-'0')<<4); // fill high octet
    } else {
      addr->address[bi] = (saddr[i]-'0')&0x0F; // fill low octet
    }
  }
  if( len%2 != 0 ) {
    addr->address[sz-1] |= 0xF0;
  } 
}

void ConvAddrMSISDN2Smc(const ET96MAP_SM_RP_OA_T* ma,Address* sa){
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
//    char c = 0;
//    sa->setValue(0,&c);
    throw runtime_error("MAP::ConvAddrMap2Smc  ET96MAP_SM_RP_OA_T length should be greater than 0");
  }
}

USHORT_T  MapDialog::Et96MapV2ForwardSmMOInd( 
    ET96MAP_LOCAL_SSN_T lssn, 
    ET96MAP_DIALOGUE_ID_T dialogId,
    ET96MAP_INVOKE_ID_T invokeId, 
    ET96MAP_SM_RP_DA_T* dstAddr, 
    ET96MAP_SM_RP_OA_T* srcAddr,  
    ET96MAP_SM_RP_UI_T* ud )
{
#if defined USE_MAP
  try{
    __trace2__("MAP::MapDialog::Et96MapV2ForwardSmMOInd dta len %d",ud->signalInfoLen);
    setInvokeId(invokeId);
    SMS sms;
    Address src_addr;
    Address dest_addr;
    SMS_SUMBMIT_FORMAT_HEADER* ssfh = (SMS_SUMBMIT_FORMAT_HEADER*)ud->signalInfo;
    __trace2__("MAP::DIALOG::ForwardReaq: MR(8) = 0x%x",ssfh->mr);
    __trace2__("MAP::DIALOG::ForwardReaq: MSG_TYPE_IND(2) = 0x%x",ssfh->mg_type_ind);
    __trace2__("MAP::DIALOG::ForwardReaq: MSG_VPF(2) = 0x%x",ssfh->tp_vp);
    __trace2__("MAP::DIALOG::ForwardReaq: MSG_UDHI(1) = %d",ssfh->udhi);
    __trace2__("MAP::DIALOG::ForwardReaq: MSG_REJECT_DUPL(1) = %d",ssfh->reject_dupl);
    __trace2__("MAP::DIALOG::ForwardReaq: MSG_REPLY_PATH(1) = %d",ssfh->reply_path);
    __trace2__("MAP::DIALOG::ForwardReaq: MSG_SRR(1) = %d",ssfh->srr);
    MAP_SMS_ADDRESS* msa = (MAP_SMS_ADDRESS*)(ud->signalInfo+2);
    __trace2__("MAP::DIALOG::ForwardReaq: MSA tonpi.va(8):0x%x, ton(3):0x%x, npi(4):0x%x, len(8):%d",
               msa->tonpi,msa->st.ton,msa->st.npi,msa->len);
    //__trace2__("MAP::DIALOG::ForwardReaq: user_data_len = user_data_len");
    unsigned msa_len = msa->len/2+msa->len%2+2;
    unsigned char protocol_id = *(unsigned char*)(ud->signalInfo+2+msa_len);
    __trace2__("MAP::DIALOG::ForwardReaq: protocol_id = 0x%x",protocol_id);
    unsigned char user_data_coding = *(unsigned char*)(ud->signalInfo+2+msa_len+1);
    __trace2__("MAP::DIALOG::ForwardReaq: user_data_encoding = 0x%x",user_data_coding);
    unsigned char user_data_len = *(unsigned char*)(ud->signalInfo+2+((ssfh->tp_vp==0||ssfh->tp_vp==2)?1:7)+msa_len+2);
    __trace2__("MAP::DIALOG::ForwardReaq: user_data_len = %d",user_data_len);
    unsigned char* user_data = (unsigned char*)(ud->signalInfo+2+((ssfh->tp_vp==0||ssfh->tp_vp==2)?1:7)+msa_len+2+1);
    unsigned encoding = 0;
    if ( (user_data_coding & 0xc0) == 0 ||  // 00xxxxxx
         (user_data_coding & 0xc0) == 0x40 )  // 01xxxxxx
    {
      if ( user_data_coding&(1<<5) ){
        __trace2__("MAP::DIALOG::ForwardReq: required compression");
        throw runtime_error("MAP::DIALOG::ForwardReq: required compression");
      }
      encoding = user_data_coding&0x0c;
      if ( (user_data_coding & 0xc0) == 0x40 )
        sms.setIntProperty(Tag::MS_VALIDITY,0x03);
    }
    else if ( (user_data_coding & 0xf0) == 0xc0 ) // 1100xxxx
    {
      encoding = MAP_OCTET7BIT_ENCODING;
      sms.setIntProperty(Tag::MS_VALIDITY,0x3);
      sms.setIntProperty(Tag::SMPP_MS_MSG_WAIT_FACILITIES,
                         (user_data_coding&0x3)|((user_data_coding&0x8)<<4));
    }
    else if ( (user_data_coding & 0xf0) == 0xd0 ) // 1101xxxx
    {
      encoding = MAP_OCTET7BIT_ENCODING;
      sms.setIntProperty(Tag::MS_VALIDITY,0x0);
      sms.setIntProperty(Tag::SMPP_MS_MSG_WAIT_FACILITIES,
                         (user_data_coding&0x3)|((user_data_coding&0x8)<<4));
    }
    else if ( (user_data_coding & 0xf0) == 0xe0 ) // 1110xxxx
    {
      encoding = MAP_UCS2_ENCODING;
      sms.setIntProperty(Tag::MS_VALIDITY,0x0);
      sms.setIntProperty(Tag::SMPP_MS_MSG_WAIT_FACILITIES,
                         (user_data_coding&0x3)|((user_data_coding&0x8)<<4));
    }
    else if ( (user_data_coding & 0xf0) == 0xf0 ) // 1111xxxx
    {
      if ( user_data_coding & 0x4 ) encoding = MAP_8BIT_ENCODING;
      else encoding = MAP_OCTET7BIT_ENCODING;
      sms.setIntProperty(Tag::MS_DESTADDRSUBUNIT,user_data_coding&0x3);
    }
    else{
      __trace2__("MAP::DIALOG::ForwardReq: unknown coding scheme 0x%x",user_data_coding);
      throw runtime_error("unknown coding scheme");
    }
    {
      if (  encoding == MAP_OCTET7BIT_ENCODING ){
        if ( ssfh->udhi){
          MicroString ms;
          auto_ptr<unsigned char> b(new unsigned char[255*2]);
          unsigned udh_len = ((unsigned)*user_data)&0x0ff;
          __trace2__("MAP::DIALOG::ForwardReq: ud_length 0x%x",user_data_len);
          __trace2__("MAP::DIALOG::ForwardReq: udh_len 0x%x",udh_len);
          unsigned x = (udh_len+1)*8;
          if ( x%7 != 0 ) x+=7-(x%7);
          unsigned symbols = user_data_len-x/7;
          __trace2__("MAP::DIALOG::ForwardReq: text symbols 0x%x",symbols);
          __trace2__("MAP::DIALOG::ForwardReq: text bit offset 0x%x",x-(udh_len+1)*8);
          Convert7BitToSMSC7Bit(user_data+udh_len+1,symbols,&ms,x-(udh_len+1)*8);
          memcpy(b.get(),user_data,udh_len+1);
          memcpy(b.get()+udh_len+1,ms.bytes,ms.len);
          sms.setBinProperty(Tag::SMPP_SHORT_MESSAGE,(char*)b.get(),udh_len+1+symbols);
          sms.setIntProperty(Tag::SMPP_SM_LENGTH,udh_len+1+symbols);
        }else{
          MicroString ms;
          //Convert7BitToText(user_data,user_data_len,&ms);
          Convert7BitToSMSC7Bit(user_data,user_data_len,&ms,0);
          sms.setBinProperty(Tag::SMPP_SHORT_MESSAGE,ms.bytes,ms.len);
          sms.setIntProperty(Tag::SMPP_SM_LENGTH,ms.len);
        }
        sms.setIntProperty(Tag::SMPP_DATA_CODING,(unsigned)MAP_SMSC7BIT_ENCODING);
      }
      else{
        sms.setBinProperty(Tag::SMPP_SHORT_MESSAGE,(const char*)user_data,user_data_len);
        sms.setIntProperty(Tag::SMPP_SM_LENGTH,user_data_len);
        sms.setIntProperty(Tag::SMPP_DATA_CODING,(unsigned)encoding);
      }
    }
    unsigned esm_class = 0;
    esm_class |= (ssfh->udhi?0x40:0);
    esm_class |= (ssfh->reply_path?0x80:0);
    sms.setIntProperty(Tag::SMPP_ESM_CLASS,esm_class);
    sms.setIntProperty(Tag::SMPP_SM_LENGTH,user_data_len);
    sms.setIntProperty(Tag::SMPP_PROTOCOL_ID,protocol_id);
    sms.setMessageReference(ssfh->mr);
    //if ( ssfh->srr ) sms.setDeliveryReport(1);
    if ( ssfh->srr ) sms.setIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST,1);
    ConvAddrMSISDN2Smc(srcAddr,&src_addr);
    sms.setOriginatingAddress(src_addr);
    ConvAddrMap2Smc(msa,&dest_addr);
    sms.setDestinationAddress(dest_addr);
    MapProxy* proxy = MapDialogContainer::getInstance()->getProxy();
    SmscCommand cmd = SmscCommand::makeSumbmitSm(sms,((uint32_t)dialogId)&0xffff);
    proxy->putIncomingCommand(cmd);
    state = MAPST_WAIT_SUBMIT_RESPONSE;
    __trace2__("MAP::MapDialog::Et96MapV2ForwardSmMOInd OK");
  }catch(...){
    __trace2__("MAP::MapDialog::Et96MapV2ForwardSmMOInd exception catched");
  }
#endif
  return ET96MAP_E_OK;
}

#if 0

ET96MAP_SM_RP_UI_T* mkDeliverPDU(SMS* sms,ET96MAP_SM_RP_UI_T* pdu)
{
#if defined USE_MAP
  memset(pdu,0,sizeof(ET96MAP_SM_RP_UI_T));
  SMS_DELIVERY_FORMAT_HEADER* header = (SMS_DELIVERY_FORMAT_HEADER*)pdu->signalInfo;
  header->uu.s.mg_type_ind = 0;
  header->uu.s.mms = 0;
  header->uu.s.reply_path = (sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x80)?1:0;;
  header->uu.s.srri = 0;
  header->uu.s.udhi = (sms->getIntProperty(Tag::SMPP_ESM_CLASS)&0x40)?1:0;
  MAP_SMS_ADDRESS* oa = (MAP_SMS_ADDRESS*)(pdu->signalInfo+1);
  oa->st.ton = sms->getOriginatingAddress().getTypeOfNumber();
  oa->st.npi = sms->getOriginatingAddress().getNumberingPlan();
  oa->st.reserved_1 = 1;
  oa->len = sms->getOriginatingAddress().getLength();
  unsigned oa_length = (oa->len+1)/2; 
  __trace2__("MAP::mkDeliverPDU: oa_length: 0x%x", oa_length);
  {
    char* sval = sms->getOriginatingAddress().value;
    for ( int i=0; i<oa->len; ++i ){
      int bi = i/2;
      if( i%2 == 1 ){ 
        oa->val[bi] |= ((sval[i]-'0')<<4); // fill high octet
      }else{
        oa->val[bi] = (sval[i]-'0')&0x0F; // fill low octet
      }
    }
    if( oa->len%2 != 0 ) oa->val[oa_length-1] |= 0xF0;
  }
  unsigned char *pdu_ptr = pdu->signalInfo+1+2+oa_length;
  *pdu_ptr++ = (unsigned char)sms->getIntProperty(Tag::SMPP_PROTOCOL_ID);
  unsigned encoding = sms->getIntProperty(Tag::SMPP_DATA_CODING);
  __trace2__("MAP::mkDeliveryPDU: encoding = 0x%x",encoding);
  unsigned char value;
  if ( encoding != 0 && 
       encoding != 0x08 && 
       encoding != 0x03 && 
       encoding != 0x04 && 
       encoding != 0xf0) {
    __trace2__("MAP::mkDeliverPDU: unsuppprted encoding 0x%x",encoding);
    throw runtime_error("unsupported encoding");
  }
  else // make coding scheme
  {
    if ( sms->hasIntProperty(Tag::MS_DESTADDRSUBUNIT) ){
      __trace2__("MAP::mkDeliveryPDU: dest_addr_subunit = 0x%x",
                 sms->getIntProperty(Tag::MS_DESTADDRSUBUNIT));
      // coding scheme 1111xxxx
      value = 0xf0;
      if ( encoding == MAP_UCS2_ENCODING){
        __trace2__("MAP::mkDeliveryPDU: coding group 1111xxxx could'not has USC2");
        throw runtime_error("MAP::mkDeliveryPDU: coding group 1111xxxx incompatible with encoding UCS2");
      }
      if ( encoding == MAP_OCTET7BIT_ENCODING || encoding == MAP_LATIN1_ENCODING )
        ;//value |=  nothing
      else // 8bit
        value |= (1<<2);
      value |= sms->getIntProperty(Tag::MS_DESTADDRSUBUNIT)&0x3;
    }
    else
    {
      if ( sms->hasIntProperty(Tag::SMPP_MS_MSG_WAIT_FACILITIES) ){
        __trace2__("MAP::mkDeliveryPDU: ms_msg_wait_facilities = 0x%x",
                   sms->getIntProperty(Tag::SMPP_MS_MSG_WAIT_FACILITIES));
        if ( encoding == MAP_UCS2_ENCODING ){
          value = 0xe0;
          unsigned _val = sms->getIntProperty(Tag::SMPP_MS_MSG_WAIT_FACILITIES);
          value |= _val&0x3;
          if ( _val&0x80 )value |= 0x8;
        }
        else
        {
          if ( !sms->hasIntProperty(Tag::MS_VALIDITY) ){
            __trace2__("MAP::mkDeliveryPDU: Opss, has no ms_validity");
            throw runtime_error("MAP::mkDeliveryPDU: Opss, has no ms_validity");
          }
          unsigned ms_validity = sms->getIntProperty(Tag::MS_VALIDITY);
          __trace2__("MAP::mkDeliveryPDU: ms_validity = 0x%x",
                     ms_validity);
          if ( (ms_validity & 0x3) == 0x3 ){
            value = 0xc0;
          }else if ( (ms_validity & 0x3) == 0 ){
            value = 0xd0;
          }else{
            __trace2__("MAP::mkDeliveryPDU: Opss, ms_validity = 0x%x but must be 0x0 or 0x3",
                       ms_validity);
            throw runtime_error("bad ms_validity value");
          }
          unsigned _val = sms->getIntProperty(Tag::SMPP_MS_MSG_WAIT_FACILITIES);
          value |= _val&0x3;
          if ( _val&0x80 )value |= 0x8;
        }
      }
      else
      {
        /*if ( !sms->hasIntProperty(Tag::MS_VALIDITY) ){
          __trace2__("MAP::mkDeliveryPDU: Opss, has no ms_validity");
          throw runtime_error("MAP::mkDeliveryPDU: Opss, has no ms_validity");
        }*/
        unsigned ms_validity = 0;
        if ( sms->hasIntProperty(Tag::MS_VALIDITY) )
          ms_validity = sms->getIntProperty(Tag::MS_VALIDITY);
        __trace2__("MAP::mkDeliveryPDU: ms_validity = 0x%x",
                   ms_validity);
        if ( (ms_validity & 0x3) == 0x3 ){
          __trace2__("MAP::mkDeliveryPDU: (validity & 0x3) == 0x3");
          value = 0x40;
        }else if ( (ms_validity & 0x3) == 0 ){
          __trace2__("MAP::mkDeliveryPDU: (validity & 0x3) == 0");
          value = 0x00;
        }else{
          __trace2__("MAP::mkDeliveryPDU: Opss, ms_validity = 0x%x but must be 0x0 or 0x3",
                     ms_validity);
          throw runtime_error("bad ms_validity value");
        }
        if ( encoding == MAP_UCS2_ENCODING ){
          __trace2__("MAP::mkDeliveryPDU: MAP_UCS2_ENCODING");
           value |= 0x8;
        }
        //if ( encoding == 0 && encoding == 0x3 ) nothing
        if ( encoding == MAP_8BIT_ENCODING ){
          __trace2__("MAP::mkDeliveryPDU: MAP_8BIT_ENCODING");
           value |= 0x4;
        }
      }
    }
    __trace2__("MAP::mkDeliveryPDU: user data coding = 0x%x",(unsigned)value);
    *pdu_ptr++ = value;
  }
  {
    time_t t;
    time(&t);
    struct tm* tms = localtime(&t);  
    MAP_TIMESTAMP* pdu_tm = (MAP_TIMESTAMP*)pdu_ptr;
    __trace2__("MAP::mkDeliverPDU:sizeof(pdu_tm) %d",sizeof(*pdu_tm));
    pdu_tm->year.first  =  ((tms->tm_year)%100)/10;
    pdu_tm->year.second  = tms->tm_year%10;
    //__trace2__("MAP::mkDeliverPDU: year: 0x%x, tms_year 0x%lx",pdu_tm->year,tms->tm_year);
    pdu_tm->mon.first  =  (tms->tm_mon+1)/10;
    pdu_tm->mon.second  = (tms->tm_mon+1)%10;
    pdu_tm->day.first  =  tms->tm_mday/10;
    pdu_tm->day.second  = tms->tm_mday%10;
    pdu_tm->hour.first  =  tms->tm_hour/10;
    pdu_tm->hour.second  = tms->tm_hour%10;
    pdu_tm->min.first  =  tms->tm_min/10;
    pdu_tm->min.second  = tms->tm_min%10;
    pdu_tm->sec.first  =  tms->tm_sec/10;
    pdu_tm->sec.second  = tms->tm_sec%10;
    int tz = timezone;
    if ( tms->tm_isdst ) tz-=3600;
    tz = -tz/900;
    __trace2__("MAP::mkDeliverPDU: timezone %d, %ld",tz,timezone);
    pdu_tm->tz = tz;
    pdu_ptr+=sizeof(MAP_TIMESTAMP);
  }
  if ( encoding == MAP_OCTET7BIT_ENCODING  || encoding == MAP_LATIN1_ENCODING ){
    unsigned text_len;
    const unsigned char* text = (const unsigned char*)sms->getBinProperty(Tag::SMPP_SHORT_MESSAGE,&text_len);
    unsigned elen;
    unsigned bytes;
    bytes = ConvertText27bit(text,text_len,pdu_ptr+1,&elen);
    *pdu_ptr++ = elen;
    pdu_ptr += bytes;
  }else if ( encoding == MAP_SMSC7BIT_ENCODING  ){ // 7bit
      unsigned text_len;
      const unsigned char* text = (const unsigned char*)sms->getBinProperty(Tag::SMPP_SHORT_MESSAGE,&text_len);
      if ( header->uu.s.udhi ){
        unsigned udh_len = (unsigned)*text;
        __trace2__("MAP::mkDeliverPDU: udh_len 0x%x",udh_len);
        memcpy(pdu_ptr+1,text,udh_len+1);
        unsigned x = (udh_len+1)*8;
        if ( x%7 != 0 ) x+=7-(x%7);
        unsigned symbols = text_len-udh_len-1;
        __trace2__("MAP::mkDeliverPDU: text symbols 0x%x",symbols);
        __trace2__("MAP::mkDeliverPDU: text bit offset 0x%x",x-(udh_len+1)*8);
        unsigned _7bit_text_len = ConvertSMSC7bit27bit(
          text+1+udh_len,
          symbols,
          pdu_ptr+udh_len+1+1,
          x-(udh_len+1)*8);
        *pdu_ptr++ = x/7+text_len+1;
        pdu_ptr+= udh_len+_7bit_text_len+1;
      }else{
        *pdu_ptr++ = text_len;
        pdu_ptr += ConvertSMSC7bit27bit(text,text_len,pdu_ptr);
      }
  }else{ // UCS2 || 8BIT
    unsigned text_len;
    const unsigned char* text = (const unsigned char*)sms->getBinProperty(Tag::SMPP_SHORT_MESSAGE,&text_len);
    //unsigned size_x = /*pdu_ptr-(unsigned char*)pdu->signalInfo*;
    if ( text_len > 140 ){
      __trace2__("MAP::mkDeliverPDU:  UCS2 text length(%d) > 140",
                text_len);
      throw runtime_error("MAP::mkDeliverPDU:  UCS2 text length > pdu_ptr-pdu->signalInfoLen");
    }
    memcpy(pdu_ptr+1,text,text_len);
    *pdu_ptr++ = text_len;
    pdu_ptr += text_len;
  }
  pdu->signalInfoLen  = pdu_ptr-(unsigned char*)pdu->signalInfo;
  //if ( pdu->signalInfoLen > 140 ) header->uu.s.mms = 1;
  __trace2__("MAP::mkDeliverPDU: signalInfoLen 0x%x",pdu->signalInfoLen);
  {
    char text[sizeof(*pdu)*4] = {0,};
    int k = 0;
    for ( int i=0; i<pdu->signalInfoLen; ++i){
      k+=sprintf(text+k,"%02x ",(unsigned)pdu->signalInfo[i]);
    }
    __trace2__("MAP::mkDeliverPDU: PDU %s",text);
  }
  return pdu;
#else
  return 0;
#endif
}

#endif

#include "MapDialogMkPDU.cxx"

void MapDialog::Et96MapDelimiterInd(
  ET96MAP_LOCAL_SSN_T lssn,
  ET96MAP_DIALOGUE_ID_T dialogId,
  UCHAR_T priorityOrder)
{
#if defined USE_MAP
  __trace2__("MAP::MapDialog::Et96MapDelimiterInd");
  if ( sms.get() != 0 ){
  __trace2__("MAP::DELIVERY_SM %d.%d.%s -> %d.%d.%s",
             sms->getOriginatingAddress().getTypeOfNumber(),
             sms->getOriginatingAddress().getNumberingPlan(),
             sms->getOriginatingAddress().value,
             sms->getDestinationAddress().getTypeOfNumber(),
             sms->getDestinationAddress().getNumberingPlan(),
             sms->getDestinationAddress().value);
  }
  if ( state == MAPST_START ){
    __trace2__("MAP::MapDialog::Et96MapDelimiterInd: send Req");
    Et96MapDelimiterReq(ssn,dialogId,priorityOrder,0);
  }
  else if ( state == MAPST_WAIT_SEGMINTATION )
  {
    USHORT_T result;
    state = MAPST_READY_FOR_CLOSE;
    __trace2__("MAP::Et96MapCloseInd:Et96MapV2ForwardSmMTReq");
    result = Et96MapV2ForwardSmMTReq( SSN, dialogid, 1, &smRpDa, &smRpOa, auto_ui.get(), FALSE);
    if( result != ET96MAP_E_OK ) {
      __trace2__("MAP::Et96MapCloseInd:Et96MapV2ForwardSmMTReq error 0x%x",result);
    }
    __trace2__("MAP::Et96MapCloseInd:Et96MapV2ForwardSmMTReq OK");
    result = Et96MapDelimiterReq( SSN, dialogid, 0, 0 );
    if( result != ET96MAP_E_OK ) {
      __trace2__("MAP::Et96MapCloseInd:Et96MapDelimiterReq error 0x%x",result);
    }
    __trace2__("MAP::send response to SMSC");
  }
#endif
}

void MapDialog::Et96MapPAbortInd(
  ET96MAP_LOCAL_SSN_T lssn,
  ET96MAP_DIALOGUE_ID_T dialogid,
  ET96MAP_PROV_REASON_T reason,
  ET96MAP_SOURCE_T source,
  UCHAR_T priorityOrder)
{
#if defined USE_MAP
  __trace2__("MAP::MapDialog::Et96MapPAbortInd");
  state = MAPST_READY_FOR_CLOSE;
#endif
}

void MapDialog::Et96MapUAbortInd(
  ET96MAP_LOCAL_SSN_T lssn,
  ET96MAP_DIALOGUE_ID_T dialogid,
  ET96MAP_USER_REASON_T *reason,
  ET96MAP_DIAGNOSTIC_INFO_T* diag,
  ET96MAP_USERDATA_T *ud,
  UCHAR_T priorityOrder)
{
#if defined USE_MAP
  __trace2__("MAP::MapDialog::Et96MapUAbortInd");
  state = MAPST_READY_FOR_CLOSE;
#endif
}

bool  MapDialog::Et96MapCloseInd(ET96MAP_LOCAL_SSN_T,
                         ET96MAP_DIALOGUE_ID_T,
                         ET96MAP_USERDATA_T *,
                         UCHAR_T priorityOrder)
{
#if defined USE_MAP
  __trace2__("MAP::Et96MapCloseInd state: 0x%x",state);
  if ( sms.get() != 0 ){
  __trace2__("MAP::DELIVERY_SM %d.%d.%s -> %d.%d.%s",
             sms->getOriginatingAddress().getTypeOfNumber(),
             sms->getOriginatingAddress().getNumberingPlan(),
             sms->getOriginatingAddress().value,
             sms->getDestinationAddress().getTypeOfNumber(),
             sms->getDestinationAddress().getNumberingPlan(),
             sms->getDestinationAddress().value);
  }
  if ( state == MAPST_READY_FOR_SENDSMS ){
    state = MAPST_READY_FOR_CLOSE;
    try{
      __trace2__("MAP::Et96MapCloseInd state: REDY_FOR_SEND_SMS");

      MapDialogContainer::getInstance()->reAssignDialog(dialogid);
      __trace2__("MAP::Et96MapCloseInd state: 0x%x",dialogid);
      
      ET96MAP_APP_CNTX_T appContext;
    	appContext.acType = ET96MAP_SHORT_MSG_MT_RELAY;
    	appContext.version = ET96MAP_APP_CNTX_T::ET96MAP_VERSION_2;
  
      USHORT_T result;
      
      result = Et96MapOpenReq(ssn, dialogid, &appContext, &destMscAddr, &scAddr, 0, 0, 0 );
      if ( result != ET96MAP_E_OK ) {
        __trace2__("MAP::MapDialog::Et96MapCloseInd Et96MapOpenReq error 0x%x",result);
        throw runtime_error("MAP::MapDialog::Et96MapCloseInd Et96MapOpenReq error");
      }
  
  	  smRpOa.typeOfAddress = ET96MAP_ADDRTYPE_SCADDR;
  	  smRpOa.addrLen = (m_scAddr.addressLength+1)/2+1;
  	  smRpOa.addr[0] = m_scAddr.typeOfAddress;
  	  memcpy( smRpOa.addr+1, m_scAddr.address, (m_scAddr.addressLength+1)/2 );
  
      //auto_ptr<ET96MAP_SM_RP_UI_T> ui(mkDeliverPDU(sms.get()));// = mkDeliverPDU( oaddress, message ); 
      ET96MAP_SM_RP_UI_T* ui;
      auto_ui = auto_ptr<ET96MAP_SM_RP_UI_T>(ui=new ET96MAP_SM_RP_UI_T);
      mkDeliverPDU(sms.get(),ui);
  
      if ( ui->signalInfoLen > 98 ) {
        __trace2__("MAP::Et96MapCloseInd:Et96MapDelimiterReq");
        result = Et96MapDelimiterReq( SSN, dialogid, 0, 0 );
        if( result != ET96MAP_E_OK ) {
          __trace2__("MAP::Et96MapCloseInd:Et96MapDelimiterReq error 0x%x",result);
        }
        state = MAPST_WAIT_SEGMINTATION;
      }else{
        __trace2__("MAP::Et96MapCloseInd:Et96MapV2ForwardSmMTReq");
    	  result = Et96MapV2ForwardSmMTReq( SSN, dialogid, 1, &smRpDa, &smRpOa, auto_ui.get(), FALSE);
    	  if( result != ET96MAP_E_OK ) {
          __trace2__("MAP::Et96MapCloseInd:Et96MapV2ForwardSmMTReq error 0x%x",result);
    	  }
        __trace2__("MAP::Et96MapCloseInd:Et96MapV2ForwardSmMTReq OK");
      	result = Et96MapDelimiterReq( SSN, dialogid, 0, 0 );
        if( result != ET96MAP_E_OK ) {
          __trace2__("MAP::Et96MapCloseInd:Et96MapDelimiterReq error 0x%x",result);
        }
        //__trace2__("MAP::send response to SMSC OK");
      }
      //return true;// :) optimization
      return false;
    }catch(VeryLongText&){
      __trace2__("MAP::Et96MapCloseInd VeryLongText");
      SmscCommand cmd = SmscCommand::makeDeliverySmResp("0",this->smscDialogId,MAKE_ERRORCODE(CMD_ERR_FATAL,MAP_NETWORKERROR));
      MapDialogContainer::getInstance()->getProxy()->putIncomingCommand(cmd);
      return true;
    }catch(exception& e){
      __trace2__("MAP::Et96MapCloseInd exception %s",e.what());
      SmscCommand cmd = SmscCommand::makeDeliverySmResp("0",this->smscDialogId,MAKE_ERRORCODE(CMD_ERR_FATAL,MAP_NETWORKERROR));
      MapDialogContainer::getInstance()->getProxy()->putIncomingCommand(cmd);
      return true;
    }catch(...){
      __trace2__("MAP::Et96MapCloseInd unknown exception");
      SmscCommand cmd = SmscCommand::makeDeliverySmResp("0",this->smscDialogId,MAKE_ERRORCODE(CMD_ERR_FATAL,MAP_NETWORKERROR));
      MapDialogContainer::getInstance()->getProxy()->putIncomingCommand(cmd);
      return true;
    }
  }
  else if (state == MAPST_READY_FOR_CLOSE)
  {
    __trace2__("MAP::Et96MapCloseInd ready for close");
    return true;
  }
  else
  {
    __trace2__("MAP::Et96MapCloseInd Opss, strange close, did 0x%x,smscDid 0x%x, state 0x%x",
               dialogid,
               smscDialogId,
               state);
    //throw "";
    return true; // drop dialog
    //return false;
  }
#else
  return false;
#endif
}

void MapDialog::Et96MapV2ForwardSmMTConf (
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogid,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_ERROR_FORW_SM_MT_T *errorForwardSMmt_sp,
  ET96MAP_PROV_ERR_T *provErrCode_p)
{
#if defined USE_MAP
  if ( sms.get() != 0 ){
  __trace2__("MAP::DELIVERY_SM %d.%d.%s -> %d.%d.%s",
             sms->getOriginatingAddress().getTypeOfNumber(),
             sms->getOriginatingAddress().getNumberingPlan(),
             sms->getOriginatingAddress().value,
             sms->getDestinationAddress().getTypeOfNumber(),
             sms->getDestinationAddress().getNumberingPlan(),
             sms->getDestinationAddress().value);
  }
  if ( errorForwardSMmt_sp ){
    __trace2__("MAP::Et96MapV2ForwardSmMTConf:did 0x%x errorForwardSMmt_sp->errorCode 0x%x",dialogid,errorForwardSMmt_sp->errorCode);
    bool fatal = false;
    switch( errorForwardSMmt_sp->errorCode ){
    case 27: /*Absent*/
    case 31: /*Busy*/
      fatal = false;
      break;
    case 32: /*Delivery failure*/
      fatal = false;
      break;
    case 34: /*System failure*/
      fatal = false;
      break;
    default:
      fatal = true;
    }
    SmscCommand cmd;
    if ( fatal ) {
      cmd = SmscCommand::makeDeliverySmResp("0",this->smscDialogId,MAKE_ERRORCODE(CMD_ERR_FATAL,MAP_NETWORKERROR));
    }else{
      cmd = SmscCommand::makeDeliverySmResp("0",this->smscDialogId,MAKE_ERRORCODE(CMD_ERR_TEMP,MAP_NETWORKERROR));
    }
    MapDialogContainer::getInstance()->getProxy()->putIncomingCommand(cmd);
    __trace2__("MAP::Et96MapV2ForwardSmMTConf:did 0x%x/0x%x was send %s to SMSC",dialogid,smscDialogId, 
               fatal?"CMD_ERR_FATAL":"CMD_ERR_TEMP");
  }else if (provErrCode_p){
    __trace2__("MAP::Et96MapV2ForwardSmMTConf:did 0x%x/0x%x *provErrCode_p 0x%x",dialogid,smscDialogId,*provErrCode_p);
    SmscCommand cmd;
    cmd = SmscCommand::makeDeliverySmResp("0",this->smscDialogId,MAKE_ERRORCODE(CMD_ERR_TEMP,MAP_NETWORKERROR));
    MapDialogContainer::getInstance()->getProxy()->putIncomingCommand(cmd);
    __trace2__("MAP::Et96MapV2ForwardSmMTConf:did 0x%x/0x%x was send CMD_ERR_TEMP to SMSC",dialogid,smscDialogId);
  }else{
    SmscCommand cmd = SmscCommand::makeDeliverySmResp("0",this->smscDialogId,MAKE_ERRORCODE(CMD_OK,0));
    MapDialogContainer::getInstance()->getProxy()->putIncomingCommand(cmd);
    __trace2__("MAP::Et96MapV2ForwardSmMTConf:did 0x%x/0x%x was send OK to SMSC",dialogid,smscDialogId);
  }
#endif
}

USHORT_T  MapDialog::Et96MapV2SendRInfoForSmConf ( ET96MAP_LOCAL_SSN_T localSsn,
				       ET96MAP_DIALOGUE_ID_T dialogueId,
				       ET96MAP_INVOKE_ID_T invokeId,
				       ET96MAP_IMSI_T *imsi_sp,
				       ET96MAP_ADDRESS_T *mscNumber_sp,
				       ET96MAP_LMSI_T *lmsi_sp,
				       ET96MAP_ERROR_ROUTING_INFO_FOR_SM_T *errorSendRoutingInfoForSm_sp,
				       ET96MAP_PROV_ERR_T *provErrCode_p )
{
#if defined USE_MAP
  __trace2__( "Et96MapV2SendRInfoForSmConf received ssn=%d, dialog=%d, invokeId=%d\n", localSsn, dialogueId, invokeId );
  if ( sms.get() != 0 ){
  __trace2__("MAP::DELIVERY_SM %d.%d.%s -> %d.%d.%s",
             sms->getOriginatingAddress().getTypeOfNumber(),
             sms->getOriginatingAddress().getNumberingPlan(),
             sms->getOriginatingAddress().value,
             sms->getDestinationAddress().getTypeOfNumber(),
             sms->getDestinationAddress().getNumberingPlan(),
             sms->getDestinationAddress().value);
  }
  state = MAPST_RINFOIND;
  if ( errorSendRoutingInfoForSm_sp != 0 ){
    bool fatal = false;
    switch( errorSendRoutingInfoForSm_sp->errorCode){
    case 1: 
    case 11:
    case 13:
    case 21:
      fatal = true;
    case 6:
      fatal = false;
    default:    
      fatal = true;
    }
    SmscCommand cmd;
    if ( fatal ) {
      cmd = SmscCommand::makeDeliverySmResp("0",this->smscDialogId,MAKE_ERRORCODE(CMD_ERR_FATAL,MAP_NETWORKERROR));
    }else{
      cmd = SmscCommand::makeDeliverySmResp("0",this->smscDialogId,MAKE_ERRORCODE(CMD_ERR_TEMP,MAP_NETWORKERROR));
    }
    MapDialogContainer::getInstance()->getProxy()->putIncomingCommand(cmd);
    __trace2__("MAP::Et96MapV2SendRInfoForSmConf errorSendRoutingInfoForSm_sp->errorCode 0x%hx",
               errorSendRoutingInfoForSm_sp->errorCode);
    throw runtime_error("MAP::Et96MapV2SendRInfoForSmConf error");
  }
  if ( provErrCode_p != 0 ){
    // error hadling
    SmscCommand cmd;
    if ( *provErrCode_p == 0x02 || // unsupported service
         *provErrCode_p == 0x03 || // mystyped parametor
         *provErrCode_p == 0x06 || // unexcpected responnse from peer
         *provErrCode_p == 0x09 || // invalid responce recived
         (*provErrCode_p > 0x0a && *provErrCode_p <= 0x10)) // unxpected component end other
    {
      cmd = SmscCommand::makeDeliverySmResp("0",this->smscDialogId,MAKE_ERRORCODE(CMD_ERR_FATAL,MAP_NETWORKERROR));
    }else{
      cmd = SmscCommand::makeDeliverySmResp("0",this->smscDialogId,MAKE_ERRORCODE(CMD_ERR_TEMP,MAP_NETWORKERROR));
    }
    MapDialogContainer::getInstance()->getProxy()->putIncomingCommand(cmd);
    __trace2__("MAP::Et96MapV2SendRInfoForSmConf provErrCode_p 0x%hx",*provErrCode_p);
    throw runtime_error("MAP::Et96MapV2SendRInfoForSmConf error");
  }
  
  mkSS7GTAddress( &destMscAddr, mscNumber_sp, 8 );
	smRpDa.typeOfAddress = ET96MAP_ADDRTYPE_IMSI;
	smRpDa.addrLen = imsi_sp->imsiLen;
	memcpy( smRpDa.addr, imsi_sp->imsi, imsi_sp->imsiLen );
  
  state = MAPST_READY_FOR_SENDSMS;

#endif
  return ET96MAP_E_OK;
}				       

void  MapDialog::Et96MapOpenConf (
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_OPEN_RESULT_T openResult,
  ET96MAP_REFUSE_REASON_T *refuseReason_p,
  ET96MAP_SS7_ADDR_T *respondingAddr_sp,
  ET96MAP_APP_CNTX_T *appContext_sp,
  ET96MAP_USERDATA_T *specificInfo_sp,
  ET96MAP_PROV_ERR_T *provErrCode_p)
{
#if defined USE_MAP  
  try{
    USHORT_T result;
    __trace2__("MAP::MapDialog::Et96MapOpenConf: 0x%x", openResult);
    if ( openResult != ET96MAP_RESULT_OK ){
      __trace2__("MAP::MapDialog::Et96MapOpenConf: Opss, dialog is not opened!");
      SmscCommand cmd;
      bool fatal = false;
      if ( provErrCode_p != 0 ){
        __trace2__("MAP::MapDialog::Et96MapOpenConf: provErrCode 0x%x",*provErrCode_p);
        fatal = provErrCodeFatal(*provErrCode_p);
      }
      if ( refuseReason_p ) {
        const char* reason = "<Unknown-reason>";
        switch ( *refuseReason_p ) {
        case ET96MAP_NO_REASON: reason = "ET96MAP_NO_REASON"; break;
        case ET96MAP_INV_DEST_REF: 
          reason = "ET96MAP_INV_DEST_REF"; 
          fatal = true;
          break;
        case ET96MAP_INV_ORIG_REF: reason = "ET96MAP_INV_ORIG_REF"; break;
        case ET96MAP_APP_CONTEXT_NOT_SUPP: 
          reason = "ET96MAP_APP_CONTEXT_NOT_SUPP"; 
          fatal = true;
          break;
        case ET96MAP_NODE_NOT_REACHABLE: 
          reason = "ET96MAP_NODE_NOT_REACHABLE"; 
          fatal = true;
          break;
        case ET96MAP_VERS_INCOMPATIBLE: 
          reason = "ET96MAP_VERS_INCOMPATIBLE"; 
          fatal = true;
          break;
        }
        __trace2__("MAP::MapDialog::Et96MapOpenConf: refuseReason_p 0x%x",
                   *refuseReason_p,
                   reason);
      }
      if ( fatal )
        cmd = SmscCommand::makeDeliverySmResp("0",this->smscDialogId,MAKE_ERRORCODE(CMD_ERR_FATAL,MAP_NETWORKERROR));
      else 
        cmd = SmscCommand::makeDeliverySmResp("0",this->smscDialogId,MAKE_ERRORCODE(CMD_ERR_TEMP,MAP_NETWORKERROR));
      throw runtime_error("MAP::MapDialog::Et96MapOpenConf: Opss, dialog is not opened!");
    }
    if ( state == MAPST_OPENCONF ){
      result = Et96MapV2SendRInfoForSmReq(ssn, dialogid, 1, &m_msAddr, ET96MAP_DO_NOT_ATTEMPT_DELIVERY, &m_scAddr );
      if ( result != ET96MAP_E_OK ) {
        __trace2__("MAP::MapDialog::Et96MapOpenConf: Et96MapV2SendRInfoForSmReq error 0x%x",result);
        throw runtime_error("MAP::MapDialog::Et96MapOpenConf: Et96MapV2SendRInfoSmReq error");
      }
      __trace2__("MAP::MapDialog::Et96MapOpenConf: Et96MapV2SendRInfoForSmReq OK");
      result = Et96MapDelimiterReq(ssn, dialogid, 0, 0 );
      if ( result != ET96MAP_E_OK ) {
        __trace2__("MAP::MapDialog::Et96MapOpenConf: Et96MapDelimiterReq error 0x%x",result);
        throw runtime_error("MAP::MapDialog::Et96MapOpenConf: Et96MapDelimiterReq error");
      }
      __trace2__("MAP::MapDialog::Et96MapOpenConf: Et96MapDelimiterReq OK");
      state = MAPST_WAIT_RINFO;
    }else{
      __trace2__("incorrect state, must be MAPST_OPENCONF(%d) but %d",
                          MAPST_OPENCONF,state);
      throw runtime_error("incorrect state, must be MAPST_OPENCONF");
    }
  }catch(...){
    state = MAPST_BROKEN;
    throw;
  }
#else
#endif
}

bool MapDialog::ProcessCmd(const SmscCommand& cmd){
#if defined USE_MAP  
  __trace2__("MAP::MapDialog::ProcessCmd");
  try{
    __trace2__("MAP::MapDialog::ProcessCmd: 0x%x",cmd->get_commandId());
    switch ( cmd->get_commandId() ){
    case SUBMIT_RESP: {
      ET96MAP_ERROR_FORW_SM_MO_T err;
      memset(&err,0,sizeof(ET96MAP_ERROR_FORW_SM_MO_T));
      switch ( cmd->get_resp()->get_status() )
      { 
      case SmscCommand::Status::OK: break;  
      case SmscCommand::Status::SYSERROR:
        err.errorCode = 32;
        break;
      case SmscCommand::Status::INVSRC:
      case SmscCommand::Status::INVDST:
      case SmscCommand::Status::NOROUTE:
        err.errorCode = 9;
        break;
      case SmscCommand::Status::DBERROR:
      case SmscCommand::Status::INVALIDSCHEDULE:
        err.errorCode = 32;
        break;
      case SmscCommand::Status::INVALIDVALIDTIME:
      case SmscCommand::Status::INVALIDDATACODING:
      default:
        err.errorCode = 36;
        break;
      };
      USHORT_T result = Et96MapV2ForwardSmMOResp(ssn,dialogid,invokeId,
           (cmd->get_resp()->get_status()!=SmscCommand::Status::OK)?&err:0);
      if ( result != ET96MAP_E_OK ) {
        __trace2__("MAP::MapDialog::ProcessCmd: Et96MapV2ForwardSmMOResp return error 0x%hx",result);
      }else{
        __trace2__("MAP::MapDialog::ProcessCmdg: Et96MapV2ForwardSmMOResp OK");
      }
      return true;
      }
    case DELIVERY: {
      USHORT_T result;
      sms = auto_ptr<SMS>(cmd->get_sms_and_forget());
      {
        __trace2__("MAP::DELIVERY_SM %d.%d.%s -> %d.%d.%s",
                   sms->getOriginatingAddress().getTypeOfNumber(),
                   sms->getOriginatingAddress().getNumberingPlan(),
                   sms->getOriginatingAddress().value,
                   sms->getDestinationAddress().getTypeOfNumber(),
                   sms->getDestinationAddress().getNumberingPlan(),
                   sms->getDestinationAddress().value);
      }

	    mkMapAddress( &m_msAddr, sms->getDestinationAddress().value, sms->getDestinationAddress().length );
	    mkMapAddress( &m_scAddr, "79029869999", 11 );
	    mkSS7GTAddress( &scAddr, &m_scAddr, 8 );
    	mkSS7GTAddress( &mshlrAddr, &m_msAddr, 6 );
      
      appContext.acType = ET96MAP_SHORT_MSG_GATEWAY_CONTEXT;
      appContext.version = ET96MAP_APP_CNTX_T::ET96MAP_VERSION_2;
      
      result = Et96MapOpenReq(ssn, dialogid, &appContext, &mshlrAddr, &scAddr, 0, 0, 0 );
      if ( result != ET96MAP_E_OK ) {
        __trace2__("MAP::MapDialog::ProcessCmdg: Et96MapOpenReq error 0x%x",result);
        throw runtime_error("MAP::MapDialog::ProcessCmdg: Et96MapOpenReq error");
      }
      __trace2__("MAP::MapDialog::ProcessCmdg: Et96MapOpenReq OK");
    }
    return false;
    default:
      __trace2__("MAP::MapDialog::ProcessCmdg: here is no command %d",cmd->get_commandId());
      return true;
    }
  }catch(exception& e){
    __trace2__("MAP::exception %s",e.what());
    throw;
  }
#else
  return true;
#endif
}

unsigned char  lll_7bit_2_8bit[128] = {
0x40,0xa3,0x24,0xa5,0xe8,0xe9,0xf9,0xec, // 0
0xf2,0xc7,0x0a,0xd8,0xf8,0x0d,0xc5,0xe5, // 8
0x44,0x5f,0x46,0x47,0x4c,0x57,0x50,0x59, // 16
0x53,0x51,0x58,0x00,0xc6,0xe6,0xdf,0xc9, // 24
0x20,0x21,0x22,0x23,0xa4,0x25,0x26,0x27, // 32
0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f, // 40
0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37, // 48
0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f, // 56
0xa1,0x41,0x42,0x43,0x44,0x45,0x46,0x47, // 64
0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f, // 72
0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57, // 80
0x58,0x59,0x5a,0xc4,0xd6,0xd1,0xdc,0xa7, // 88
0xbf,0x61,0x62,0x63,0x64,0x65,0x66,0x67, // 96
0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f, // 104
0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77, // 112
0x78,0x79,0x7a,0xe4,0xf6,0xf1,0xfc,0xe0}; // 120

unsigned char  lll_8bit_2_7bit[256] = {
0x1b,0x54,0x54,0x54,0x54,0x54,0x54,0x54,
0x54,0x54,0x0a,0x54,0x54,0x0d,0x54,0x54,
0x54,0x54,0x54,0x54,0x54,0x54,0x54,0x54,
0x54,0x54,0x54,0x54,0x54,0x54,0x54,0x54,
0x20,0x21,0x22,0x23,0x02,0x25,0x26,0x27,
0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,
0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
0x00,0x41,0x42,0x43,0x44,0x45,0x46,0x47,
0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,
0x58,0x59,0x5a,0x54,0x54,0x54,0x54,0x11,
0x54,0x61,0x62,0x63,0x64,0x65,0x66,0x67,
0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,
0x78,0x79,0x7a,0x54,0x54,0x54,0x54,0x54,
0x54,0x54,0x54,0x54,0x54,0x54,0x54,0x54,
0x54,0x54,0x54,0x54,0x54,0x54,0x54,0x54,
0x54,0x54,0x54,0x54,0x54,0x54,0x54,0x54,
0x54,0x54,0x54,0x54,0x54,0x54,0x54,0x54,
0x54,0x40,0x54,0x01,0x24,0x03,0x54,0x5f,
0x54,0x54,0x54,0x54,0x54,0x54,0x54,0x54,
0x54,0x54,0x54,0x54,0x54,0x54,0x54,0x54,
0x54,0x54,0x54,0x54,0x54,0x54,0x54,0x60,
0x54,0x54,0x54,0x54,0x5b,0x0e,0x1c,0x09,
0x54,0x1f,0x54,0x54,0x54,0x54,0x54,0x54,
0x54,0x5d,0x54,0x54,0x54,0x54,0x5c,0x54,
0x0b,0x54,0x54,0x54,0x5e,0x54,0x54,0x1e,
0x7f,0x54,0x54,0x54,0x7b,0x0f,0x1d,0x54,
0x04,0x05,0x54,0x54,0x07,0x54,0x54,0x54,
0x54,0x7d,0x08,0x54,0x54,0x54,0x7c,0x54,
0x0c,0x06,0x54,0x54,0x7e,0x54,0x54,0x54};


extern void CloseDialog(ET96MAP_LOCAL_SSN_T lssn,ET96MAP_DIALOGUE_ID_T dialogId);
extern void CloseAndRemoveDialog(ET96MAP_LOCAL_SSN_T lssn,ET96MAP_DIALOGUE_ID_T dialogId);
extern void AbortAndRemoveDialog(ET96MAP_LOCAL_SSN_T lssn,ET96MAP_DIALOGUE_ID_T dialogId);

void MapProxy::putCommand(const SmscCommand& cmd)
{
#if defined USE_MAP  
  //MutexGuard g(mutex);
  uint32_t did = cmd->get_dialogId();
  DialogRefGuard dialog;
  __trace2__("MAPPROXY::putCommand");
  try
  {
    ET96MAP_DIALOGUE_ID_T dialogid = (ET96MAP_DIALOGUE_ID_T)did;
    if ( did > 0xffff ) {
      __trace2__("MAP::QueueProcessing: SMSC request");
      dialog.assign(MapDialogContainer::getInstance()->createSMSCDialog(did,SSN,cmd->get_sms()->getDestinationAddress().value));
      if ( dialog.isnull() ) {
        __trace2__("MAP::QueryProcessing: can't create SMSC->MS dialog, locked!");
        SmscCommand cmd = SmscCommand::makeDeliverySmResp("0",did,MAKE_ERRORCODE(CMD_ERR_RESCHEDULENOW,0));
        MapDialogContainer::getInstance()->getProxy()->putIncomingCommand(cmd);
        __trace2__("MAP::QueryProcessing: was send RESCHEDULENOW to SMSC");
        return;
      }
      did = dialog->getDialogId();
      __trace2__("MAP::QueueProcessing: dialog translation SC:%x -> MAP:%x",
                 dialog->getSMSCDialogId(),
                 dialog->getDialogId());
    }else{
      __trace2__("MAP::QueueProcessing: MAP request");
      dialog.assign(MapDialogContainer::getInstance()->getDialog(did));
    }
    __trace2__("MAP:: process to dialog with ptr 0x%p",dialog);
    if ( dialog.isnull() ){
      __trace2__("MAP::QueueProcessing: Opss, here is no dialog with id x%x",dialogid);
      CloseDialog(SSN,dialogid);
    }else{
      __trace2__("MAP::QueueProcessing: processing dialog x%x",dialogid);
      bool close_dlg = dialog->ProcessCmd(cmd);
      if ( did <= 0xffff && close_dlg ) CloseAndRemoveDialog(SSN,dialogid);
    }
  }
  catch(...)
  {
    if ( did <= 0xffff )
      CloseAndRemoveDialog(SSN,did);
    //else 
    //  AbortAndRemoveDialog(SSN,did);
    throw;
  }
  __trace2__("MAPPROXY::putCommand OK");
  //notifyOutThread();
#endif
}

