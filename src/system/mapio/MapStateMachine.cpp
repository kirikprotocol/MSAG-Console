#if defined USE_MAP

#include <stdlib.h>
#include <stdarg.h>
#include <map>
#include <string>

using namespace std;
//using namespace smsc::core::synchronization;
//using namespace smsc::core::buffers;

#include "MapDialog_spcific.cxx"
#include "MapDialogMkPDU.cxx"

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

static const bool SMS_SEGMENTATION = true;

static map<string,unsigned> x_map;

static void CloseMapDialog(unsigned dialogid){
  USHORT_T res = Et96MapCloseReq (SSN,dialogid,ET96MAP_NORMAL_RELEASE,0,0,0);
  if ( res != ET96MAP_E_OK ){
    __trace2__("MAP::%s dialog 0x%x error, code 0x%hx",__PRETTY_FUNCTION__,dialogid,res);
  }else{
    __trace2__("MAP::%s dialog 0x%x closed",__PRETTY_FUNCTION__,dialogid);
  }
}

static void DropMapDialog_(unsigned dialogid){
  MapDialogContainer::getInstance()->dropDialog(dialogid);
}

static void DropMapDialog(MapDialog* dialog){
  MapDialogContainer::getInstance()->dropDialog(dialog->dialogid_map);
}

struct MAPDIALOG_ERROR : public runtime_error
{
  unsigned code;
  MAPDIALOG_ERROR(unsigned code,const string& s) :
    runtime_error(s),code(code){}
};
struct MAPDIALOG_BAD_STATE : public MAPDIALOG_ERROR
{
  MAPDIALOG_BAD_STATE(const string& s) :
    MAPDIALOG_ERROR(MAKE_ERRORCODE(CMD_ERR_FATAL,0),s){}
};
struct MAPDIALOG_TEMP_ERROR : public MAPDIALOG_ERROR
{
  MAPDIALOG_TEMP_ERROR(const string& s,unsigned c=1) :
    MAPDIALOG_ERROR(MAKE_ERRORCODE(CMD_ERR_TEMP,c),s){}
};
struct MAPDIALOG_FATAL_ERROR : public MAPDIALOG_ERROR
{
  MAPDIALOG_FATAL_ERROR(const string& s,unsigned c=1) :
    MAPDIALOG_ERROR(MAKE_ERRORCODE(CMD_ERR_FATAL,c),s){}
};
struct MAPDIALOG_HEREISNO_ID : public MAPDIALOG_ERROR
{
  MAPDIALOG_HEREISNO_ID(const string& s,unsigned c=1) :
    MAPDIALOG_ERROR(0,s){}
};

static void SendErrToSmsc(unsigned dialogid,unsigned code)
{
  if ( dialogid == 0 ) return;
  __trace2__("Send error 0x%x to SMSC",code);
  SmscCommand cmd = SmscCommand::makeDeliverySmResp("0",dialogid,code);
  MapDialogContainer::getInstance()->getProxy()->putIncomingCommand(cmd);
  __trace2__("Send error to SMSC OK");
}

static void SendOkToSmsc(unsigned dialogid)
{
  if ( dialogid == 0 ) return;
  __trace2__("Send OK to SMSC");
  SmscCommand cmd = SmscCommand::makeDeliverySmResp("0",dialogid,0);
  MapDialogContainer::getInstance()->getProxy()->putIncomingCommand(cmd);
  __trace2__("Send OK to SMSC done");
}

static void QueryHlrVersion(MapDialog* dialog)
{
  USHORT_T result = 
    Et96MapGetACVersionReq(SSN,&dialog->mshlrAddr,ET96MAP_SHORT_MSG_GATEWAY_CONTEXT);
  if ( result != ET96MAP_E_OK ) {
    throw MAPDIALOG_FATAL_ERROR(
      FormatText("MAP::QueryHlrVersion: error 0x%x when GetAcVersion",result));
  }
  string s_((char*)dialog->mshlrAddr.ss7Addr,(char*)dialog->mshlrAddr.ss7Addr+sizeof(dialog->mshlrAddr.ss7Addr));
  x_map[s_] = dialog->dialogid_map;
}

static void QueryMcsVersion(MapDialog* dialog)
{
  USHORT_T result = 
    Et96MapGetACVersionReq(SSN,&dialog->destMscAddr,ET96MAP_SHORT_MSG_MT_RELAY);
  if ( result != ET96MAP_E_OK ) {
    throw MAPDIALOG_FATAL_ERROR(
      FormatText("MAP::QueryMcsVersion: error 0x%x when GetAcVersion",result));
  }
  string s_((char*)dialog->destMscAddr.ss7Addr,(char*)dialog->destMscAddr.ss7Addr+sizeof(dialog->destMscAddr.ss7Addr));
  x_map[s_] = dialog->dialogid_map;
}

static inline 
void SetVersion(ET96MAP_APP_CNTX_T& ac,unsigned version){
  if ( version > 3 || version == 0 ) throw runtime_error(
    FormatText("MAP::%s: Opss, version = %d, why?",__PRETTY_FUNCTION__,version));
  switch(version){
  case 3:
    ac.version = ET96MAP_APP_CNTX_T::ET96MAP_VERSION_3;
    break;
  case 2:
    ac.version = ET96MAP_APP_CNTX_T::ET96MAP_VERSION_2;
    break;
  case 1:
    ac.version = ET96MAP_APP_CNTX_T::ET96MAP_VERSION_1;
    break;
  }
}

