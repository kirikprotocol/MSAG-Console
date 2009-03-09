#ident "$Id$"
#ifndef __SMSC_MTSMS_UTIL_HPP__
#define __SMSC_MTSMS_UTIL_HPP__
#include <string>
#include <inttypes.h>
extern "C" int print2vec(const void *buffer, size_t size, void *app_key);
namespace smsc{
namespace mtsmsme{
namespace processor{
namespace util{

#define OCTET_STRING_DECL(name, szo) unsigned char name##_buf[szo]; OCTET_STRING_t name
#define ZERO_OCTET_STRING(name) { memset(&name, 0, sizeof(name)); name.buf = name##_buf; }
#define Address2OCTET_STRING(octs, addr)        { ZERO_OCTET_STRING(octs); \
    octs.size = packMAPAddress2OCTS(addr, (TONNPI_ADDRESS_OCTS *)(octs.buf)); }

extern std::string dump(uint16_t len, uint8_t* udp);
extern std::string getAddressDescription(uint8_t len, uint8_t* buf);
extern const char* getUserState(uint8_t state);
extern std::string getTypeOfServiceDescription(uint8_t typeOfService);
extern const char* getStackStatusDescription(uint8_t mmState);
extern const char* getResultDescription(uint8_t result);
extern const char* getReturnCodeDescription(uint16_t code);
extern unsigned packNumString2BCD(unsigned char* bcd, const char* str,unsigned slen, bool filler = true);
extern unsigned packNumString2BCD91(unsigned char* bcd, const char* str,unsigned slen, bool filler = true);
extern unsigned unpackBCD912NumString(char* dst, const unsigned char* src, unsigned srclen);
extern unsigned packSCCPAddress(unsigned char* dst, unsigned char npi, const char *saddr, unsigned char ssn);
extern bool modifyssn(uint8_t* src, uint8_t len, const char* pattern, uint8_t newssn, bool unconditional = false);
}//namespace util
}//namespace processor
}//namespace mtsmsme
}//namespace smsc
#endif
