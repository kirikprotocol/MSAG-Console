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
    for ( int i=0; i<ma->len;){
      sa_val[i]=ma->val[i>>1]&0x0f+0x30;
      ++i;
      if ( i < ma->len ){
        sa_val[i] = (ma->val[i>>1]>>4)+0x30;
        ++i;
      }else break;
    }
    sa->setValue(ma->len,sa_val);
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
  __trace2__("MAP::MapDialog::Et96MapV2ForwardSmMOInd");
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
  unsigned msa_len = msa->len;
  if ( msa_len%2 ) msa_len+1;
  msa_len = msa_len/2+2;
  unsigned user_data_len = *(unsigned char*)(ud->signalInfo+2+ssfh->tp_vp?7:1+msa_len+2);
  unsigned char* user_data = (unsigned char*)(user_data_len+1);
  unsigned protocol_id = *(unsigned char*)(ud->signalInfo+2+msa_len);
  unsigned user_data_coding = *(unsigned char*)(ud->signalInfo+2+msa_len+1);
  __trace2__("MAP::DIALOG::ForwardReaq: protocol_id = 0x%x",protocol_id);
  __trace2__("MAP::DIALOG::ForwardReaq: user_data_len = %d",user_data_len);
  __trace2__("MAP::DIALOG::ForwardReaq: user_data_encoding = 0x%x",user_data_coding);
  sms.setBinProperty(Tag::SMPP_SHORT_MESSAGE,(const char*)user_data,user_data_len);
  sms.setIntProperty(Tag::SMPP_SM_LENGTH,user_data_len);
  sms.setIntProperty(Tag::SMPP_DATA_CODING,user_data_coding);
  sms.setIntProperty(Tag::SMPP_PROTOCOL_ID,protocol_id);
  sms.setMessageReference(ssfh->mr);
  ConvAddrMap2Smc((MAP_SMS_ADDRESS*)srcAddr->addr,&src_addr);
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