static void SendRInfo(MapDialog* dialog)
{
  ET96MAP_APP_CNTX_T appContext;
  appContext.acType = ET96MAP_SHORT_MSG_GATEWAY_CONTEXT;
  SetVersion(appContext,dialog->version);
  USHORT_T result = Et96MapOpenReq(
    SSN, dialog->dialogid_map, 
    &appContext, &dialog->mshlrAddr, &dialog->scAddr, 0, 0, 0 );
  if ( result != ET96MAP_E_OK ) {
    throw MAPDIALOG_FATAL_ERROR(
      FormatText("MAP::MapDialog::ProcessCmdg: Et96MapOpenReq error 0x%x",result));
  }
  result = Et96MapV2SendRInfoForSmReq(SSN, dialog->dialogid_map, 1, &dialog->m_msAddr, ET96MAP_DO_NOT_ATTEMPT_DELIVERY, &dialog->m_scAddr );
  if ( result != ET96MAP_E_OK ) {
    throw MAPDIALOG_FATAL_ERROR(
      FormatText("MAP::MapDialog::Et96MapOpenConf: Et96MapV2SendRInfoForSmReq error 0x%x",result));
  }
  __trace2__("MAP::MapDialog::Et96MapOpenConf: Et96MapV2SendRInfoForSmReq OK");
  result = Et96MapDelimiterReq(SSN, dialog->dialogid_map, 0, 0 );
  if ( result != ET96MAP_E_OK ) {
    throw MAPDIALOG_FATAL_ERROR(
      FormatText("MAP::MapDialog::Et96MapOpenConf: Et96MapDelimiterReq error 0x%x",result));
  }
  __trace2__("MAP::MapDialog::ProcessCmdg: Et96MapOpenReq OK");
}

