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
extern std::string & str_cut_first_of(std::string & use_str, const char * pattern);
//Cuts off ending chars matching the given pattern
extern std::string & str_cut_last_of(std::string & use_str, const char * pattern);

//Cuts off leading/ending blanks matching the pattern
extern std::string & str_cut_blanks(std::string & use_str, const char * pattern = " \t\r\n");

} //util
} //smsc

#endif /* __SMSC_UTIL_STRINGS */

