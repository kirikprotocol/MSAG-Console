#ifndef SMSC_UTIL_CSTRING
#define SMSC_UTIL_CSTRING

#include <string>
#include <errno.h>
#include <xercesc/dom/DOMString.hpp>

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

const char ENCODE_DECODE_ESCAPE_CHAR = '%';

inline std::string encode_(std::string src)
{
	std::string result;
	for (std::string::const_iterator i = src.begin(); i != src.end(); i++)
	{
		char c = *i;
		if (    c == ' ' ||  c == '_'
				|| (c >= 'a' && c <= 'z')
				|| (c >= 'A' && c <= 'Z')
				|| (c >= '0' && c <= '9')
				)
		{
			result += c;
		}
		else
		{
			char num[3];
			snprintf(num, sizeof(num), "%.2X", (uint8_t) c);
			result += ENCODE_DECODE_ESCAPE_CHAR;
			result += num;
		}
	}
	return result;
}

inline char * encode_(const char* src)
{
	return cStringCopy(encode_(std::string(src)).c_str());
}

inline std::string decode_(const std::string &src)
{
	std::string result;
	std::string::size_type pos = 0;
	std::string::size_type curpos = src.find(ENCODE_DECODE_ESCAPE_CHAR);
	while (curpos != std::string::npos)
	{
		result.append(src, pos, curpos - pos);
		char numstr[3];
		numstr[0] = src[curpos+1];
		numstr[1] = src[curpos+2];
		numstr[2] = 0;
		char c = strtol(numstr, 0, 16);
		if (c == 0 && errno == EINVAL)
		{// error occured, take it unchanged
			result += numstr;
		}	else
			result += c;
		pos = curpos + 3;
		curpos = src.find(ENCODE_DECODE_ESCAPE_CHAR, pos);
	}
	result.append(src, pos, src.size() - pos);
	return result;
}

/**
 * декодирует строку, закодированную функцией encode.
 *
 * @param src    Строка, которую нужно декодировать.
 * @return Новую строку, в которой результат декодирования. <B>Удаление этой строки на совести вызывающего.</B>
 */
inline char* decode_(const char * src)
{
	return cStringCopy(decode_(std::string(src)).c_str());
}

inline char* decode(DOMString src)
{
	std::auto_ptr<char> tmp(src.transcode());
	return decode_(tmp.get());
}

}				
}

#endif //ifndef SMSC_UTIL_CSTRING