void ResponseMO(MapDialog* dialog,unsigned status)
{
  ET96MAP_ERROR_FORW_SM_MO_T err;
  memset(&err,0,sizeof(ET96MAP_ERROR_FORW_SM_MO_T));
  switch ( status )
  { 
  case SmscCommand::Status::OK: break;  
  case SmscCommand::Status::SYSERROR:
    err.errorCode = 32;
    break;
  case SmscCommand::Status::INVSRC:
  case SmscCommand::Status::INVDST:
  case SmscCommand::Status::NOROUTE:
    err.errorCode = 5;
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
  USHORT_T result = Et96MapV2ForwardSmMOResp(
    SSN,
    dialog->dialogid_map,
    dialog->invokeId,
    (status!=SmscCommand::Status::OK)?&err:0);
  if ( result != ET96MAP_E_OK ) {
    __trace2__("MAP::MapDialog::ProcessCmd: Et96MapV2ForwardSmMOResp return error 0x%hx",result);
  }else{
    __trace2__("MAP::MapDialog::ProcessCmdg: Et96MapV2ForwardSmMOResp OK");
  }
}

static void AttachSmsToDialog(MapDialog* dialog,ET96MAP_SM_RP_UI_T *ud,ET96MAP_SM_RP_OA_T *srcAddr)
{
  __trace2__("MAP::%s",__PRETTY_FUNCTION__);  
  auto_ptr<SMS> _sms ( new SMS() );
  SMS& sms = *_sms.get();
  Address src_addr;
  Address dest_addr;
  SMS_SUMBMIT_FORMAT_HEADER* ssfh = (SMS_SUMBMIT_FORMAT_HEADER*)ud->signalInfo;
  __trace2__("MAP:: MR(8) = 0x%x",ssfh->mr);
  __trace2__("MAP:: MSG_TYPE_IND(2) = 0x%x",ssfh->mg_type_ind);
  __trace2__("MAP:: MSG_VPF(2) = 0x%x",ssfh->tp_vp);
  __trace2__("MAP:: MSG_UDHI(1) = %d",ssfh->udhi);
  __trace2__("MAP:: MSG_REJECT_DUPL(1) = %d",ssfh->reject_dupl);
  __trace2__("MAP:: MSG_REPLY_PATH(1) = %d",ssfh->reply_path);
  __trace2__("MAP:: MSG_SRR(1) = %d",ssfh->srr);
  MAP_SMS_ADDRESS* msa = (MAP_SMS_ADDRESS*)(ud->signalInfo+2);
  __trace2__("MAP:: MSA tonpi.va(8):0x%x, ton(3):0x%x, npi(4):0x%x, len(8):%d",
             msa->tonpi,msa->st.ton,msa->st.npi,msa->len);
  unsigned msa_len = msa->len/2+msa->len%2+2;
  unsigned char protocol_id = *(unsigned char*)(ud->signalInfo+2+msa_len);
  __trace2__("MAP:: protocol_id = 0x%x",protocol_id);
  unsigned char user_data_coding = *(unsigned char*)(ud->signalInfo+2+msa_len+1);
  __trace2__("MAP:: user_data_encoding = 0x%x",user_data_coding);
  unsigned char user_data_len = *(unsigned char*)(ud->signalInfo+2+((ssfh->tp_vp==0||ssfh->tp_vp==2)?1:7)+msa_len+2);
  __trace2__("MAP:: user_data_len = %d",user_data_len);
  unsigned char* user_data = (unsigned char*)(ud->signalInfo+2+((ssfh->tp_vp==0||ssfh->tp_vp==2)?1:7)+msa_len+2+1);
  unsigned encoding = 0;
  if ( (user_data_coding & 0xc0) == 0 ||  // 00xxxxxx
       (user_data_coding & 0xc0) == 0x40 )  // 01xxxxxx
  {
    if ( user_data_coding&(1<<5) ){
      __trace2__("MAP:: required compression");
      throw runtime_error("MAP:: required compression");
    }
    encoding = user_data_coding&0x0c;
    if ( (user_data_coding & 0xc0) == 0x40 )
      sms.setIntProperty(Tag::SMPP_MS_VALIDITY,0x03);
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
    if ( user_data_coding & 0x4 ) encoding = MAP_8BIT_ENCODING;
    else encoding = MAP_OCTET7BIT_ENCODING;
    sms.setIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT,user_data_coding&0x3);
  }
  else{
    __trace2__("MAP:: unknown coding scheme 0x%x",user_data_coding);
    throw runtime_error("unknown coding scheme");
  }
  {
    if (  encoding == MAP_OCTET7BIT_ENCODING ){
      if ( ssfh->udhi){
        MicroString ms;
        auto_ptr<unsigned char> b(new unsigned char[255*2]);
        unsigned udh_len = ((unsigned)*user_data)&0x0ff;
        __trace2__("MAP:: ud_length 0x%x",user_data_len);
        __trace2__("MAP:: udh_len 0x%x",udh_len);
        unsigned x = (udh_len+1)*8;
        if ( x%7 != 0 ) x+=7-(x%7);
        unsigned symbols = user_data_len-x/7;
        __trace2__("MAP:: text symbols 0x%x",symbols);
        __trace2__("MAP:: text bit offset 0x%x",x-(udh_len+1)*8);
        Convert7BitToSMSC7Bit(user_data+udh_len+1,symbols,&ms,x-(udh_len+1)*8);
        memcpy(b.get(),user_data,udh_len+1);
        memcpy(b.get()+udh_len+1,ms.bytes,ms.len);
        sms.setBinProperty(Tag::SMPP_SHORT_MESSAGE,(char*)b.get(),udh_len+1+symbols);
        sms.setIntProperty(Tag::SMPP_SM_LENGTH,udh_len+1+symbols);
      }else{
        MicroString ms;
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
  if ( ssfh->srr ) sms.setIntProperty(Tag::SMSC_STATUS_REPORT_REQUEST,1);
  ConvAddrMSISDN2Smc(srcAddr,&src_addr);
  sms.setOriginatingAddress(src_addr);
  ConvAddrMap2Smc(msa,&dest_addr);
  sms.setDestinationAddress(dest_addr);
  dialog->sms = _sms;
}

static void SendSubmitCommand(MapDialog* dialog)
{
  if ( dialog->sms.get() == 0 )
    throw runtime_error("MAP::hereis no SMS for submiting");
  MapProxy* proxy = MapDialogContainer::getInstance()->getProxy();
  SmscCommand cmd = SmscCommand::makeSumbmitSm(
    *dialog->sms.get(),((uint32_t)dialog->dialogid_map)&0xffff);
  proxy->putIncomingCommand(cmd);
  //dialog->sms = 0;
}

static const unsigned DIALOGID_BEFORE_CREATING = 0x10000;

static void TryDestroyDialog(unsigned dialogid)
{
  {
    __trace2__("MAP::TryDestroyDialog: dialog 0x%x , reason error",dialogid);
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogid));
    if ( dialog.isnull() ) {
      __trace2__("MAP::TryDestroyDialog: has no dialog 0x%x",dialogid);
      return;
    }
    switch(dialog->state){
    /*case MAPST_WaitHlrVersion:
    case MAPST_SendingRInfo:
    case MAPST_RInfoFallBack:
    case MAPST_WaitRInfoConf:
    case MAPST_WaitRInfoClose:
    case MAPST_WaitMcsVersion:
    //case MAPST_SendingSms:
    case MAPST_WaitSpecOpenConf:
    case MAPST_WaitOpenConf:
    case MAPST_WaitSmsConf:
    case MAPST_WaitSpecDelimeter:
    case MAPST_WaitSmsClose:*/
    default:
      CloseMapDialog(dialog->dialogid_map);
    }
  }
  MapDialogContainer::getInstance()->dropDialog(dialogid);
}

static string RouteToString(MapDialog* dialog)
{
  static string skiped("<unknown> -> <unknown>");
#if !defined DIABLE_TRACING
  if ( dialog->sms.get() == 0 ) return skiped;
  auto_ptr<char> b(new char[1024]);
  memset(b.get(),0,1024);
  snprintf(b.get(),1023,"%d.%d.%s -> %d.%d.%s",
    dialog->sms->getOriginatingAddress().getTypeOfNumber(),
    dialog->sms->getOriginatingAddress().getNumberingPlan(),
    dialog->sms->getOriginatingAddress().value,
    dialog->sms->getDestinationAddress().getTypeOfNumber(),
    dialog->sms->getDestinationAddress().getNumberingPlan(),
    dialog->sms->getDestinationAddress().value);
  return string(b.get());
#else
  return skiped;
#endif
}

#define MAP_TRY  try {
#define MAP_CATCH(__dialogid_map,__dialogid_smsc)     \
}catch(MAPDIALOG_HEREISNO_ID& x){\
  __trace2__("#NOID#MAP::%s# MAP.did 0x%x, SMSC.did 0x%x",__PRETTY_FUNCTION__,__dialogid_map,__dialogid_smsc);\
  __trace2__("   <exception>:%s",x.what());\
}catch(MAPDIALOG_ERROR& err){\
  __trace2__("#ERR#MAP::%s# MAP.did 0x%x, SMSC.did 0x%x",__PRETTY_FUNCTION__,__dialogid_map,__dialogid_smsc);\
  __trace2__("   <exception>:%s",err.what());\
  TryDestroyDialog(__dialogid_map);\
  SendErrToSmsc(__dialogid_smsc,err.code);\
}catch(exception& e){\
  __trace2__("#except#MAP::%s# MAP.did 0x%x, SMSC.did 0x%x",__PRETTY_FUNCTION__,__dialogid_map,__dialogid_smsc);\
  __trace2__("   <exception>:%s",e.what());\
  TryDestroyDialog(__dialogid_map);\
  SendErrToSmsc(__dialogid_smsc,MAKE_ERRORCODE(CMD_ERR_FATAL,0));\
}

