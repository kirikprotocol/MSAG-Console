/* ************************************************************************** *
 * Restricted Characters Set String: NumericString
 * ************************************************************************** */
#ifndef __SMSC_UTIL_NUMERIC_STRING_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_UTIL_NUMERIC_STRING_HPP

#include <inttypes.h>
#include "inman/common/RCHStringT.hpp"

namespace smsc {
namespace util {

extern RCHAlphaBet  _numericStrAlphaBet;

template <
    unsigned _MAX_SZ_Arg
  , unsigned _MIN_SZ_Arg = 0
>
class NumericString_T : public RCHFixedString_T<_MAX_SZ_Arg, _MIN_SZ_Arg> {
public:
  NumericString_T()
    : RCHFixedString_T<_MAX_SZ_Arg, _MIN_SZ_Arg>(_numericStrAlphaBet)
  { }
  ~NumericString_T()
  { }

  static bool validateChars(const char * str_val)
  {
    return RCHFixedString_T<_MAX_SZ_Arg, _MIN_SZ_Arg>::validateChars(_numericStrAlphaBet, str_val);
  }

  //NOTE: char is to convert must have a valid value!
  static uint8_t char2uint(char ch_val)
  {
    int chIdx = _numericStrAlphaBet.getCharIdx(ch_val);
    return (chIdx < 0) ? (uint8_t)-1 : (ch_val - '0');
  }

  //Returns EOS char in case of uknown binary value
  static char uint2char(uint8_t bin_val)
  {
    return (bin_val <= 0x09) ? ('0' + bin_val) : _numericStrAlphaBet._charEOS;
  }

  uint8_t char2uintByIdx(unsigned ch_idx) const
  {
    return char2uint(this->_value[ch_idx]);
  }

  void uint2charByIdx(unsigned ch_idx, uint8_t bin_val)
  {
    this->_value[ch_idx] = uint2char(bin_val);
  }
};

}//inman
}//smsc
#endif /* __SMSC_UTIL_NUMERIC_STRING_HPP */

