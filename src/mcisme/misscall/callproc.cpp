#include <unistd.h>
#include <vector>
#include "callproc.hpp"
#include "clbks.hpp"
#include "util.hpp"
#include "logger/Logger.h"
#include "sms/sms.h"

using namespace std;
namespace smsc{
namespace misscall{

using smsc::logger::Logger;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;

#define MAXENTRIES 1000
#define USER USER02_ID
#define MGMT_VER 6
void unblockCurcuits()
{
  UCHAR_T UBL[]={0x00/*HSN*/, 0x00/*SPN*/, 0xFE, 0xFF, 0xFE, 0xFF/*TS MASK*/,0x00/*NO FORCE*/};
  EINSS7_MgmtApiSendOrderReq(USER,MGMT_ID,ISUP_ID,0x0B/*UNBLOCK CURCUITS*/,sizeof(UBL),UBL,NO_WAIT);
}
int going;

enum {INIT,WORKING} state;
Logger* missedCallProcessorLogger = 0;
Mutex MissedCallProcessor::lock;
MissedCallProcessor* volatile MissedCallProcessor::processor = 0;
MissedCallProcessor* MissedCallProcessor::instance()
{
  if ( processor == 0 ) {
    MutexGuard g(lock);
    if ( processor == 0 )
    {
      processor = new MissedCallProcessor();
    }
  }
  return processor;
}
						    
MissedCallProcessor::MissedCallProcessor()
{
  missedCallProcessorLogger = Logger::getInstance("smsc.misscall");
  listener = 0;
}
void MissedCallProcessor::addMissedCallListener(MissedCallListener* _listener)
{
  MutexGuard g(lock);
  listener = _listener;
}
void MissedCallProcessor::removeMissedCallListener()
{
  MutexGuard g(lock);
  listener = 0;
}
void MissedCallProcessor::fireMissedCallEvent(MissedCallEvent& event)
{
  if (listener)
  {
    {
      MutexGuard g(lock);
      if (listener) listener->missed(event);
    }
    smsc_log_debug(missedCallProcessorLogger, "was event: %s->%s",event.from.c_str(),event.to.c_str());
  }
}
void MissedCallProcessor::stop()
{
  going = 0;
}
int MissedCallProcessor::run()
{
  USHORT_T result;
  
  result = EINSS7CpMsgInitNoSig(MAXENTRIES);
  if (result != 0) {
    smsc_log_error(missedCallProcessorLogger,"MsgInit Failed with code %d",result);
    goto failed_msginit;
  }

  result = MsgOpen(USER);
  if (result != 0) {
    smsc_log_error(missedCallProcessorLogger,"MsgOpen failed with code %d",result);
    goto failed_msgopen;
  }

  result = MsgConn(USER,ISUP_ID);
  if (result != 0) {
    smsc_log_error(missedCallProcessorLogger,"MsgConn to ISUP failed with code %d",result);
    goto failed_msgconnisup;
  }
  result = MsgConn(USER,MGMT_ID);
  if (result != 0) {
    smsc_log_error(missedCallProcessorLogger,"MsgConn to MGMT failed with code %d",result);
    goto failed_msgconnmgmt;
  }
  result = EINSS7_I97IsupBindReq(USER);
  if( result != EINSS7_I97_REQUEST_OK ) {
    smsc_log_error(missedCallProcessorLogger,"EINSS7_I97IsupBindReg() failed with code %d",result);
    goto failed_sndbindisup;
  }
  result = EINSS7_MgmtApiSendBindReq(USER,MGMT_ID,MGMT_VER,NO_WAIT);
  if( result != EINSS7_MGMTAPI_REQUEST_OK ) {
    smsc_log_error(missedCallProcessorLogger,"EINSS7_MgmtApiSendBindReq() failed with code %d",result);
    goto failed_sndbindmgmt;
  }
  smsc_log_debug(missedCallProcessorLogger,"MissedCallProcessor is started up");
  MSG_T message;

  going = 1;
  while (going) {
    message.receiver = USER;
    //result = EINSS7CpMsgRecv_r( &message, 1000 );
    result = MsgRecvEvent( &message, 0, 0, 1000 );
    if( result == MSG_TIMEOUT ) {
      continue;
    }
    if( result != MSG_OK ) {
      smsc_log_error(missedCallProcessorLogger,"MsgRecvEvent failed: %d", result );
      going = 0;
      break;
    }
    // handle received message
    {
      //printf("MsgRecv rcvd msg 0x%hx->0x%hx prim=%d sz=%d\n",message.sender,message.receiver,message.primitive,message.size);
      switch(message.sender)
      {
        case ISUP_ID:
          result = EINSS7_I97IsupHandleInd(&message);
          if( result != EINSS7_I97_REQUEST_OK ) {
            smsc_log_error(missedCallProcessorLogger,"ISUP callback function return code %d ",result);
          }
          break;
        case MGMT_ID:
          result = EINSS7_MgmtApiReceivedXMMsg(&message);
          if( result != EINSS7_MGMTAPI_RETURN_OK ) {
            smsc_log_error(missedCallProcessorLogger,"MGMT callback function return code %d ",result);
          }
          break;
      }
      EINSS7CpReleaseMsgBuffer(&message);
    }
  }
  EINSS7_MgmtApiSendUnbindReq(USER,MGMT_ID);
failed_sndbindmgmt:
  EINSS7_I97IsupUnBindReq();
failed_sndbindisup:
  MsgRel(USER,MGMT_ID);
failed_msgconnmgmt:
  MsgRel(USER,ISUP_ID);
failed_msgconnisup:
  MsgClose(USER);
failed_msgopen:
  MsgExit();
failed_msginit:
  smsc_log_debug(missedCallProcessorLogger,"MissedCallProcessor is down");
  return result;
}
}//namespace misscall
}//namespace smsc