static bool SendSms(MapDialog* dialog){
  __trace2__("MAP::SendSms: MAP.did 0x%x",dialog->dialogid_map);
  ET96MAP_APP_CNTX_T appContext;
  appContext.acType = ET96MAP_SHORT_MSG_MT_RELAY;
  SetVersion(appContext,dialog->version);
  USHORT_T result;
  bool segmentation = false;

  result = Et96MapOpenReq(SSN,dialog->dialogid_map,&appContext,&dialog->destMscAddr,&dialog->scAddr,0,0,0);
  if ( result != ET96MAP_E_OK )
    throw MAPDIALOG_FATAL_ERROR(FormatText("MAP::SendSms: Et96MapOpenReq error 0x%x",result));
  
  dialog->smRpOa.typeOfAddress = ET96MAP_ADDRTYPE_SCADDR;
  dialog->smRpOa.addrLen = (dialog->m_scAddr.addressLength+1)/2+1;
  dialog->smRpOa.addr[0] = dialog->m_scAddr.typeOfAddress;
  memcpy( dialog->smRpOa.addr+1, dialog->m_scAddr.address, (dialog->m_scAddr.addressLength+1)/2 );

  ET96MAP_SM_RP_UI_T* ui;
  dialog->auto_ui = auto_ptr<ET96MAP_SM_RP_UI_T>(ui=new ET96MAP_SM_RP_UI_T);
  mkDeliverPDU(dialog->sms.get(),ui);
  if ( ui->signalInfoLen > 98 ) {
    __trace2__("MAP::SendSMSCToMT:Et96MapDelimiterReq");
    result = Et96MapDelimiterReq( SSN, dialog->dialogid_map, 0, 0 );
    if( result != ET96MAP_E_OK )
      throw MAPDIALOG_FATAL_ERROR(
        FormatText("MAP::SendSMSCToMT:Et96MapDelimiterReq error 0x%x",result));
    segmentation = true;
  }else{
    __trace2__("MAP::Et96MapGetACVersionConfEt96MapV2ForwardSmMTReq");
    result = Et96MapV2ForwardSmMTReq( SSN, dialog->dialogid_map, 1, &dialog->smRpDa, &dialog->smRpOa, dialog->auto_ui.get(), FALSE);
    if( result != ET96MAP_E_OK )
      throw MAPDIALOG_FATAL_ERROR(
        FormatText("MAP::SendSMSCToMT:Et96MapV2ForwardSmMTReq error 0x%x",result));
    __trace2__("MAP::SendSMSCToMT:Et96MapV2ForwardSmMTReq OK");
    result = Et96MapDelimiterReq( SSN, dialog->dialogid_map, 0, 0 );
    if( result != ET96MAP_E_OK ) 
      throw MAPDIALOG_FATAL_ERROR(
        FormatText("MAP::SendSMSCToMT:Et96MapDelimiterReq error 0x%x",result));
  }
  return segmentation;
}

static void SendSegmentedSms(MapDialog* dialog)
{
  USHORT_T result;
  __trace2__("MAP::Et96MapGetACVersionConfEt96MapV2ForwardSmMTReq");
  result = Et96MapV2ForwardSmMTReq( SSN, dialog->dialogid_map, 1, &dialog->smRpDa, &dialog->smRpOa, dialog->auto_ui.get(), FALSE);
  if( result != ET96MAP_E_OK )
    throw MAPDIALOG_FATAL_ERROR(
      FormatText("MAP::SendSMSCToMT:Et96MapV2ForwardSmMTReq error 0x%x",result));
  __trace2__("MAP::SendSMSCToMT:Et96MapV2ForwardSmMTReq OK");
  result = Et96MapDelimiterReq( SSN, dialog->dialogid_map, 0, 0 );
  if( result != ET96MAP_E_OK ) 
    throw MAPDIALOG_FATAL_ERROR(
      FormatText("MAP::SendSMSCToMT:Et96MapDelimiterReq error 0x%x",result));
}

void MAPIO_PutCommand(const SmscCommand& cmd )
{
  unsigned dialogid_smsc = cmd->get_dialogId();
  unsigned dialogid_map = 0;
  __trace2__(">>MAPPROXY::putCommand dialog:0x%x",dialogid_smsc);
  DialogRefGuard dialog;
  MAP_TRY {  
    if ( dialogid_smsc > 0xffff ) { // SMSC dialog
      if ( cmd->get_commandId() != DELIVERY )
        throw MAPDIALOG_BAD_STATE("MAP::putCommand: must be SMS DELIVERY");
      try{
        dialog.assign(MapDialogContainer::getInstance()->
                    createOrAttachSMSCDialog(
                      dialogid_smsc,
                      SSN,
                      string(cmd->get_sms()->getDestinationAddress().value)));
        if ( dialog.isnull() ) throw MAPDIALOG_TEMP_ERROR("Can't create or attach dialog");
      }catch(exception& e){
        __trace2__("#except#MAP::PutCommand# when create dialog");
        __trace2__("   <exception>:%s",e.what());
        throw MAPDIALOG_TEMP_ERROR("MAP::PutCommand: can't create dialog");
      }
      if ( dialog.isnull() ) {
        __trace2__("MAP::putCommand: can't create SMSC->MS dialog (locked), request has bean attached");
        // command has bean attached by dialog container
      }else{
        dialogid_map = dialog->dialogid_map;
        __trace2__("MAP::putCommand: Preapre SMSC command");
        dialog->sms = auto_ptr<SMS>(cmd->get_sms_and_forget());
        __trace2__("MAP::%s:DELIVERY_SM %s",__PRETTY_FUNCTION__,RouteToString(dialog.get()).c_str());
        mkMapAddress( &dialog->m_msAddr, dialog->sms->getDestinationAddress().value, dialog->sms->getDestinationAddress().length );
        mkMapAddress( &dialog->m_scAddr, "79029869999", 11 );
        mkSS7GTAddress( &dialog->scAddr, &dialog->m_scAddr, 8 );
        mkSS7GTAddress( &dialog->mshlrAddr, &dialog->m_msAddr, 6 );
        __trace2__("MAP::putCommand: Query HLR AC version");
        QueryHlrVersion(dialog.get());
        dialog->state = MAPST_WaitHlrVersion;
      }
    }else{ // MAP dialog
      dialog.assign(MapDialogContainer::getInstance()->getDialog(dialogid_smsc));
      if ( dialog.isnull() )
        throw MAPDIALOG_FATAL_ERROR(
          FormatText("MAP::putCommand: Opss, here is no dialog with id x%x",(ET96MAP_DIALOGUE_ID_T)dialogid_smsc));
      dialogid_map = dialogid_smsc;
      if ( dialog->state == MAPST_WaitSubmitCmdConf ){
        ResponseMO(dialog.get(),cmd->get_resp()->get_status());
        CloseMapDialog(dialog->dialogid_map);
        DropMapDialog(dialog.get());
      }else 
        throw MAPDIALOG_BAD_STATE(
          FormatText("MAP::%s bad state %d, did 0x%x, SMSC.did 0x%x",__PRETTY_FUNCTION__,dialog->state,dialog->dialogid_map,dialog->dialogid_smsc));
    }
  }MAP_CATCH(dialogid_map,dialogid_smsc);
}

