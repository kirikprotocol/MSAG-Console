#ifndef SMSC_UTIL_CSTRING
#define SMSC_UTIL_CSTRING

#include <string>

namespace smsc {
namespace util {

inline char * cStringCopy(const char * const src)
{
	if (src != 0)
	{
		char * dst = new char[std::strlen(src)+1];
		std::strcpy(dst, src);
		return dst;
	} else
		return 0;
}

}
}
#endif //ifndef SMSC_UTIL_CSTRING
