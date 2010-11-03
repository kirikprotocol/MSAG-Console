/* ************************************************************************** *
 * Helpers: Character Separated Values List element traits template.
 * ************************************************************************** */
#ifndef __SMSC_UTIL_CSV_VALUE_TRAITS_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_UTIL_CSV_VALUE_TRAITS_HPP

#include "util/Exception.hpp"

namespace smsc {
namespace util {
namespace csv {

class CSValueException : public util::Exception {
public:
  CSValueException(const char * nm_type, const char * str_val)
    : util::Exception("bad %s value: \'%s\'", nm_type, str_val)
  { }
};


template <
  class _TArg
> 
struct CSValueTraits_T {
  static _TArg        str2val(const std::string & str_val) /*throw(std::exception)*/;
  static std::string  val2str(const _TArg & use_val) /*throw()*/;
};

template </*_TArg = std::string */> 
struct CSValueTraits_T<std::string> {
  static std::string str2val(const std::string & str_val) /*throw(std::exception)*/
  {
    return std::string(str_val);
  }
  static std::string val2str(const std::string & use_val) /*throw()*/
  {
    return std::string(use_val);
  }
};

} //csv
} //util
} //smsc

#endif /* __SMSC_UTIL_CSV_VALUE_TRAITS_HPP */