extern "C"
USHORT_T Et96MapGetACVersionConf(ET96MAP_LOCAL_SSN_T localSsn,UCHAR_T version,ET96MAP_SS7_ADDR_T *ss7Address_sp,ET96MAP_APP_CONTEXT_T ac)
{
  unsigned dialogid_map = 0;
  unsigned dialogid_smsc = 0;
  MAP_TRY{
    __trace2__("MAP::%s ",__PRETTY_FUNCTION__);
    string s_((char*)ss7Address_sp->ss7Addr,(char*)ss7Address_sp->ss7Addr+sizeof(ss7Address_sp->ss7Addr));
    map<string,unsigned>::iterator it = x_map.find(s_);
    if ( it == x_map.end() ){
      //throw MAPDIALOG_FATAL_ERROR("MAP::Et96MapGetACVersionConf has no address for AC resolving");
      throw runtime_error("MAP::Et96MapGetACVersionConf has no address for AC resolving");
    }
    dialogid_map = it->second;
    x_map.erase(it);
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogid_map));
    if ( dialog.isnull() ) {
      throw MAPDIALOG_HEREISNO_ID(
        FormatText("MAP::dialog 0x%x is not present",dialogid_map));
    }
    else
    {
      dialogid_smsc = dialog->dialogid_smsc;
      switch( dialog->state ){
      case MAPST_WaitHlrVersion:
        dialog->version = version;
        SendRInfo(dialog.get());
        dialog->state = MAPST_SendingRInfo;
        break;
      case MAPST_WaitMcsVersion:
        dialog->version = version;
        if ( SendSms(dialog.get()) == SMS_SEGMENTATION )
          dialog->state = MAPST_WaitSpecOpenConf;
        else
          dialog->state = MAPST_WaitOpenConf;
        break;
      default:
        throw MAPDIALOG_BAD_STATE(
          FormatText("MAP::%s bad state %d, did 0x%x, SMSC.did 0x%x",__PRETTY_FUNCTION__,dialog->state,dialog->dialogid_map,dialog->dialogid_smsc));
      }
      dialog->state  = MAPST_SendingRInfo;
    }
  }MAP_CATCH(dialogid_map,dialogid_smsc);
  return ET96MAP_E_OK;
}

extern "C"
USHORT_T Et96MapOpenConf (
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_OPEN_RESULT_T openResult,
  ET96MAP_REFUSE_REASON_T *refuseReason_p,
  ET96MAP_SS7_ADDR_T *respondingAddr_sp,
  ET96MAP_APP_CNTX_T *appContext_sp,
  ET96MAP_USERDATA_T *specificInfo_sp,
  ET96MAP_PROV_ERR_T *provErrCode_p)
{
  unsigned dialogid_map = dialogueId;
  unsigned dialogid_smsc = 0;
  MAP_TRY{
    __trace2__("MAP::%s dialog 0x%x",__PRETTY_FUNCTION__,dialogueId);
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogid_map));
    if ( dialog.isnull() ) {throw 
      MAPDIALOG_HEREISNO_ID(
        FormatText("MAP::dialog 0x%x is not present",dialogid_map));}
    __trace2__("MAP::%s:DELIVERY_SM %s",__PRETTY_FUNCTION__,RouteToString(dialog));
    switch( dialog->state ){
    case MAPST_RInfoFallBack:
    case MAPST_WaitSpecOpenConf:
    case MAPST_WaitOpenConf:
      if ( openResult == ET96MAP_RESULT_NOT_OK ){
        if ( refuseReason_p && *refuseReason_p == ET96MAP_APP_CONTEXT_NOT_SUPP ){
          if ( dialog->version != 1 ){
            --dialog->version;
            dialogid_map = RemapDialog(dialog.get());
            switch ( dialog_state ) {
            case MAPST_RInfoFallBack: 
              dialog->state = MAPST_SendingRInfo;
              SendRInfo(dialog.get());
              break;
            case MAPST_WaitSpecOpenConf:
            case MAPST_WaitOpenConf:
              if ( SendSms(dialog.get()) == SMS_SEGMENTATION )
                dialog->state = MAPST_WaitSpecOpenConf;
              else
                dialog->state = MAPST_WaitOpenConf;
              break;
            }
            break;
          }else{
            //dialog->state = MAPST_BROKEN;
            throw MAPDIALOG_FATAL_ERROR(
              FormatText("MAP::%s version alredy 1 and unsupported!",__PRETTY_FUNCTION__));
          }
        }else{
          //dialog->state = MAPST_BROKEN;
          throw MAPDIALOG_BAD_STATE(
            FormatText("MAP::%s bad state %d, MAP.did 0x%x, SMSC.did 0x%x",__PRETTY_FUNCTION__,dialog->state,dialogid_map,dialogid_smsc));
        }
      }
      switch(dialog->state){
      case MAPST_RInfoFallBack:
        dialog->state = MAPST_WaitRInfoConf;
        break;
      case MAPST_WaitSpecOpenConf:
        dialog->state = MAPST_WaitSpecDelimeter;
        break;
      case MAPST_WaitOpenConf:
        dialog->state = MAPST_WaitSmsConf;
        break;
      }
      break;
    default:
      throw MAPDIALOG_BAD_STATE(
        FormatText("MAP::%s bad state %d",__PRETTY_FUNCTION__,dialog->state));
    }
  }MAP_CATCH(dialogid_map,dialogid_smsc);
  return ET96MAP_E_OK;
}

