/* ************************************************************************** *
 * Helpers: Character Separated Values List element traits template
 * specialization for integer types.
 * ************************************************************************** */
#ifndef __SMSC_UTIL_CSV_VALUE_TRAITS_INT_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_UTIL_CSV_VALUE_TRAITS_INT_HPP

#include <inttypes.h>
#include <limits.h>
#include <stdlib.h>

#include "util/csv/CSValueTraits.hpp"

namespace smsc {
namespace util {
namespace csv {

template </* _TArg = int8_t */> 
struct CSValueTraits_T<int8_t> {
  //Note: sizeof(int16_t) <= sizeof(signed int)
  static int8_t str2val(const std::string & str_val) /*throw(std::exception)*/
  {
    errno = 0;
    long ival = strtol(str_val.c_str(), NULL, 0);
    if ((ival > 127) || (ival < (-128)) || (!ival && errno))
      throw CSValueException("int8_t", str_val.c_str());
    return (int8_t)ival;
  }

  static std::string val2str(const int8_t & use_val) /*throw()*/
  {
    char buf[sizeof(int)*3 + 2];
    snprintf(buf, sizeof(buf), "%d", (int)use_val);
    return std::string(buf);
  }
};

template </* _TArg = uint8_t */> 
struct CSValueTraits_T<uint8_t> {
  static uint8_t str2val(const std::string & str_val) /*throw(std::exception)*/
  {
    errno = 0;
    unsigned long ival = strtoul(str_val.c_str(), NULL, 0);
    if ((ival > 127) || (!ival && errno))
      throw CSValueException("uint8_t", str_val.c_str());
    return (uint8_t)ival;
  }

  static std::string val2str(const uint8_t & use_val) /*throw()*/
  {
    char buf[sizeof(unsigned)*3 + 2];
    snprintf(buf, sizeof(buf), "%u", (unsigned)use_val);
    return std::string(buf);
  }
};

template </* _TArg = int16_t */> 
struct CSValueTraits_T<int16_t> {
  //Note: sizeof(int16_t) <= sizeof(signed int)
  static int16_t str2val(const std::string & str_val) /*throw(std::exception)*/
  {
    errno = 0;
    long ival = strtol(str_val.c_str(), NULL, 0);
    if ((ival > 32767) || (ival < (-32768)) || (!ival && errno))
      throw CSValueException("int16_t", str_val.c_str());
    return (int16_t)ival;
  }

  static std::string val2str(const int16_t & use_val) /*throw()*/
  {
    char buf[sizeof(int)*3 + 2];
    snprintf(buf, sizeof(buf), "%d", (int)use_val);
    return std::string(buf);
  }
};

template </* _TArg = uint16_t */> 
struct CSValueTraits_T<uint16_t> {
  //Note: sizeof(uint16_t) <= sizeof(unsigned int)
  static uint16_t str2val(const std::string & str_val) /*throw(std::exception)*/
  {
    errno = 0;
    unsigned long ival = strtoul(str_val.c_str(), NULL, 0);
    if ((ival > 32767) || (!ival && errno))
      throw CSValueException("uint16_t", str_val.c_str());
    return (uint16_t)ival;
  }

  static std::string val2str(const uint16_t & use_val) /*throw()*/
  {
    char buf[sizeof(unsigned int)*3 + 2];
    snprintf(buf, sizeof(buf), "%u", (unsigned)use_val);
    return std::string(buf);
  }
};


template </* _TArg = int32_t */> 
struct CSValueTraits_T<int32_t> {
  //Note: sizeof(int32_t) <= sizeof(signed long)
  static int32_t str2val(const std::string & str_val) /*throw(std::exception)*/
  {
    errno = 0;
    long ival = strtol(str_val.c_str(), NULL, 0);
    if ((!ival || (ival == LONG_MAX)  || (ival == LONG_MIN)) && errno)
      throw CSValueException("int32_t", str_val.c_str());
    return (int32_t)ival;
  }

  static std::string val2str(const int32_t & use_val) /*throw()*/
  {
    char buf[sizeof(long)*3 + 2];
    snprintf(buf, sizeof(buf), "%ld", (long)use_val);
    return std::string(buf);
  }
};

template </* _TArg = uint32_t */> 
struct CSValueTraits_T<uint32_t> {
  //Note: sizeof(uint32_t) <= sizeof(unsigned long)
  static uint32_t str2val(const std::string & str_val) /*throw(std::exception)*/
  {
    errno = 0;
    unsigned long ival = strtoul(str_val.c_str(), NULL, 0);
    if ((!ival || (ival == ULONG_MAX)) && errno)
      throw CSValueException("uint32_t", str_val.c_str());
    return (uint32_t)ival;
  }

  static std::string val2str(const uint32_t & use_val) /*throw()*/
  {
    char buf[sizeof(unsigned long)*3 + 2];
    snprintf(buf, sizeof(buf), "%lu", (unsigned long)use_val);
    return std::string(buf);
  }
};


template </* _TArg = int64_t */> 
struct CSValueTraits_T<int64_t> {
  //Note: sizeof(int64_t) <= sizeof(signed long long)
  static int64_t str2val(const std::string & str_val) /*throw(std::exception)*/
  {
    errno = 0;
    long long ival = strtoll(str_val.c_str(), NULL, 0);
    if ((!ival || (ival == LLONG_MAX) || (ival == LLONG_MIN)) && errno)
      throw CSValueException("int64_t", str_val.c_str());
    return (int64_t)ival;
  }

  static std::string val2str(const int64_t & use_val) /*throw()*/
  {
    char buf[sizeof(long long)*3 + 2];
    snprintf(buf, sizeof(buf), "%Ld", (long long)use_val);
    return std::string(buf);
  }
};

template </* _TArg = uint64_t */> 
struct CSValueTraits_T<uint64_t> {
  //Note: sizeof(uint64_t) <= sizeof(unsigned long long)
  static uint64_t str2val(const std::string & str_val) /*throw(std::exception)*/
  {
    errno = 0;
    unsigned long long ival = strtoull(str_val.c_str(), NULL, 0);
    if ((!ival || (ival == ULLONG_MAX)) && errno)
      throw CSValueException("uint64_t", str_val.c_str());
    return (uint64_t)ival;
  }

  static std::string val2str(const uint64_t & use_val) /*throw()*/
  {
    char buf[sizeof(unsigned long long)*3 + 2];
    snprintf(buf, sizeof(buf), "%Lu", (unsigned long long)use_val);
    return std::string(buf);
  }
};

} //csv
} //util
} //smsc

#endif /* __SMSC_UTIL_CSV_VALUE_TRAITS_INT_HPP */

