#ident "$Id$"
#ifndef __SMSC_MTSMS_CLBKS_HPP__
#define __SMSC_MTSMS_CLBKS_HPP__
#include "ss7cp.h"
extern "C" {
  #include "sccp_api.h"
}

/*********************************************************************/
/* SCCP                                                              */
/*********************************************************************/
extern "C" {
USHORT_T EINSS7_SccpNoticeInd(UCHAR_T, UCHAR_T, EINSS7INSTANCE_T, UCHAR_T, UCHAR_T, UCHAR_T, UCHAR_T, UCHAR_T *, UCHAR_T, UCHAR_T *, USHORT_T, UCHAR_T *);
USHORT_T EINSS7_SccpConnectInd(UCHAR_T, UCHAR_T, EINSS7INSTANCE_T, ULONG_T, UCHAR_T, UCHAR_T, UCHAR_T *, UCHAR_T, UCHAR_T *, USHORT_T, UCHAR_T *); 
USHORT_T EINSS7_SccpConnectConf(UCHAR_T, UCHAR_T, EINSS7INSTANCE_T, ULONG_T, ULONG_T, UCHAR_T, UCHAR_T, UCHAR_T *, USHORT_T, UCHAR_T *);
USHORT_T EINSS7_SccpDataInd(UCHAR_T, UCHAR_T, EINSS7INSTANCE_T, ULONG_T, ULONG_T, UCHAR_T, USHORT_T,UCHAR_T*); 
USHORT_T EINSS7_SccpDiscInd(UCHAR_T, UCHAR_T, EINSS7INSTANCE_T, ULONG_T, ULONG_T, UCHAR_T, UCHAR_T, UCHAR_T, UCHAR_T*, USHORT_T, UCHAR_T*);
}