void DoRInfoErrorProcessor(
  ET96MAP_ERROR_ROUTING_INFO_FOR_SM_T *errorSendRoutingInfoForSm_sp,
  ET96MAP_PROV_ERR_T *provErrCode_p )
{
  if ( errorSendRoutingInfoForSm_sp != 0 ){
    bool fatal = false;
    switch( errorSendRoutingInfoForSm_sp->errorCode){
      case 1:case 11:case 13:case 21:fatal = true;
      case 6:fatal = false;
      default:fatal = true;
    }
    if ( fatal )throw MAPDIALOG_FATAL_ERROR("__fatal_error__");
    else throw MAPDIALOG_TEMP_ERROR("__temp_error__");
  }
  if ( provErrCode_p != 0 ){
    if ( *provErrCode_p == 0x02 || // unsupported service
         *provErrCode_p == 0x03 || // mystyped parametor
         *provErrCode_p == 0x06 || // unexcpected responnse from peer
         *provErrCode_p == 0x09 || // invalid responce recived
         (*provErrCode_p > 0x0a && *provErrCode_p <= 0x10)) // unxpected component end other
      throw MAPDIALOG_FATAL_ERROR("__fatal_error__");
    else
      throw MAPDIALOG_TEMP_ERROR("__temp_error__");
    throw MAPDIALOG_FATAL_ERROR("__fatal_error__");
  }
}

extern "C"
USHORT_T  Et96MapV2SendRInfoForSmConf ( 
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_IMSI_T *imsi_sp,
  ET96MAP_ADDRESS_T *mscNumber_sp,
  ET96MAP_LMSI_T *lmsi_sp,
  ET96MAP_ERROR_ROUTING_INFO_FOR_SM_T *errorSendRoutingInfoForSm_sp,
  ET96MAP_PROV_ERR_T *provErrCode_p )
{
  unsigned dialogid_map = dialogueId;
  unsigned dialogid_smsc = 0;
  MAP_TRY{
    __trace2__("MAP::%s dialog 0x%x",__PRETTY_FUNCTION__,dialogid_map);
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogid_map));
    if ( dialog.isnull() ) {
      unsigned _di = dialogid_map;
      dialogid_map = 0;
      throw MAPDIALOG_ERROR(
        FormatText("MAP::%s dialog 0x%x is not present",__PRETTY_FUNCTION__,_di));
    }
    dialogid_smsc = dialog->dialogid_smsc;
    __trace2__("MAP::%s:DELIVERY_SM %s",__PRETTY_FUNCTION__,RouteToString(dialog));
    DoRInfoErrorProcessor(dialog,errorSendRoutingInfoForSm_sp,provErrCode_p));
    switch( dialog->state ){
    case MAPST_WaitRInfoConf:
      {
        mkSS7GTAddress( &dialog->destMscAddr, mscNumber_sp, 8 );
        dialog->smRpDa.typeOfAddress = ET96MAP_ADDRTYPE_IMSI;
        dialog->smRpDa.addrLen = imsi_sp->imsiLen;
        memcpy( dialog->smRpDa.addr, imsi_sp->imsi, imsi_sp->imsiLen );
#if !defined DISABLE_TRACING    
        {
          auto_ptr<char> b(new char[imsi_sp->imsiLen*4+1]);
          memset(b.get(),0,imsi_sp->imsiLen*4+1);
          for ( int i=0,k=0; i < imsi_sp->imsiLen; ++i ) {
            k += sprintf(b.get()+k,"%02x ",imsi_sp->imsi[i]);
          }
          __trace2__("MAP::Et96MapV2SendRInfoForSmConf:IMSI: %s",b.get());
        }
        {
          auto_ptr<char> b(new char[mscNumber_sp->addressLength*4+1]);
          memset(b.get(),0,mscNumber_sp->addressLength*4+1);
          for ( int i=0,k=0; i < (mscNumber_sp->addressLength+1)/2; ++i ) {
            k += sprintf(b.get()+k,"%02x ",mscNumber_sp->address[i]);
          }
          __trace2__("MAP::Et96MapV2SendRInfoForSmConf:MSCNUMBER: %s",b.get());
        }
#endif
        dialog->state = MAPST_WaitSmsClose;
        break;
      }
    default:
      throw MAPDIALOG_BAD_STATE(
        FormatText("MAP::%s bad state %d, MAP.did 0x%x, SMSC.did 0x%x",__PRETTY_FUNCTION__,dialog->state,dialogid_map,dialogid_smsc));
    }
  }MAP_CATCH(dialogid_map,dialogid_smsc);
  return ET96MAP_E_OK;
}				       

extern "C"
USHORT_T Et96MapV1SendRInfoForSmConf (
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_IMSI_T *imsi_sp
  ET96MAP_LOCATION_INFO_T *locationInfo_sp,
  ET96MAP_LMSI_T *lmsi_sp,
  ET96MAP_MWD_SETS_T *mwdSet,
  ET96MAP_ERROR_ROUTING_INFO_FOR_SM_T *errorSendRoutingInfoForSm_sp,
  ET96MAP_PROV_ERR_T *provErrCode_p)
{
  return ET96MAP_E_OK;
}