/*********************************************************************/
/* ISUP CALLBACKS IMPLEMENTATION                                     */
/*-------------------------------------------------------------------*/
/* IndError                                                          */
/* BindConf                                                          */
/* SetupInd                                                          */
/* ReleaseConf                                                       */
/* ReleaseInd                                                        */
/*********************************************************************/
using smsc::misscall::MissedCallEvent;
using smsc::misscall::MissedCallProcessor;
using smsc::misscall::missedCallProcessorLogger;
using smsc::logger::Logger;
using namespace smsc::misscall::util;
using smsc::sms::MAX_FULL_ADDRESS_VALUE_LENGTH;

USHORT_T EINSS7_I97IsupIndError(USHORT_T errorCode,MSG_T *message)
{
  char *msg = new char[message->size*4+1];
  int k = 0;
  for ( int i=0; i<message->size; i++){
    k+=sprintf(msg+k,"%02x ",(unsigned)message->msg_p[i]);
  }
  msg[k]=0;
  const char* reason;
  switch(errorCode)
  {
    case EINSS7_I97_IND_UNKNOWN_CODE : reason = "\"Unknown primitive code\"";break;
    case EINSS7_I97_IND_LENGTH_ERROR : reason = "\"Length error\"";break;
  }
  smsc_log_error(missedCallProcessorLogger,
                 "EINSS7_I97IsupHandleInd found error $s in msg: %s",
                 reason,
                 msg);
  delete msg;
  return EINSS7_I97_REQUEST_OK;
}
void  fillEvent(EINSS7_I97_CALLINGNUMB_T *calling,
                EINSS7_I97_ORIGINALNUMB_T *called,
                MissedCallEvent& event)
{
  time(&event.time);
  if (calling && calling->noOfAddrSign <= MAX_FULL_ADDRESS_VALUE_LENGTH)
  {
    vector<char> addr(calling->noOfAddrSign +  1);
    unpack_addr(&addr[0], calling->addrSign_p, calling->noOfAddrSign);
    if (calling->natureOfAddr == EINSS7_I97_NATIONAL_NO)
    {
      addr.insert(addr.begin(),'7'); // valid only for Russia!!!
      addr.insert(addr.begin(),'+');
    }
    else if (calling->natureOfAddr == EINSS7_I97_INTERNATIONAL_NO)
    {
      addr.insert(addr.begin(),'+');
    }
    event.from = &addr[0];
  }
  if (called && called->noOfAddrSign <= MAX_FULL_ADDRESS_VALUE_LENGTH)
  {
    vector<char> addr(called->noOfAddrSign +  1);
    unpack_addr(&addr[0], called->addrSign_p, called->noOfAddrSign);
    if (called->natureOfAddr == EINSS7_I97_NATIONAL_NO)
    {
      addr.insert(addr.begin(),'7'); // valid only for Russia!!!
      addr.insert(addr.begin(),'+');
    }
    else if (called->natureOfAddr == EINSS7_I97_INTERNATIONAL_NO)
    {
      addr.insert(addr.begin(),'+');
    }
    event.to = &addr[0];
  }
}

