#include <stdio.h>
#include "util/debug.h"

extern "C"{

#define _CB(a) unsigned short a() {__warning2__("MAP::%s callback was called\n",#a); /*return ET96MAP_E_OK;*/ return 1;}

_CB(MapSubmitSMInd)
_CB(MapReportSMInd)
_CB(MapAlertInd)
_CB(MapBeginInd)
_CB(MapErrorInd)
_CB(MapUssdReqConf)
_CB(MapUssdNotifyConf)
_CB(MapProcUssdDataInd)
_CB(MapSubmitConf)
_CB(MapForwardMTPositioningConf)
_CB(MapProvideSubscriberLCSConf)
_CB(MapSendRoutingInfoForLCSConf)
_CB(MapPerformLocationLCSInd)
_CB(MapBindConf)
_CB(MapIndicationError)

//_CB(Et96MapNoticeInd)
//_CB(Et96MapIndicationError)

//_CB(Et96MapV1BeginSubscriberActivityInd)
//_CB(Et96MapV1ProcessUnstructuredSSDataInd)
_CB(Et96MapV1EnhancedImeiCheckInd)
//_CB(Et96MapV1AlertSCInd)
//_CB(Et96MapV1ReportSmDelStatConf)
//_CB(Et96MapV1SendRInfoForSmConf)
//_CB(Et96MapV1ForwardSmMT_MOConf)
//_CB(Et96MapV1ForwardSmMOInd)

//_CB(Et96MapV2ForwardSmMOConf)
//_CB(Et96MapV2ReportSmDelStatConf)
//_CB(Et96MapV2AlertSCInd)
//_CB(Et96MapV2InformSCInd)
//_CB(Et96MapV2ProcessUnstructuredSSRequestInd)
//_CB(Et96MapV2UnstructuredSSRequestConf)
//_CB(Et96MapV2UnstructuredSSNotifyConf)
_CB(Et96MapV2SendImsiConf)

_CB(Et96MapV3ProvideSubscriberLCSConf)
_CB(Et96MapV3SendRInfoForLCSConf)
_CB(Et96MapV3SubscriberLCSReportInd)
//_CB(Et96MapV3ForwardSmMTConf)
_CB(Et96MapV3SendRInfoForSmConf)
//_CB(Et96MapV3ForwardSmMOInd)
_CB(Et96MapV3ReportSmDelStatConf)
_CB(Et96MapV3InformSCInd)
//_CB(Et96MapV3AnyTimeInterrogationConf)
_CB(Et96MapV3AnyTimeInterrogationInd)
_CB(Et96MapV3ProvideSubscriberInfoConf)

_CB(Et96MapGetACVersionConf)
_CB(Et96MapV2CheckImeiConf)
_CB(Et96MapV3ForwardSmMOConf)
_CB(Et96MapV2CheckImeiInd)

}
