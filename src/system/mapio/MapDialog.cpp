#include "MapIoTask.h"
#include "sms/sms.h"
#include "smeman/smsccmd.h"
#include <memory>
using namespace std;
using namespace smsc::sms;
using namespace smsc::smeman;
  
#define TP_VP_NONE  0
#define TP_VP_REL   1
#define TP_VP_ENCH  2
#define TP_VP_ABS   3

#pragma pack(1)

struct MicroString{
  unsigned len;
  char bytes[256];
};

extern unsigned char lll_7bit_2_8bit[128];


inline char GetChar(const char*& ptr,unsigned& shift){
  char val = (*ptr >> shift)&0x7f;
  if ( shift > 1 )
    val |= (*(ptr+1) << (8-shift))&0x7f;
  shift += 7;
  if ( shift >= 8 ) 
  {
    shift&=0x7;
    ++ptr;
  }
  return lll_7bit_2_8bit[val];
}

void Convert7BitToText(
  const char* bit7buf, int chars,
  MicroString* text)
{
  __require__(chars<=255);
  unsigned shift = 0;
  for ( int i=0; i< chars; ++i ){
    text->bytes[i] = GetChar(bit7buf,shift);
  }
  text->len = chars;
  text->bytes[chars] = 0;
  __trace2__("MAP::7bit->latin1: %s",text->bytes);
}


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
    Convert7BitToText((const char*)user_data,user_data_len,&ms);
    sms.setBinProperty(Tag::SMPP_SHORT_MESSAGE,ms.bytes,ms.len);
    sms.setIntProperty(Tag::SMPP_DATA_CODING,0x03); // Latin1
  }
  else
  {
    sms.setBinProperty(Tag::SMPP_SHORT_MESSAGE,(const char*)user_data,user_data_len);
    sms.setIntProperty(Tag::SMPP_DATA_CODING,user_data_coding);
  }
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
  return ET96MAP_E_OK;
}

bool MapDialog::ProcessCmd(SmscCommand& cmd){
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
    default:
      __trace2__("MAP::MapDialog::ProcessCmdg: here is no command %d",cmd->get_commandId());
      return true;
    }
  }catch(exception& e){
    __trace2__("MAP::exception %s",e.what());
    throw;
  }
}

unsigned char  lll_7bit_2_8bit[128] = {
0x40,
0xa3,
0x24,
0xa5,
0xe8,
0xe9,
0xf9,
0xec,
0xf2,
0xc7,
0xa,
0xd8,
0xf8,
0xd,
0xc5,
0xe5,
0x44,
0x5f,
0x46,
0x47,
0x4c,
0x57,
0x50,
0x59,
0x53,
0x51,
0x58,
0x0,
0xc6,
0xe6,
0xdf,
0xc9,
0x20,
0x21,
0x22,
0x23,
0xa4,
0x25,
0x26,
0x27,
0x28,
0x29,
0x2a,
0x2b,
0x2c,
0x2d,
0x2e,
0x2f,
0x30,
0x31,
0x32,
0x33,
0x34,
0x35,
0x36,
0x37,
0x38,
0x39,
0x3a,
0x3b,
0x3c,
0x3d,
0x3e,
0x3f,
0xa1,
0x41,
0x42,
0x43,
0x44,
0x45,
0x46,
0x47,
0x48,
0x49,
0x4a,
0x4b,
0x4c,
0x4d,
0x4e,
0x4f,
0x50,
0x51,
0x52,
0x53,
0x54,
0x55,
0x56,
0x57,
0x58,
0x59,
0x5a,
0xc4,
0xd6,
0xd1,
0xdc,
0xa7,
0xbf,
0x61,
0x62,
0x63,
0x64,
0x65,
0x66,
0x67,
0x68,
0x69,
0x6a,
0x6b,
0x6c,
0x6d,
0x6e,
0x6f,
0x70,
0x71,
0x72,
0x73,
0x74,
0x75,
0x76,
0x77,
0x78,
0x79,
0x7a,
0xe4,
0xf6,
0xf1,
0xfc,
0xe0};