extern "C"
USHORT_T Et96MapCloseInd(
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_USERDATA_T *specificInfo_sp,
  UCHAR_T priorityOrder)
{
  unsigned dialogid_map = dialogueId;
  unsigned dialogid_smsc = 0;
  MAP_TRY{
    __trace2__("MAP::%s dialog 0x%x",__PRETTY_FUNCTION__,dialogid_map);
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogid_map));
    if ( dialog.isnull() ) {
      unsigned _di = dialogid_map;
      dialogid_map = 0;
      throw MAPDIALOG_HEREISNO_ID(
        FormatText("MAP::dialog 0x%x is not present",_di));}
    dialogid_smsc = dialog->dialogid_smsc;
    __trace2__("MAP::%s:DELIVERY_SM %s",__PRETTY_FUNCTION__,RouteToString(dialog));
    switch( dialog->state ){
    case MAPST_WaitRInfoClose:
      MapDialogContainer::getInstance()->reAssignDialog(dialogueId);
      dialogueId = dialog->dialogid;
      QueryMcsVersion(dialog);
      dialog->state = MAPST_WaitMcsVersion;
      break;
    case MAPST_WaitSmsClose:
      // penging processing
      dialog->state = MAPST_CLOSED;
      SendOkToSmsc(dialog->dialogid_smsc);
      break;
    default:
      throw MAPDIALOG_BAD_STATE(
        FormatText("MAP::%s bad state %d, MAP.did 0x%x, SMSC.did 0x%x",__PRETTY_FUNCTION__,dialog->state,dialogid_map,dialogid_smsc));
    }
  }MAP_CATCH(dialogid_map,dialogid_smsc);
  return ET96MAP_E_OK;
}

extern "C"
USHORT_T Et96MapDelimiterInd(
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  UCHAR_T priorityOrder)
{
  unsigned dialogid_map = dialogueId;
  unsigned dialogid_smsc = 0;
  MAP_TRY{
    __trace2__("MAP::%s dialog 0x%x",__PRETTY_FUNCTION__,dialogid_map);
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogid_map));
    if ( dialog.isnull() ) {
      unsigned _di = dialogid_map;
      dialogid_map = 0;
      throw MAPDIALOG_ERROR(
        FormatText("MAP::%s dialog 0x%x is not present",__PRETTY_FUNCTION__,_di));
    }
    dialogid_smsc = dialog->dialogid_smsc;
    __trace2__("MAP::%s:DELIVERY_SM %s",__PRETTY_FUNCTION__,RouteToString(dialog));
    switch( dialog->state ){
    case MAPST_WaitSpecDelimeter:
      SendSegmentedSms(dialog);
      dialog->state = MAPST_WaitSmsConf;
      break;
    //case MAPST_WaitSmsClose:
    //  break;
    default:
      throw MAPDIALOG_BAD_STATE(
        FormatText("MAP::%s bad state %d, MAP.did 0x%x, SMSC.did 0x%x",__PRETTY_FUNCTION__,dialog->state,dialogid_map,dialogid_smsc));
    }
  }MAP_CATCH(dialogid_map,dialogid_smsc);
  return ET96MAP_E_OK;
}

extern "C"
USHORT_T Et96MapUAbortInd (
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_USER_REASON_T *userReason_p,
  ET96MAP_DIAGNOSTIC_INFO_T *diagInfo_p,
  ET96MAP_USERDATA_T *specificInfo_sp,
  UCHAR_T priorityOrder)
{
  unsigned dialogid_map = dialogueId;
  unsigned dialogid_smsc = 0;
  MAP_TRY{
    __trace2__("MAP::%s dialog 0x%x",__PRETTY_FUNCTION__,dialogid_map);
    throw runtime_error("UABORT");
  }MAP_CATCH(dialogid_map,dialogid_smsc);
  return ET96MAP_E_OK;
}

USHORT_T Et96MapPAbortInd(
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_PROV_REASON_T provReason,
  ET96MAP_SOURCE_T source,
  UCHAR_T priorityOrder)
{
  unsigned dialogid_map = dialogueId;
  unsigned dialogid_smsc = 0;
  MAP_TRY{
    __trace2__("MAP::%s dialog 0x%x",__PRETTY_FUNCTION__,dialogid_map);
    throw runtime_error("PABORT");
  }MAP_CATCH(dialogid_map,dialogid_smsc);
  return ET96MAP_E_OK;
}

extern "C"
USHORT_T Et96MapOpenInd (
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_APP_CNTX_T *appContext_sp,
  ET96MAP_SS7_ADDR_T *ss7DestAddr_sp,
  ET96MAP_SS7_ADDR_T *ss7OrigAddr_sp,
  ET96MAP_IMSI_T *destRef_sp,
  ET96MAP_ADDRESS_T *origRef_sp,
  ET96MAP_USERDATA_T *specificInfo_sp)
{
  try{
    __trace2__("MAP::%s dialog 0x%x",__PRETTY_FUNCTION__,dialogueId);
    DialogRefGuard dialog(MapDialogContainer::getInstance()->createDialog(dialogueId,SSN/*,0*/));
    if ( dialog.isnull() )
      throw rumtime_error("MAP:: can't create dialog");
    __trace2__("MAP:: create dialog with ptr 0x%p, dialogid 0x%x",mdci.get(),dialogId);
    dialog->state = MAPST_WaitSms;
  }
  catch(exception& e)
  {
    __trace2__("#except#MAP::%s# MAP.did 0x%x",__PRETTY_FUNCTION__,dialogueId);
    __trace2__("   <exception>:%s",e.what());
    ET96MAP_REFUSE_REASON_T reason = ET96MAP_NO_REASON;
    MapDialogContainer::getInstance()->dropDialog(dialogueId);
    Et96MapOpenResp(SSN,dialogueId,ET96MAP_RESULT_NOT_OK,&reason,0,0,0);
    Et96MapDelimiterReq(SSN,dialogueId,0,0);
  }
  return ET96MAP_E_OK;
}

