static char const ident[] = "$Id$";
#include "mtsmsme/sccphd/clbks.hpp"
#include "logger/Logger.h"


using smsc::logger::Logger;
namespace smsc{namespace mtsmsme{namespace processor{
extern Logger* MtSmsProcessorLogger;
}}}

using smsc::mtsmsme::processor::MtSmsProcessorLogger;
USHORT_T EINSS7_SccpNoticeInd(UCHAR_T           subSystemNumber,
                              UCHAR_T           nodeId,
                              EINSS7INSTANCE_T  sccpInstanceId,
                              UCHAR_T           segmentationInd,
                              UCHAR_T           errorLocation,
                              UCHAR_T           reasonForReturn,
                              UCHAR_T           calledAddressLength,
                              UCHAR_T          *calledAddress_p,
                              UCHAR_T           callingAddressLength,
                              UCHAR_T          *callingAddress_p,
                              USHORT_T          userDataLength,
                              UCHAR_T          *userData_p)
{
  smsc_log_warn(MtSmsProcessorLogger, "EMPTY CALLBACK: EINSS7_SccpNoticeInd");
  return EINSS7_SCCP_REQUEST_OK;
}
extern "C"
USHORT_T EINSS7_SccpConnectInd(UCHAR_T           subSystemNumber,
                               UCHAR_T           nodeId,
                               EINSS7INSTANCE_T  sccpInstanceId,
                               ULONG_T	     moduleConnectionId,
                               UCHAR_T           messPriImportance,
                               UCHAR_T           calledAddressLength,
                               UCHAR_T          *calledAddress_p,
                               UCHAR_T           callingAddressLength,
                               UCHAR_T          *callingAddress_p,
                               USHORT_T          userDataLength,
                               UCHAR_T          *userData_p)
{
  smsc_log_warn(MtSmsProcessorLogger, "EMPTY CALLBACK: EINSS7_SccpConnectInd");
  return EINSS7_SCCP_REQUEST_OK;
}
extern "C"
USHORT_T EINSS7_SccpConnectConf(UCHAR_T           subSystemNumber,
                                UCHAR_T           nodeId,
                                EINSS7INSTANCE_T  sccpInstanceId,
                                ULONG_T	      userConnectionId,
                                ULONG_T	      moduleConnectionId,
                                UCHAR_T           messPriImportance,
                                UCHAR_T           calledAdrLength,
                                UCHAR_T          *calledAdr_p,  
                                USHORT_T          userDataLength,
                                UCHAR_T          *userData_p)
{
  smsc_log_warn(MtSmsProcessorLogger, "EMPTY CALLBACK: EINSS7_SccpConnectConf");
  return EINSS7_SCCP_REQUEST_OK;
}
extern "C"
USHORT_T EINSS7_SccpDataInd(UCHAR_T           subSystemNumber,
                            UCHAR_T           nodeId,
                            EINSS7INSTANCE_T  sccpInstanceId,
                            ULONG_T	          userConnectionId,
                            ULONG_T	          moduleConnectionId,
                            UCHAR_T           messPriImportance,
                            USHORT_T          userDataLength,
                            UCHAR_T          *userData_p)
{
  smsc_log_warn(MtSmsProcessorLogger, "EMPTY CALLBACK: EINSS7_SccpDataInd");
  return EINSS7_SCCP_REQUEST_OK;
}
extern "C"
USHORT_T EINSS7_SccpDiscInd(UCHAR_T           subSystemNumber,
                            UCHAR_T           nodeId,
                            EINSS7INSTANCE_T  sccpInstanceId,
                            ULONG_T	          userConnectionId,
                            ULONG_T           moduleConnectionId,
                            UCHAR_T           reason,
                            UCHAR_T           originator,
                            UCHAR_T           calledAdrLength,
                            UCHAR_T          *calledAdr_p,
                            USHORT_T          userDataLength,
                            UCHAR_T          *userData_p)
{
  smsc_log_warn(MtSmsProcessorLogger, "EMPTY CALLBACK: EINSS7_SccpDiscInd");
  return EINSS7_SCCP_REQUEST_OK;
}
