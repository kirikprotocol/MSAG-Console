/* ************************************************************************** *
 * Various std::string related helpers.
 * ************************************************************************** */
#ifndef __SMSC_UTIL_STRINGS
#ident "@(#)$Id$"
#define __SMSC_UTIL_STRINGS

#include <string>

namespace smsc {
namespace util {

//Cuts off leading chars matching the given pattern
inline std::string & str_cut_first_of(std::string & use_str, const char * use_pattern)
{
  if (!use_str.empty()) {
    std::string::size_type bpos = use_str.find_first_not_of(use_pattern);
    if (bpos == use_str.npos) //string contains only matched chars
      use_str.clear();
    else if (bpos)
      use_str.erase(0, bpos);
  }
  return use_str;
}

//Cuts off ending chars matching the given pattern
inline std::string & str_cut_last_of(std::string & use_str, const char * use_pattern)
{
  if (!use_str.empty()) {
    std::string::size_type bpos = use_str.find_last_not_of(use_pattern);
    if (bpos == use_str.npos) //string contains only matched chars
      use_str.clear();
    else
      use_str.erase(bpos + 1, use_str.npos);
  }
  return use_str;
}

//Cuts off leading/ending blanks matching the pattern
inline std::string & str_cut_blanks(std::string & use_str, const char * use_pattern = " \t\r\n")
{
  str_cut_first_of(use_str, use_pattern);
  return str_cut_last_of(use_str, use_pattern);
}

} //util
} //smsc

#endif /* __SMSC_UTIL_STRINGS */

