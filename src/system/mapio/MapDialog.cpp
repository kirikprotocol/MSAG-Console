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

struct MicroString{
  unsigned len;
  char bytes[256];
};

extern unsigned char lll_7bit_2_8bit[128];
extern unsigned char lll_8bit_2_7bit[256];

//list<unsigned> DialogMapContainer::dialogId_pool;

inline char GetChar(const unsigned char*& ptr,unsigned& shift){
  __trace2__("MAP: 7bit: shift %d *ptr 0x%x",shift,*ptr);
  char val = (*ptr >> shift)&0x7f;
  if ( shift > 1 )
    val |= (*(ptr+1) << (8-shift))&0x7f;
  shift += 7;
  if ( shift >= 8 ) 
  {
    shift&=0x7;
    ++ptr;
  }
  __trace2__("MAP: 7bit : %x",val);
  return lll_7bit_2_8bit[val];
}

inline void PutChar(unsigned char*& ptr,unsigned& shift,unsigned char val8bit){
  __trace2__("MAP: 7bit: shift %d *ptr 0x%x",shift,*ptr);
  unsigned char val = lll_8bit_2_7bit[val8bit];
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
  __trace2__("MAP: 7bit : %x",val);
}

void Convert7BitToText(
  const unsigned char* bit7buf, int chars,
  MicroString* text)
{
  __require__(chars<=255);
  unsigned shift = 0;
  for ( int i=0; i< chars; ++i ){
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
  const unsigned char* text, int chars, unsigned char* bit7buf)
{
  __require__(chars<=255);
  unsigned char* base = bit7buf;
  unsigned shift = 0;
  for ( int i=0; i< chars; ++i ){
     PutChar(bit7buf,shift,text[i]);
  }
#if !defined DISABLE_TRACING
  __trace2__("MAP::latin1->7bit: %s",text);
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
  return bit7buf-base+(shift?0:1);
}

#pragma pack(1)

struct SMS_SUMBMIT_FORMAT_HEADER{
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
    char c = 0;
    sa->setValue(0,&c);
  }
}

void mkSS7GTAddress( ET96MAP_SS7_ADDR_T *addr, ET96MAP_ADDRESS_T *saddr, ET96MAP_LOCAL_SSN_T ssn) {
  int i;
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
  int i;
  int sz = (len+1)/2;
  addr->addressLength = len;
  addr->typeOfAddress = 0x91; // InterNational, ISDN
  for( i = 0; i < len; i++ ) {
    int bi = i/2;
    int even = i%2;
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
    char c = 0;
    sa->setValue(0,&c);
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
  if ( user_data_coding == 0 ) // 7bit
  {
    MicroString ms;
    Convert7BitToText(user_data,user_data_len,&ms);
    sms.setBinProperty(Tag::SMPP_SHORT_MESSAGE,ms.bytes,ms.len);
    sms.setIntProperty(Tag::SMPP_DATA_CODING,0x0); // default
  }
  else if ( (user_data_coding & 0x06) == 0x04  ) // UCS2
  {
    sms.setBinProperty(Tag::SMPP_SHORT_MESSAGE,(const char*)user_data,user_data_len);
    sms.setIntProperty(Tag::SMPP_DATA_CODING,/*user_data_coding*/0x08);
  }
  unsigned esm_class = 0;
  esm_class |= (ssfh->udhi?0x40:0);
  esm_class |= (ssfh->reply_path?0x80:0);
  sms.setIntProperty(Tag::SMPP_ESM_CLASS,esm_class);
  sms.setIntProperty(Tag::SMPP_SM_LENGTH,user_data_len);
  sms.setIntProperty(Tag::SMPP_PROTOCOL_ID,protocol_id);
  sms.setMessageReference(ssfh->mr);
  ConvAddrMSISDN2Smc(srcAddr,&src_addr);
  sms.setOriginatingAddress(src_addr);
  ConvAddrMap2Smc(msa,&dest_addr);
  sms.setDestinationAddress(dest_addr);
  MapProxy* proxy = MapDialogContainer::getInstance()->getProxy();
  SmscCommand cmd = SmscCommand::makeSumbmitSm(sms,((uint32_t)dialogId)&0xffff);
  proxy->putIncomingCommand(cmd);
  state = MAPST_WAIT_SUBMIT_RESPONSE;
  __trace2__("MAP::MapDialog::Et96MapV2ForwardSmMOInd OK");
#endif
  return ET96MAP_E_OK;
}

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
  oa->len = sms->getOriginatingAddress().getLength();
  unsigned oa_length = (oa->len+1)/2; 
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
  if ( encoding == 0 ) *pdu_ptr++ = 0;
  else if ( encoding == 0x08 ) *pdu_ptr++ = 0x04;
  else{
    __trace2__("MAP::mkDeliverPDU: unsuppprted encoding 0x%x",encoding);
    throw runtime_error("unsupported encoding");
  }
  {
    time_t t;
    time(&t);
    struct tm* tms = gmtime(&t);  
    MAP_TIMESTAMP* pdu_tm = (MAP_TIMESTAMP*)pdu_ptr;
    pdu_tm->year.first  =  ((tms->tm_year)%100)/10;
    pdu_tm->year.second  = tms->tm_year%10;
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
    pdu_tm->tz = 0;
    pdu_ptr+=sizeof(MAP_TIMESTAMP);
  }
  if ( encoding == 0 ){ // 7bit
    unsigned text_len;
    const unsigned char* text = (const unsigned char*)sms.getBinProperty(Tag::SMPP_SHORT_MESSAGE,&text_len);
    *pdu_ptr++ = text_len;
    pdu_ptr += ConvertText27bit(text,text_len,pdu_ptr);
    
  }else{ // UCS2
  }
  pdu->signalInfoLen  = pdu_ptr-(unsigned char*)pdu->signalInfo;
  return pdu.release();
#else
  return 0;
#endif
}


