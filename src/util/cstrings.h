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

inline std::string encode(std::string src)
{
	std::string result;
	for (std::string::const_iterator i = src.begin(); i != src.end(); i++)
	{
		char c = *i;
		if ((c >= 'a' && c <= 'z')
				|| (c >= 'A' && c <= 'Z')
				|| (c >= '0' && c <= '9'))
		{
			result += c;
		}
		else
		{
			char num[3];
			snprintf(num, sizeof(num), "%.2X", (uint8_t) c);
			result += '_';
			result += num;
		}
	}
	return result;
}

inline std::string decode(const std::string &src)
{
	std::string result;
	std::string::size_type pos = 0;
	std::string::size_type curpos = src.find('_');
	while (curpos != std::string::npos)
	{
		result.append(src, pos, curpos - pos);
		char numstr[3];
		numstr[0] = src[curpos+1];
		numstr[1] = src[curpos+2];
		numstr[2] = 0;
		result += strtol(numstr, 0, 16);
		pos = curpos + 3;
		curpos = src.find('c', pos);
	}
	result.append(src, pos, src.size() - pos);
	return result;
}

}
}
#endif //ifndef SMSC_UTIL_CSTRING
