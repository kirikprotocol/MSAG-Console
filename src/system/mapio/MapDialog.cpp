#include "MapIoTask.h"
#include "sms/sms.h"
#include <memory>
using namespace std;
using namespace smsc::sms;
  
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
  //sms->
  return ET96MAP_E_OK;
}


