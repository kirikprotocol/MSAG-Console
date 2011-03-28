static char const ident[] = "$Id$";
#include <stdio.h>
#include <sys/types.h>
#include <stdexcept>

extern "C" {
#include <constr_TYPE.h>
#include <MT-ForwardSM-Arg.h>
}
#include "sms/sms.h"
#include "smeman/smsccmd.h"
#include "MTRequest.hpp"
#include "MtForward.hpp"

namespace smsc{namespace mtsmsme{namespace processor{

using namespace std;
using namespace smsc::sms;
using namespace smsc::smeman;


#define MAP_OCTET7BIT_ENCODING 0x0
#define MAP_UCS2_ENCODING 0x8
#define MAP_SMSC7BIT_ENCODING  smsc::smpp::DataCoding::SMSC7BIT

struct SMS_DELIVERY_FORMAT_HEADER{
  union{
    struct{
      unsigned char reply_path:1;
      unsigned char udhi:1;
      unsigned char srri:1;
      unsigned char reserved:2;
      unsigned char mms:1;
      unsigned char mg_type_ind:2;
    }head;
    unsigned char _val_01;
  }u;
};
struct MAP_SMS_ADDRESS{
  unsigned char len;
  union{
    struct{
      unsigned char reserved_1:1;
      unsigned char ton:3;
      unsigned char npi:4;
    }st;
    unsigned char tonpi;
  }u;
  unsigned char val[10];
};
struct MicroString{
  unsigned len;
  char bytes[256];
};
static string FormatText(const char* format,...)
{
  auto_ptr<char> b(new char[1024]);
  memset(b.get(),0,1024);
  va_list arg;
  va_start(arg,format);
  vsnprintf(b.get(),1024-1,format,arg);
  va_end(arg);
  return string(b.get());
}

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
inline void PutChar(unsigned char*& ptr,unsigned& shift,unsigned char val8bit,unsigned char* ptr_end){
  if ( ptr >= ptr_end || (shift > 1 && (ptr+1) >= ptr_end)) throw runtime_error("PutChar VeryLongText");
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
extern unsigned char lll_8bit_2_7bit[256];
unsigned ConvertText27bit(
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
extern unsigned char lll_7bit_2_8bit[128];
inline void Convert7BitToText(
  const unsigned char* bit7buf, unsigned chars,
  MicroString* text,unsigned offset=0)
{
  if (chars>255) throw runtime_error("Convert7BitToSMSC7Bit invalid chars value");
  unsigned shift = offset;
  for ( unsigned i=0; i< chars; ++i ){
    text->bytes[i] = lll_7bit_2_8bit[GetChar(bit7buf,shift)&0x7f];
  }
  text->len = chars;
  text->bytes[chars] = 0;
}
inline void ConvAddrMap2Smc(const MAP_SMS_ADDRESS* ma,Address* sa){
  sa->setTypeOfNumber(ma->u.st.ton);
  sa->setNumberingPlan(ma->u.st.npi);
  if ( ma->u.st.ton == 5 && ma->u.st.npi == 0 )
  {
    MicroString text;
    Convert7BitToText(ma->val,(ma->len/2)*8/7,&text,0);
    if ( text.len == 0 ) throw runtime_error("MAP::ConvAddrMap2Smc: zero address");
    if( text.bytes[text.len-1]==0x0d ) {
      text.len--;
      text.bytes[text.len]=0;
    }
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
    {
      char b[256] = {0,};
      memcpy(b,sa_val,ma->len);
    }
  }else{
    throw runtime_error("MAP::ConvAddrMap2Smc  MAP_SMS_ADDRESS length should be greater than 0");
  }
}
inline void ConvAddrMSISDN2Smc( MT_ForwardSM_Arg_t *msg, Address* sa){
  if (msg->sm_RP_DA.present != SM_RP_DA_PR_imsi)
    throw runtime_error("MAP::ConvAddrMSISDN2Smc  sm_RP_DA is not IMSI");

  OCTET_STRING_t *imsi = &(msg->sm_RP_DA.choice.imsi);
  sa->setTypeOfNumber(0);
  sa->setNumberingPlan(1);

  uint8_t sign;int len = (imsi->size)*2;
  int idx = 0;
  char sa_val[21] = {0,};
  do {
    if (idx & 1) sign = (imsi->buf[idx/2] >> 4) & 0x0f;
    else       sign = imsi->buf[idx/2] & 0x0f;
    switch(sign){
      case 0: case 1: case 2: case 3: case 4: case 5:
      case 6: case 7: case 8: case 9: sa_val[idx] = sign + 0x30; idx++; break;
      default: sa_val[idx] = 0; sign = 15; break;
    }
  } while( sign != 15 && idx < len) ;
  sa_val[idx] = 0;
  sa->setValue(idx,sa_val);
}

void makeSmsToRequest(MtForward* mtf,MTR* req)
{
  MT_ForwardSM_Arg_t *msg = (MT_ForwardSM_Arg_t*)mtf->potroha();
  SMS& sms = req->sms;
  SMS_DELIVERY_FORMAT_HEADER* ssfh = (SMS_DELIVERY_FORMAT_HEADER*)msg->sm_RP_UI.buf;
  MAP_SMS_ADDRESS* msa = (MAP_SMS_ADDRESS*)(msg->sm_RP_UI.buf+1);
  if( msa->len > 20 ) throw runtime_error( "Address length is invalid in received PDU" );

  unsigned msa_len = msa->len/2+msa->len%2+2;
  unsigned char protocol_id = *(unsigned char*)(msg->sm_RP_UI.buf+1+msa_len);
  unsigned char user_data_coding = *(unsigned char*)(msg->sm_RP_UI.buf+1+msa_len+1);
  //skip time stamp
  unsigned char user_data_len = *(unsigned char*)(msg->sm_RP_UI.buf+1+msa_len+1+1+7);

  unsigned max_data_len = (msg->sm_RP_UI.size-(1+msa_len+1+1+7+1) );
  unsigned char* user_data = (unsigned char*)(msg->sm_RP_UI.buf+1+msa_len+1+1+7+1);
  unsigned encoding = 0;

  /* parse datacoding */
  if ( (user_data_coding & 0xc0) == 0 ||  // 00xxxxxx
       (user_data_coding & 0xc0) == 0x40 )  // 01xxxxxx
  {
    if ( user_data_coding&(1<<5) ){
      throw runtime_error("required compression");
    }
    encoding = user_data_coding&0x0c;
    if ( (user_data_coding & 0xc0) == 0x40 ) // 01xxxxxx
      sms.setIntProperty(Tag::SMPP_MS_VALIDITY,0x03);
    if ( user_data_coding & 0x10 ) // xxx1xxxx
      sms.setIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT,(user_data_coding&0x3)+1);
  }
  else if ( (user_data_coding & 0xf0) == 0xc0 ) // 1100xxxx
  {
    encoding = MAP_OCTET7BIT_ENCODING;
    sms.setIntProperty(Tag::SMPP_MS_VALIDITY,0x3);
    sms.setIntProperty(Tag::SMPP_MS_MSG_WAIT_FACILITIES,
                       (user_data_coding&0x3)|((user_data_coding&0x8)<<4));
  }
  else if ( (user_data_coding & 0xf0) == 0xd0 ) // 1101xxxx
  {
    encoding = MAP_OCTET7BIT_ENCODING;
    sms.setIntProperty(Tag::SMPP_MS_VALIDITY,0x0);
    sms.setIntProperty(Tag::SMPP_MS_MSG_WAIT_FACILITIES,
                       (user_data_coding&0x3)|((user_data_coding&0x8)<<4));
  }
  else if ( (user_data_coding & 0xf0) == 0xe0 ) // 1110xxxx
  {
    encoding = MAP_UCS2_ENCODING;
    sms.setIntProperty(Tag::SMPP_MS_VALIDITY,0x0);
    sms.setIntProperty(Tag::SMPP_MS_MSG_WAIT_FACILITIES,
                       (user_data_coding&0x3)|((user_data_coding&0x8)<<4));
  }
  else if ( (user_data_coding & 0xf0) == 0xf0 ) // 1111xxxx
  {
    encoding = user_data_coding&0x0c;
    sms.setIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT,(user_data_coding&0x3)+1);
  }
  else{
    throw runtime_error("unknown coding scheme");
  }
  /* end of parse datacoding */

  /* validating data length */
  if ( encoding == MAP_OCTET7BIT_ENCODING )
  {
    unsigned octet_data_len = (user_data_len+1)*7/8;
    if ( octet_data_len > max_data_len )
      throw runtime_error(FormatText("bad user_data_len %d must be <= %d, PDU len=%d",octet_data_len,max_data_len,msg->sm_RP_UI.size));
    if ( ssfh->u.head.udhi){
      unsigned udh_len = ((unsigned)*user_data)&0x0ff;
      if ( udh_len >= octet_data_len )
        throw runtime_error(FormatText("octet_data_len %d, but udhi_len %d",octet_data_len,udh_len));
    }
  }
  else
  {
    if ( user_data_len > max_data_len )
      throw runtime_error(FormatText("bad user_data_len %d must be <= %d, PDU len=%d",user_data_len,max_data_len,msg->sm_RP_UI.size));
    if ( ssfh->u.head.udhi){
      unsigned udh_len = ((unsigned)*user_data)&0x0ff;
      if ( udh_len >= user_data_len )
        throw runtime_error(FormatText("user_data_len %d, but udhi_len %d",user_data_len,udh_len));
    }
  }/* end of validating data length */


  {/* write message body */
    if (  encoding == MAP_OCTET7BIT_ENCODING ){
      if ( ssfh->u.head.udhi){
        /* UDH present */
        MicroString ms;
        auto_ptr<unsigned char> b(new unsigned char[255*2]);
        unsigned udh_len = ((unsigned)*user_data)&0x0ff;
        unsigned x = (udh_len+1)*8;
        if ( x%7 != 0 ) x+=7-(x%7);
        unsigned symbols = user_data_len-x/7;
        Convert7BitToSMSC7Bit(user_data+udh_len+1,symbols,&ms,x-(udh_len+1)*8);
        memcpy(b.get(),user_data,udh_len+1);
        memcpy(b.get()+udh_len+1,ms.bytes,ms.len);
        sms.setBinProperty(Tag::SMPP_SHORT_MESSAGE,(char*)b.get(),udh_len+1+symbols);
        sms.setIntProperty(Tag::SMPP_SM_LENGTH,udh_len+1+symbols);
      }
      else
      /* no UDH present */
      {
        MicroString ms;
        Convert7BitToSMSC7Bit(user_data,user_data_len,&ms,0);
        sms.setBinProperty(Tag::SMPP_SHORT_MESSAGE,ms.bytes,ms.len);
        sms.setIntProperty(Tag::SMPP_SM_LENGTH,ms.len);
      }
      sms.setIntProperty(Tag::SMPP_DATA_CODING,(unsigned)MAP_SMSC7BIT_ENCODING);
    }
    /* encoding =! MAP_OCTET7BIT_ENCODING */
    else{
      sms.setBinProperty(Tag::SMPP_SHORT_MESSAGE,(const char*)user_data,user_data_len);
      sms.setIntProperty(Tag::SMPP_SM_LENGTH,user_data_len);
      sms.setIntProperty(Tag::SMPP_DATA_CODING,(unsigned)encoding);
    }
  }/* end of write message body */

  {/* analyzing concat info */
    unsigned INVALID = (unsigned)-1;
    if ( ssfh->u.head.udhi )
    {
      unsigned ref = INVALID;
      unsigned msgNum = INVALID;
      unsigned msgCount = INVALID;

      unsigned len = ((unsigned)*user_data)&0x0ff;
      unsigned char* udh = user_data+1;
      unsigned ptr = 0;
      for (; ptr+2 < len; ptr+=2 )
      {
        unsigned elLength = udh[ptr+1];
        if ( udh[ptr] == 0 || udh[ptr] == 8)
        {
          if ( udh[ptr] == 0 ) {
            ref = udh[ptr+2];
            msgCount = udh[ptr+3];
            msgNum   = udh[ptr+4];
            break;
          }else{
            unsigned short tmpVal;
            memcpy(&tmpVal,udh+ptr+2,2);
            ref = ntohs(tmpVal);
            msgCount = udh[ptr+4];
            msgNum   = udh[ptr+5];
            break;
          }
        }
        else ptr+=elLength;
      }
      /*
      dialog->udhiRef = ref;
      dialog->udhiMsgNum = msgNum;
      dialog->udhiMsgCount = msgCount;
      */
    }
    else
    {
      /*
      dialog->udhiRef = INVALID;
      dialog->udhiMsgNum = INVALID;
      dialog->udhiMsgCount = INVALID;
      */
    }
  }/* end of analyzing concat info */
  unsigned esm_class = 0x02;// forward mode
  esm_class |= (ssfh->u.head.udhi?0x40:0);
  esm_class |= (ssfh->u.head.reply_path?0x80:0);
  sms.setIntProperty(Tag::SMPP_ESM_CLASS,esm_class);
  sms.setIntProperty(Tag::SMPP_PROTOCOL_ID,protocol_id);
  Address to;
  ConvAddrMSISDN2Smc(msg,&to);
  sms.setDestinationAddress(to);
  Address from;
  ConvAddrMap2Smc(msa,&from);
  sms.setOriginatingAddress(from);
  //dialog->sms = _sms;
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

}}}
