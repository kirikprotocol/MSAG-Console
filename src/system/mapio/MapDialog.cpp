#include "MapIoTask.h"

/*USHORT_T MapDialog::Et96MapV2ForwardSmReq(
    ET96MAP_LOCAL_SSN_T localSsn,
    ET96MAP_DIALOGUE_ID_T dialogueId,
    ET96MAP_INVOKE_ID_T invokeId,
    ET96MAP_SM_RP_DA_T *smRpDa_sp,
    ET96MAP_SM_RP_OA_T *smRpOa_sp,
    ET96MAP_SM_RP_UI_T *smRpUi_sp,
    ET96MAP_MMS_T moreMsgsToSend)
{
  __trace2__("MapDialog::Et96MapV2ForwardSmReq");
  return ET96MAPE_OK;
}*/
  
USHORT_T  MapDialog::Et96MapV2ForwardSmMOInd( 
    ET96MAP_LOCAL_SSN_T lssn, 
    ET96MAP_DIALOGUE_ID_T dialogId,
    ET96MAP_INVOKE_ID_T invokeId, 
    ET96MAP_SM_RP_DA_T* dstAddr, 
    ET96MAP_SM_RP_OA_T* srcAddr,  
    ET96MAP_SM_RP_UI_T* ud )
{
  __trace2__("MapDialog::Et96MapV2ForwardSmMOInd");
  return ET96MAP_E_OK;
}

void ForwardResponse()
{
  USHORT_T result = Et96MapV2ForwardSmMOResp(lssn,dialogId,invokeId,0);
  if ( result != ET96MAP_E_OK ){
    __trace2__("MAP::Et96MapV2ForwardSmMOInd error when send response on froward_sm");
   throw runtime_error("MAP::Et96MapV2ForwardSmMOInd error when send response on froward_sm");
  }
  CloseAndRemoveDialog(SSN,dialogId);
}


