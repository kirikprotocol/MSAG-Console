/* ************************************************************************** *
 * Helpers: Character Separated Values List element traits template
 * specialization for integer types.
 * ************************************************************************** */
#ifndef __SMSC_UTIL_CSV_VALUE_TRAITS_INT_HPP
#ident "@(#)$Id$"
#define __SMSC_UTIL_CSV_VALUE_TRAITS_INT_HPP

#include <inttypes.h>
//#include <stdio.h>
#include <stdlib.h>

#include "util/csv/CSValueTraits.hpp"

namespace smsc {
namespace util {
namespace csv {

template </* _TArg = uint8_t */> 
struct CSValueTraits_T<uint8_t> {
  static uint8_t str2val(const std::string & str_val) /*throw(std::exception)*/
  {
    int ival = atoi(str_val.c_str());
    if (!ival) { //check for '0'
      unsigned i = 0;
      while ((str_val.c_str()[i] == '0') && (i < str_val.size()))
        ++i;
      if (i < str_val.size())
        ival = 0x100;
    }
    if (ival > 0xFF)
      throw CSValueException("uint8_t", str_val.c_str());
    return (uint8_t)ival;
  }

  static std::string val2str(const uint8_t & use_val) /*throw()*/
  {
    char buf[sizeof(uint8_t)*3+2];
    snprintf(buf, sizeof(buf), "%u", (unsigned)use_val);
    return std::string(buf);
  }
};

template </* _TArg = uint16_t */> 
struct CSValueTraits_T<uint16_t> {
  static uint16_t str2val(const std::string & str_val) /*throw(std::exception)*/
  {
    int ival = atoi(str_val.c_str());
    if (!ival) { //check for '0'
      unsigned i = 0;
      while ((str_val.c_str()[i] == '0') && (i < str_val.size()))
        ++i;
      if (i < str_val.size())
        ival = 0x10000;
    }
    if (ival > 0xFFFF)
      throw CSValueException("uint16_t", str_val.c_str());
    return (uint16_t)ival;
  }

  static std::string val2str(const uint16_t & use_val) /*throw()*/
  {
    char buf[sizeof(uint16_t)*3+2];
    snprintf(buf, sizeof(buf), "%u", (unsigned)use_val);
    return std::string(buf);
  }
};

template </* _TArg = uint32_t */> 
struct CSValueTraits_T<uint32_t> {
  static uint32_t str2val(const std::string & str_val) /*throw(std::exception)*/
  {
    int ival = atoi(str_val.c_str());
    if (!ival) { //check for '0'
      unsigned i = 0;
      while ((str_val.c_str()[i] == '0') && (i < str_val.size()))
        ++i;
      if (i < str_val.size())
        throw CSValueException("uint32_t", str_val.c_str());
    }
    return (uint32_t)ival;
  }

  static std::string val2str(const uint32_t & use_val) /*throw()*/
  {
    char buf[sizeof(uint32_t)*3+2];
    snprintf(buf, sizeof(buf), "%u", use_val);
    return std::string(buf);
  }
};

} //csv
} //util
} //smsc

#endif /* __SMSC_UTIL_CSV_VALUE_TRAITS_INT_HPP */

