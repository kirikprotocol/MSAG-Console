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
  unsigned char len;
  char bytes[256];
};


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
  __trace2__("7bit->latin1: %s",text->len);
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
  unsigned char* user_data = (unsigned char*)(ud->signalInfo+2+((ssfh->tp_vp==0||ssfh->tp_vp==2)?1:7)+msa_len+2+2);
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