bool  MapDialog::Et96MapCloseInd(ET96MAP_LOCAL_SSN_T,
                         ET96MAP_DIALOGUE_ID_T,
                         ET96MAP_USERDATA_T *,
                         UCHAR_T priorityOrder)
{
#if defined USE_MAP
  __trace2__("MAP::Et96MapCloseInd state: 0x%x",state);
  if ( state == MAPST_READY_FOR_SENDSMS ){
    __trace2__("MAP::Et96MapCloseInd state: REDY_FOR_SEND_SMS");
    ET96MAP_APP_CNTX_T appContext;
  	appContext.acType = ET96MAP_SHORT_MSG_MT_RELAY;
  	appContext.version = ET96MAP_APP_CNTX_T::ET96MAP_VERSION_2;
    ET96MAP_SM_RP_OA_T smRpOa;
	  smRpOa.typeOfAddress = ET96MAP_ADDRTYPE_SCADDR;
	  smRpOa.addrLen = (m_scAddr.addressLength+1)/2+1;
	  smRpOa.addr[0] = m_scAddr.typeOfAddress;
	  memcpy( smRpOa.addr+1, m_scAddr.address, (m_scAddr.addressLength+1)/2 );

    auto_ptr<ET96MAP_SM_RP_UI_T> ui(mkDeliverPDU(sms.get()));// = mkDeliverPDU( oaddress, message ); 

    USHORT_T result;
    __trace2__("MAP::Et96MapCloseInd:Et96MapV2ForwardSmMTReq");
	  result = Et96MapV2ForwardSmMTReq( SSN, dialogid, 1, &smRpDa, &smRpOa, ui.get(), FALSE);
	  if( result != ET96MAP_E_OK ) {
      __trace2__("MAP::Et96MapCloseInd:Et96MapV2ForwardSmMTReq error 0x%x",result);
	  }
    __trace2__("MAP::Et96MapCloseInd:Et96MapV2ForwardSmMTReq OK");
  	result = Et96MapDelimiterReq( SSN, dialogid, 0, 0 );
    __trace2__("MAP::send response to SMSC");
    {
      SmscCommand cmd = SmscCommand::makeDeliverySmResp(0,this->smscDialogId,0);
      MapDialogContainer::getInstance()->getProxy()->putIncomingCommand(cmd);
    }
    __trace2__("MAP::send response to SMSC OK");
    //return true;// :) optimization
    return false;
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
  state = MAPST_RINFOIND;
  
  mkSS7GTAddress( &destMscAddr,  mscNumber_sp, 8 );
	smRpDa.typeOfAddress = ET96MAP_ADDRTYPE_IMSI;
	smRpDa.addrLen = imsi_sp->imsiLen;
	memcpy( smRpDa.addr, imsi_sp->imsi, imsi_sp->imsiLen );

  if ( provErrCode_p != 0 ){
    // error hadling
    __trace2__("MAP::Et96MapV2SendRInfoForSmConf provErrCode_p 0x%x",provErrCode_p);
    throw "";
  }
  
  state = MAPST_READY_FOR_SENDSMS;

#endif
  return ET96MAP_E_OK;
}				       



bool MapDialog::ProcessCmd(const SmscCommand& cmd){
#if defined USE_MAP  
  __trace2__("MAP::MapDialog::ProcessCmd");
  try{
    __trace2__("MAP::MapDialog::ProcessCmd: 0x%x",cmd->get_commandId());
    switch ( cmd->get_commandId() ){
    case SUBMIT_RESP: {
      USHORT_T result = Et96MapV2ForwardSmMOResp(ssn,dialogid,invokeId,0);
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

	    mkMapAddress( &m_msAddr, sms->getDestinationAddress().value, sms->getDestinationAddress().length );
	    mkMapAddress( &m_scAddr, "79029869999", 11 );
	    mkSS7GTAddress( &scAddr, &m_scAddr, 8 );
    	mkSS7GTAddress( &mshlrAddr, &m_msAddr, 6 );
      
      appContext.acType = ET96MAP_SHORT_MSG_GATEWAY_CONTEXT;
      appContext.version = ET96MAP_APP_CNTX_T::ET96MAP_VERSION_2;
      
      result = Et96MapOpenReq(ssn, dialogid, &appContext, &mshlrAddr, &scAddr, 0, 0, 0 );
      if ( result != ET96MAP_E_OK ) {
        __trace2__("MAP::MapDialog::ProcessCmdg: Et96MapOpenReq error 0x%x",result);
        throw 0;
      }
      __trace2__("MAP::MapDialog::ProcessCmdg: Et96MapOpenReq OK");
     	result = Et96MapV2SendRInfoForSmReq(ssn, dialogid, 1, &m_msAddr, ET96MAP_DO_NOT_ATTEMPT_DELIVERY, &m_scAddr );
      if ( result != ET96MAP_E_OK ) {
        __trace2__("MAP::MapDialog::ProcessCmdg: Et96MapV2SendRInfoForSmReq error 0x%x",result);
        throw 0;
      }
      __trace2__("MAP::MapDialog::ProcessCmdg: Et96MapV2SendRInfoForSmReq OK");
    	result = Et96MapDelimiterReq(ssn, dialogid, 0, 0 );
      if ( result != ET96MAP_E_OK ) {
        __trace2__("MAP::MapDialog::ProcessCmdg: Et96MapDelimiterReq error 0x%x",result);
        throw 0;
      }
      __trace2__("MAP::MapDialog::ProcessCmdg: Et96MapDelimiterReq OK");
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
  MutexGuard g(mutex);
  uint32_t did = cmd->get_dialogId();
  MapDialog* dialog = 0;
  __trace2__("MAPPROXY::putCommand");
  try
  {
    ET96MAP_DIALOGUE_ID_T dialogid = (ET96MAP_DIALOGUE_ID_T)did;
    if ( did > 0xffff ) {
      __trace2__("MAP::QueueProcessing: SMSC request");
      dialog = MapDialogContainer::getInstance()->createSMSCDialog(did,SSN);
      if ( dialog == 0 ) {
        __trace2__("MAP::QueryProcessing: can't create SMSC->MS dialog");
      }
      did = dialog->getDialogId();
      __trace2__("MAP::QueueProcessing: dialog translation SC:%x -> MAP:%x",
                 dialog->getSMSCDialogId(),
                 dialog->getDialogId());
    }else{
      __trace2__("MAP::QueueProcessing: MAP request");
      dialog = MapDialogContainer::getInstance()->getDialog(did);
    }
    __trace2__("MAP:: process to dialog with ptr %x",dialog);
    if ( dialog == 0 ){
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
  //notifyOutThread();
#endif
}

