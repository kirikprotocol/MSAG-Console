#ident "$Id$"
#ifndef __SMSC_INMAN_SS7_UTIL_HPP__
#define __SMSC_INMAN_SS7_UTIL_HPP__

#include <string>

#include "ss7cp.h"
#include "i97isup_api.h"
#include "i97tcapapi.h"

namespace smsc {
namespace inman {
namespace inap {

static const UCHAR_T TCAP_INSTANCE_ID = 0;

const char* getTcapReasonDescription(USHORT_T code);
const char* getTcapBindErrorMessage(UCHAR_T bindResult);
std::string getTypeOfServiceDescription(UCHAR_T typeOfService);
std::string getRedirectionInfoDescription(EINSS7_I97_REDIRECTIONINFO_T* redirectionInfo);
std::string getHeadDescription(EINSS7_I97_ISUPHEAD_T* isupHead_sp);
std::string getCauseDescription(EINSS7_I97_CAUSE_T *cause_sp);
std::string getResponseIndicatorDescription(UCHAR_T responseInd);
std::string getSourceIndicatorDescription(UCHAR_T sourceInd);
std::string getAutoCongLevelDescription(UCHAR_T* autoCongestLevel);
const char* getResultDescription(UCHAR_T result);
const char* getStackStatusDescription(UCHAR_T mmState);
const char* getCongLevelDescription(UCHAR_T congLevel);
const char* getIsupBindStatusDescription(UCHAR_T result);
const char* getModuleName(USHORT_T moduleId);
const char* getReturnCodeDescription(USHORT_T code);

}//namespace inap
}//namespace inman
}//namespace smsc
#endif /* __SMSC_INMAN_SS7_UTIL_HPP__ */

