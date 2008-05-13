#ident "$Id$"
#ifndef __SMSC_MTSMS_UTIL_HPP__
#define __SMSC_MTSMS_UTIL_HPP__
#include <string>
#include "ss7cp.h"
extern "C" {
#include "i96sccpapi.h"
}
extern "C" int print2vec(const void *buffer, size_t size, void *app_key);
namespace smsc{
namespace mtsmsme{
namespace processor{
namespace util{

#define MAX_SCCP_ADDRESS_LEN 32
typedef struct
{
        UCHAR_T addrLen;
        UCHAR_T addr[MAX_SCCP_ADDRESS_LEN];
} SCCP_ADDRESS_T;
#define OCTET_STRING_DECL(name, szo) unsigned char name##_buf[szo]; OCTET_STRING_t name
#define ZERO_OCTET_STRING(name) { memset(&name, 0, sizeof(name)); name.buf = name##_buf; }
#define Address2OCTET_STRING(octs, addr)        { ZERO_OCTET_STRING(octs); \
    octs.size = packMAPAddress2OCTS(addr, (TONNPI_ADDRESS_OCTS *)(octs.buf)); }

extern void unpack_addr(char* dst, UCHAR_T* src, int len);
extern std::string dump(USHORT_T len, UCHAR_T* udp);
extern void unpack_gt(UCHAR_T len, UCHAR_T* buf);
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
extern unsigned packNumString2BCD(unsigned char* bcd, const char* str,unsigned slen, bool filler = true);
extern unsigned packNumString2BCD91(unsigned char* bcd, const char* str,unsigned slen, bool filler = true);
extern unsigned packSCCPAddress(unsigned char* dst, unsigned char npi, const char *saddr, unsigned char ssn);
extern bool modifyssn(UCHAR_T* src, UCHAR_T len, const char* pattern, UCHAR_T newssn);
}//namespace util
}//namespace processor
}//namespace mtsmsme
}//namespace smsc
#endif
