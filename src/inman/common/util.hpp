#ident "$Id$"
#ifndef __SMSC_INMAN_COMMON_UTIL_HPP__
#define __SMSC_INMAN_COMMON_UTIL_HPP__
#include <string>

#include "ss7cp.h"
#include "portss7.h"
#include "ss7tmc.h"
#include "i97isup_api.h"
#include "i97tcapapi.h"
#include "types.hpp"

namespace smsc {
namespace inman{
namespace common {

extern        void pack_addr(UCHAR_T* dst, const char* src, int len);
extern        void unpack_addr(char* dst, UCHAR_T* src, int len);
extern        void fillAddress(SCCP_ADDRESS_T* dst, const char *saddr, UCHAR_T ssn);
extern const char* getTcapReasonDescription(USHORT_T code);
extern const char* getTcapBindErrorMessage(UCHAR_T bindResult);
extern std::string getTypeOfServiceDescription(UCHAR_T typeOfService);
extern std::string getCalledNumberDescription(EINSS7_I97_CALLEDNUMB_T* called);
extern std::string getCallingNumberDescription(EINSS7_I97_CALLINGNUMB_T* calling);
extern std::string getRedirectingNumberDescription(EINSS7_I97_REDIRECTINGNUMB_T* redirNumb);
extern std::string getOriginalNumberDescription(EINSS7_I97_ORIGINALNUMB_T* origNumb);
extern std::string getRedirectionInfoDescription(EINSS7_I97_REDIRECTIONINFO_T* redirectionInfo);
extern std::string getHeadDescription(EINSS7_I97_ISUPHEAD_T* isupHead_sp);
extern std::string getCauseDescription(EINSS7_I97_CAUSE_T *cause_sp);
extern std::string getResponseIndicatorDescription(UCHAR_T responseInd);
extern std::string getSourceIndicatorDescription(UCHAR_T sourceInd);
extern std::string getRedirectionNumberDescription(EINSS7_I97_REDIRECTIONNUMB_T* redirectionNumb_sp);
extern std::string getAutoCongLevelDescription(UCHAR_T* autoCongestLevel);
extern std::string dump(USHORT_T len, UCHAR_T* udp, bool ascii = false);
extern const char* getResultDescription(UCHAR_T result);
extern const char* getStackStatusDescription(UCHAR_T mmState);
extern const char* getCongLevelDescription(UCHAR_T congLevel);
extern const char* getIsupBindStatusDescription(UCHAR_T result);
extern const char* getModuleName(USHORT_T moduleId);
extern const char* getReturnCodeDescription(USHORT_T code);
extern std::string getBufferDump(const char* pBuffer, int nLength);
extern std::string format(const char* szFormat, ... );

}//namespace inap
}//namespace inman
}//namespace smsc
#endif
