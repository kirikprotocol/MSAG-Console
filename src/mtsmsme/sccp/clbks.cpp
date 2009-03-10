static char const ident[] = "$Id$";
#include "mtsmsme/sccp/clbks.hpp"
#include "logger/Logger.h"


using smsc::logger::Logger;
namespace smsc{namespace mtsmsme{namespace processor{
extern Logger* MtSmsProcessorLogger;
}}}

using smsc::mtsmsme::processor::MtSmsProcessorLogger;

USHORT_T EINSS7_I96SccpNoticeInd(UCHAR_T subSystemNumber,
                                 #ifdef SCCP_R9                                                                                                                     
                                 EINSS7INSTANCE_T sccpInstanceId,
                                 #endif
                                 UCHAR_T segmentationInd,
                                 UCHAR_T errorLocation,
                                 UCHAR_T reasonForReturn,
                                 UCHAR_T calledAddressLength,
                                 UCHAR_T *calledAddress_p,
                                 UCHAR_T callingAddressLength,
                                 UCHAR_T *callingAddress_p,
                                 USHORT_T userDataLength,
                                 UCHAR_T *userData_p)
{
  smsc_log_warn(MtSmsProcessorLogger, "EMPTY CALLBACK: EINSS7_I96SccpNoticeInd");
  return EINSS7_I96SCCP_REQUEST_OK;
}

USHORT_T EINSS7_I96SccpConnectInd(UCHAR_T subSystemNumber,
                                  #ifdef SCCP_R9                                                                                                                     
                                  EINSS7INSTANCE_T sccpInstanceId,
                                  #endif
                                  ULONG_T connectionId,
                                  UCHAR_T messPriImportance,
                                  UCHAR_T calledAddressLength,
                                  UCHAR_T *calledAddress_p,
                                  UCHAR_T callingAddressLength,
                                  UCHAR_T *callingAddress_p,
                                  USHORT_T userDataLength,
                                  UCHAR_T *userData_p)
{
  smsc_log_warn(MtSmsProcessorLogger, "EMPTY CALLBACK: EINSS7_I96SccpConnectInd");
  return EINSS7_I96SCCP_REQUEST_OK;
}

USHORT_T EINSS7_I96SccpConnectConf(UCHAR_T subSystemNumber,
                                   #ifdef SCCP_R9                                                                                                                     
                                   EINSS7INSTANCE_T sccpInstanceId,
                                   #endif
                                   ULONG_T connectionId,
                                   UCHAR_T messPriImportance,
                                   UCHAR_T respondAddressLength,
                                   UCHAR_T *respondAddress_p,
                                   USHORT_T userDataLength,
                                   UCHAR_T *userData_p)
{
  smsc_log_warn(MtSmsProcessorLogger, "EMPTY CALLBACK: EINSS7_I96SccpConnectConf");
  return EINSS7_I96SCCP_REQUEST_OK;
}
USHORT_T EINSS7_I96SccpDataInd(UCHAR_T subSystemNumber,
                               #ifdef SCCP_R9                                                                                                                     
                               EINSS7INSTANCE_T sccpInstanceId,
                               #endif
                               ULONG_T connectionId,
                               UCHAR_T messPriImportance,
                               USHORT_T userDataLength,
                               UCHAR_T *userData_p)
{
  smsc_log_warn(MtSmsProcessorLogger, "EMPTY CALLBACK: EINSS7_I96SccpDataInd");
  return EINSS7_I96SCCP_REQUEST_OK;
}
USHORT_T EINSS7_I96SccpDiscInd(UCHAR_T subSystemNumber,
                               #ifdef SCCP_R9                                                                                                                     
                               EINSS7INSTANCE_T sccpInstanceId,
                               #endif
                               ULONG_T connectionId,
                               UCHAR_T reason,
                               UCHAR_T originator,
                               UCHAR_T respondAddressLength,
                               UCHAR_T *respondAddress_p,
                               USHORT_T userDataLength,
                               UCHAR_T *userData_p)
{
  smsc_log_warn(MtSmsProcessorLogger, "EMPTY CALLBACK: EINSS7_I96SccpDiscInd");
  return EINSS7_I96SCCP_REQUEST_OK;
}
