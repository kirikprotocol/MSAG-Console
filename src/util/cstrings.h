#ifndef SMSC_UTIL_CSTRING
#define SMSC_UTIL_CSTRING

#include <string>
#include <string.h>
#include <errno.h>

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

const char ENCODE_DECODE_ESCAPE_CHAR = '/';

inline const size_t calculateStringSizeAfterEncoding(const char * const src)
{
  size_t count = 0;
  for (const char *p = src; *p != 0; p++)
  {
    switch (*p)
    {
    case '<': // &lt;
    case '>': // &gt;
      count += 4;
      break;
    case '&': // &amp;
      count += 5;
      break;
    case '"': // &quot;
      count += 6;
      break;
    default:
      count++;
    }
  }
  return count;
}

inline char * encode(const char * const src)
{
  char * result = new char[calculateStringSizeAfterEncoding(src) +1];
  char *d = result;
  for (const char *s = src; *s != 0; s++)
  {
    switch (*s)
    {
    case '<': // &lt;
      memcpy(d, "&lt;", 4);
      d += 4;
      break;
    case '>': // &gt;
      memcpy(d, "&gt;", 4);
      d += 4;
      break;
    case '&': // &amp;
      memcpy(d, "&amp;", 5);
      d += 5;
      break;
    case '"': // &quot;
      memcpy(d, "&quot;", 6);
      d += 6;
      break;
    default:
      *d++ = *s;
    }
  }
  *d=0;
  return result;
}

inline char* decodeDot(char * str)
{
  for (char *p = str; *p != 0; p++)
  {
    if (*p == ENCODE_DECODE_ESCAPE_CHAR)
      *p = '.';
  }
  return str;
}

inline char* encodeDot(char * str)
{
  for (char *p = str; *p != 0; p++)
  {
    if (*p == '.')
      *p = ENCODE_DECODE_ESCAPE_CHAR;
  }
  return str;
}

inline bool startsWith(const char * const str, const char * const prefix)
{
  return strncmp(str, prefix, strlen(prefix)) == 0;
}

/**
* trims spaces, tabs and newlines from begin and end of string.
* WARNING: modifies source string!
*/
inline char * trim(char * str)
{
  const size_t l = strlen(str);
  for (int i = l-1; i >= 0 && (str[i] == ' ' || str[i] == '\t' || str[i] == '\n'); i--) str[i]=0;
  char * s = str;
  while (*s == ' ' || *s == '\t' || *s == '\n') s++;
  return s;
}

inline bool cStringEquals(const char * const s1, const char * const s2)
{
  return    (s1 == 0 && s2 == 0)
         || (s1 != 0 && s2 != 0 && strcmp(s1, s2) == 0);
}

inline bool cStringLess(const char * const s1, const char * const s2)
{
  return    (s1 == 0 && s2 != 0)
         || (s1 != 0 && s2 != 0 && strcmp(s1, s2) < 0);
}

}
}

#endif //ifndef SMSC_UTIL_CSTRING