USHORT_T EINSS7_I97IsupBindConf(USHORT_T isupUserID,UCHAR_T result)
{
  smsc_log_debug(missedCallProcessorLogger,
                 "IsupBindConf %s",
                 getIsupBindStatusDescription(result));
  return EINSS7_I97_REQUEST_OK;
}
USHORT_T releaseConnection(EINSS7_I97_ISUPHEAD_T *isupHead_sp)
{
  char r[]="9139254896";
  USHORT_T res;
  EINSS7_I97_CAUSE_T cause;
  cause.location = EINSS7_I97_ISUP_USER;
  cause.codingStd = EINSS7_I97_ITU_STAND;
  cause.rec = 0;
  cause.causeValue = 0x14; /*subscriber absent*/
  cause.lengthOfDiagnostics = 0;
  cause.diagnostics_p = 0;

  EINSS7_I97_REDIRECTIONINFO_T redirectionInfo;
  redirectionInfo.redirecting = EINSS7_I97_CALL_FORW;
  redirectionInfo.origReason = EINSS7_I97_REASON_UNKNOWN;
  redirectionInfo.counter = 2;
  redirectionInfo.lastReason = EINSS7_I97_MOB_NOT_REACHED;

  EINSS7_I97_REDIRECTIONNUMB_T redirectionNumber;
  redirectionNumber.natureOfAddr = EINSS7_I97_NATIONAL_NO;
  redirectionNumber.numberPlan = EINSS7_I97_ISDN_PLAN;
  redirectionNumber.internalNetwNumb = EINSS7_I97_INTERNAL_ROUT;
  redirectionNumber.noOfAddrSign = strlen(r);
  UCHAR_T* addr = new UCHAR_T[(strlen(r)+1)/2];
  pack_addr(addr,r,strlen(r));
  redirectionNumber.addrSign_p = addr;
  res = EINSS7_I97IsupReleaseReq(isupHead_sp, /*to do может ли он быть нулевой*/
                                  &cause,
                                  0, /* autoCongestLevel_p */
                                  &redirectionNumber,
                                  &redirectionInfo,
                                  0  /* extraOpts_sp */
                                 );
  delete addr;
  if (res != 0)
  {
    smsc_log_error(
                   missedCallProcessorLogger,
                   "IsupReleaseReq %s failed with error code %d",
                   getHeadDescription(isupHead_sp).c_str(),
                   res
                  );
  }
  return res;
}
void registerEvent(EINSS7_I97_CALLINGNUMB_T *calling, EINSS7_I97_ORIGINALNUMB_T *called)
{
  MissedCallEvent event;
  fillEvent(calling,called,event);
  MissedCallProcessor::instance()->fireMissedCallEvent(event);
}
USHORT_T EINSS7_I97IsupSetupInd(EINSS7_I97_ISUPHEAD_T *isupHead_sp,
                                USHORT_T resourceGroup,
                                EINSS7_I97_NATUREOFCONN_T *natureOfConn_sp,
                                EINSS7_I97_FORWARD_T *forward_sp,
                                UCHAR_T *callingPartyCat_p,
                                UCHAR_T *transmissionReq_p,
                                EINSS7_I97_CALLEDNUMB_T *called,
                                EINSS7_I97_CALLINGNUMB_T *calling,
                                EINSS7_I97_OPTFORWARD_T *optForward_sp,
                                EINSS7_I97_ACCESS_T *access_sp,
                                EINSS7_I97_REDIRECTINGNUMB_T *redirecting,
                                EINSS7_I97_REDIRECTIONINFO_T *redirectionInfo_sp,
                                EINSS7_I97_ORIGINALNUMB_T *original,
                                EINSS7_I97_USERINFORMATION_T *userInformation_sp,
                                EINSS7_I97_USERSERVICE_T *userService_sp,
                                EINSS7_I97_OPTPARAMS_T *extraOpts_sp)
{
  smsc_log_debug(missedCallProcessorLogger,
                 "IsupSetupInd %s RG=%d %s %s %s %s",
                 getHeadDescription(isupHead_sp).c_str(),
                 resourceGroup,
                 getCallingNumberDescription(calling).c_str(),
                 getRedirectionInfoDescription(redirectionInfo_sp).c_str(),
                 getRedirectingNumberDescription(redirecting).c_str(),
                 getOriginalNumberDescription(original).c_str()
                );
  releaseConnection(isupHead_sp);
  registerEvent(calling,original);
  return EINSS7_I97_REQUEST_OK;
}
USHORT_T EINSS7_I97IsupReleaseConf(EINSS7_I97_ISUPHEAD_T *isupHead_sp,
                                   EINSS7_I97_OPTPARAMS_T *extraOpts_sp)
{
  smsc_log_debug(missedCallProcessorLogger,
                 "ReleaseConf %s",
                 getHeadDescription(isupHead_sp).c_str());
  return EINSS7_I97_REQUEST_OK;
}
USHORT_T confirmReleaseConnection(EINSS7_I97_ISUPHEAD_T* isupHead)
{
  USHORT_T res;
  res = EINSS7_I97IsupReleaseResp(isupHead,
                                  0  /* extraOpts_sp */
                                 );
  if (res != 0)
  {
    smsc_log_error(
                   missedCallProcessorLogger,
                   "IsupReleaseResp %s failed with error code %d",
                   getHeadDescription(isupHead).c_str(),
                   res
                  );
  }
  return res;
}