extern "C"
USHORT_T Et96MapV2ForwardSmMOInd (
  ET96MAP_LOCAL_SSN_T localSsn
  ET96MAP_DIALOGUE_ID_T dialogueId,
  ET96MAP_INVOKE_ID_T invokeId,
  ET96MAP_SM_RP_DA_T *smRpDa_sp,
  ET96MAP_SM_RP_OA_T *smRpOa_sp,
  ET96MAP_SM_RP_UI_T *smRpUi_sp)
{
  bool open_confirmed = false;
  try{
    __trace2__("MAP::%s dialog 0x%x",__PRETTY_FUNCTION__,dialogid_map);
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogid_map));
    if ( dialog.isnull() ) {
      throw MAPDIALOG_ERROR(
        FormatText("MAP::%s dialog 0x%x is not present",__PRETTY_FUNCTION__,dialogueId));
    }
    __trace2__("MAP::%s: %s",__PRETTY_FUNCTION__,RouteToString(dialog));
    switch( dialog->state ){
    case MAPST_WaitSms:
      dialog->state = MAPST_WaitSmsMODelimiter2;
      break;
    case MAPST_WaitSmsMOInd:
      open_confirmed = true;
      dialog->state = MAPST_WaitSmsMODelimiter;
      break;
    default:
      throw MAPDIALOG_BAD_STATE(
        FormatText("MAP::%s bad state %d, MAP.did 0x%x, SMSC.did 0x%x",__PRETTY_FUNCTION__,dialog->state,dialogid_map,dialogid_smsc));
    }
    dialog->invokeId = invokeId;
    AttachSmsToDialog(dialog,smRpUi_sp,smRpOa_sp);
  }
  catch(exception& e)
  {
    __trace2__("#except#MAP::%s# MAP.did 0x%x",__PRETTY_FUNCTION__,dialogueId);
    __trace2__("   <exception>:%s",e.what());
    if ( !open_confirmed ){
      ET96MAP_REFUSE_REASON_T reason = ET96MAP_NO_REASON;
      MapDialogContainer::getInstance()->dropDialog(dialogueId);
      Et96MapOpenResp(SSN,dialogueId,ET96MAP_RESULT_NOT_OK,&reason,0,0,0);
      Et96MapDelimiterReq(SSN,dialogueId,0,0);
    }
    TryDestroyDialog(dialogueId);
  }
  return ET96MAP_E_OK;
}

extern "C"
USHORT_T Et96MapDelimiterInd(
  ET96MAP_LOCAL_SSN_T localSsn,
  ET96MAP_DIALOGUE_ID_T dialogueId,
  UCHAR_T priorityOrder)
{
  bool open_confirmed = false;
  USHORT_T result;
  ET96MAP_REFUSE_REASON_T reason;
  try{
    __trace2__("MAP::%s dialog 0x%x",__PRETTY_FUNCTION__,dialogid_map);
    DialogRefGuard dialog(MapDialogContainer::getInstance()->getDialog(dialogid_map));
    if ( dialog.isnull() ) {
      throw MAPDIALOG_ERROR(
        FormatText("MAP::%s dialog 0x%x is not present",__PRETTY_FUNCTION__,dialogueId));
    }
    __trace2__("MAP::%s: %s",__PRETTY_FUNCTION__,RouteToString(dialog));
    switch( dialog->state ){
    case MAPST_WaitSms:
      dialog->state = MAPST_WaitSmsMOInd:
      reason = ET96MAP_NO_REASON;
      result = Et96MapOpenResp(SSN,dialogId,ET96MAP_RESULT_OK,&reason,0,0,0);
      if ( result != ET96MAP_E_OK )
        throw runtime_error(
          FormatText("MAP::Et96MapOpenInd dialog opened error 0x%x",result));
      __trace2__("MAP::Et96MapOpenInd dialog opened");
      result = Et96MapDelimiterReq(SSN,dialogueId,0,0);
      if ( result != ET96MAP_E_OK )
        throw runtime_error(
          FormatText("MAP::Et96MapOpenInd Et96MapDelimiterReq return error 0x%x",result)); */
      open_confirmed = true;
      break;
    case MAPST_WaitSmsMODelimiter2:
      SendSubmitCommand(dialog);
      dialog->state = MAPST_WaitSubmitCmdConf;
      break;
    case MAPST_WaitSmsMODelimiter:
      open_confirmed = true;
      SendSubmitCommand(dialog);
      dialog->state = MAPST_WaitSubmitCmdConf;
      break;
    default:
      throw MAPDIALOG_BAD_STATE(
        FormatText("MAP::%s bad state %d, MAP.did 0x%x, SMSC.did 0x%x",__PRETTY_FUNCTION__,dialog->state,dialogid_map,dialogid_smsc));
    }
    dialog->invokeId = invokeId;
    AttachSmsToDialog(dialog,smRpUi_sp);
  }
  catch(exception& e)
  {
    __trace2__("#except#MAP::%s# MAP.did 0x%x",__PRETTY_FUNCTION__,dialogueId);
    __trace2__("   <exception>:%s",e.what());
    if ( !open_confirmed ){
      ET96MAP_REFUSE_REASON_T reason = ET96MAP_NO_REASON;
      MapDialogContainer::getInstance()->dropDialog(dialogueId);
      Et96MapOpenResp(SSN,dialogueId,ET96MAP_RESULT_NOT_OK,&reason,0,0,0);
      Et96MapDelimiterReq(SSN,dialogueId,0,0);
    }
    TryDestroyDialog(dialogueId);
  }
  return ET96MAP_E_OK;
}

#endif
