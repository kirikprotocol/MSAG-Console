#ifndef __SMSC_MTSMS_UTIL_HPP__
#define __SMSC_MTSMS_UTIL_HPP__
#include <string>
#include "ss7cp.h"
extern "C" {
#include "i96sccpapi.h"
}
namespace smsc{
namespace mtsmsme{
namespace processor{
namespace util{
extern void unpack_addr(char* dst, UCHAR_T* src, int len);
extern std::string dump(USHORT_T len, UCHAR_T* udp);
extern std::string getAddressDescription(UCHAR_T len, UCHAR_T* buf);
extern const char* getReturnOptionDescription(UCHAR_T opt);
extern const char* getSequenceControlDescription(UCHAR_T ctrl);
extern const char* getUserState(UCHAR_T state);
extern std::string getTypeOfServiceDescription(UCHAR_T typeOfService);
extern const char* getStackStatusDescription(UCHAR_T mmState);
extern const char* getResultDescription(UCHAR_T result);
extern const char* getReturnCodeDescription(USHORT_T code);
extern const char* getModuleName(USHORT_T moduleId);
extern const char* getSccpBindStatusDescription(UCHAR_T result);
}//namespace util
}//namespace processor
}//namespace mtsmsme
}//namespace smsc
#endif