USHORT_T EINSS7_I97IsupReleaseInd(EINSS7_I97_ISUPHEAD_T *isupHead_sp,
                                  UCHAR_T responseInd,
                                  UCHAR_T sourceInd,
                                  EINSS7_I97_CAUSE_T *cause_sp,
                                  UCHAR_T *autoCongestLevel_p,
                                  EINSS7_I97_REDIRECTIONNUMB_T *redirectionNumb_sp,
                                  EINSS7_I97_REDIRECTIONINFO_T *redirectionInfo_sp,
                                  EINSS7_I97_OPTPARAMS_T *extraOpts_sp)
{
  smsc_log_debug(missedCallProcessorLogger,
                 "ReleaseInd %s %s %s %s %s %s %s",
                 getHeadDescription(isupHead_sp).c_str(),
                 getResponseIndicatorDescription(responseInd).c_str(),
                 getSourceIndicatorDescription(sourceInd).c_str(),
                 getCauseDescription(cause_sp).c_str(),
                 getAutoCongLevelDescription(autoCongestLevel_p).c_str(),
                 getRedirectionNumberDescription(redirectionNumb_sp).c_str(),
                 getRedirectionInfoDescription(redirectionInfo_sp).c_str()
                );
  if (responseInd & 0x01)
  {
    confirmReleaseConnection(isupHead_sp);
  }
  return EINSS7_I97_REQUEST_OK;
}
/*********************************************************************/
/* MGMT CALLBACKS                                                    */
/*-------------------------------------------------------------------*/
/* BindConf                                                          */
/* OrderConf                                                         */
/*********************************************************************/
using smsc::misscall::util::getResultDescription;
using smsc::misscall::util::getStackStatusDescription;
using smsc::misscall::util::getModuleName;
using smsc::misscall::unblockCurcuits;
USHORT_T EINSS7_MgmtApiHandleBindConf(UCHAR_T length,
                                      UCHAR_T result,
                                      UCHAR_T mmState,
                                      UCHAR_T xmRevision)
{
  smsc_log_debug(missedCallProcessorLogger,
                 "MgmtBindConf STATUS=%s STACK=%s MGMT_VER=%hu",
                 getResultDescription(result),
                 getStackStatusDescription(mmState),
                 xmRevision);
  unblockCurcuits();
  return EINSS7_MGMTAPI_RETURN_OK;
}
USHORT_T EINSS7_MgmtApiHandleOrderConf(USHORT_T moduleId,
                                       UCHAR_T orderId,
                                       UCHAR_T orderResult,
                                       UCHAR_T resultInfo,
                                       UCHAR_T lengthOfInfo,
                                       UCHAR_T *orderInfo_p)
{
  smsc_log_debug(missedCallProcessorLogger,
                 "MgmtOrderConf %s ORDER %d %s",
                 getModuleName(moduleId),
                 orderId,
                 getResultDescription(orderResult));
  return EINSS7_MGMTAPI_RETURN_OK;
}
void _mprint_msg(MSG_T* message)
{
  char *text = new char[message->size*4+1];
  int k = 0;
  for ( int i=0; i<message->size; i++){
    k+=sprintf(text+k,"%02x ",(unsigned)message->msg_p[i]);
  }
  text[k]=0;
  smsc_log_debug(missedCallProcessorLogger,
                 "msg: %s",
                 text);
  delete text;
}
