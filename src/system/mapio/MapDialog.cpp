#include "MapIoTask.h"
#include "sms/sms.h"
#include "smeman/smsccmd.h"
#include <memory>
using namespace std;
using namespace smsc::sms;
using namespace smsc::smeman;
  
USHORT_T  MapDialog::Et96MapV2ForwardSmMOInd( 
    ET96MAP_LOCAL_SSN_T lssn, 
    ET96MAP_DIALOGUE_ID_T dialogId,
    ET96MAP_INVOKE_ID_T invokeId, 
    ET96MAP_SM_RP_DA_T* dstAddr, 
    ET96MAP_SM_RP_OA_T* srcAddr,  
    ET96MAP_SM_RP_UI_T* ud )
{
  __trace2__("MapDialog::Et96MapV2ForwardSmMOInd");
  SMS sms;
  MapProxy* proxy = MapDialogContainer::getInstance()->getProxy();
  SmscCommand cmd = SmscCommand::makeSumbmitSm(sms,((uint32_t)dialogId)&0xffff);
  proxy->putIncomingCommand(cmd);
  state = MAPST_WAIT_SUBMIT_RESPONSE;
  __trace2__("MapDialog::Et96MapV2ForwardSmMOInd OK");
  return ET96MAP_E_OK;
}

bool MapDialog::ProcessCmd(SmscCommand& cmd){
  switch ( cmd->get_commandId() ){
  case SUBMIT_RESP: {
      USHORT_T result = Et96MapV2ForwardSmMOResp(ssn,dialogid,invokeId,0);
      if ( result != ET96MAP_E_OK ) {
        __trace2__("MapDialog::ProcessCmdToMsg: Et96MapV2ForwardSmMOResp return error 0x%hx",result);
      }else{
        __trace2__("MapDialog::ProcessCmdToMsg: Et96MapV2ForwardSmMOResp OK",result);
      }
      return true;
    }
  default:
    __trace2__("MapDialog::ProcessCmdToMsg: here is no command %d",cmd->get_commandId());
    return true;
  }
}


