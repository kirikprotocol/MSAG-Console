/* ************************************************************************** *
 * Universal Mobile Telecommunications System:
 *   Numbering, addressing and identification types.
 * ************************************************************************** */
#ifndef __SMSC_UTIL_HEXDEC_STRING_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_UTIL_HEXDEC_STRING_HPP

#include <inttypes.h>
#include "inman/common/RCHStringT.hpp"

namespace smsc {
namespace util {

extern RCHAlphaBet  _hexdecStrAlphaBet;
extern uint8_t      _hexdecCvtTable[22];

template <
    unsigned _MAX_SZ_Arg
  , unsigned _MIN_SZ_Arg = 0
>
class HexString_T  : public RCHFixedString_T<_MAX_SZ_Arg, _MIN_SZ_Arg> {
public:
  HexString_T()
    : RCHFixedString_T<_MAX_SZ_Arg, _MIN_SZ_Arg>(_hexdecStrAlphaBet)
  { }
  ~HexString_T()
  { }

  static bool validateChars(const char * str_val)
  {
    return RCHFixedString_T<_MAX_SZ_Arg, _MIN_SZ_Arg>::validateChars(_hexdecStrAlphaBet, str_val);
  }

  //NOTE: char is to convert must have a valid value!
  static uint8_t char2uint(char ch_val)
  {
    int chIdx = _hexdecStrAlphaBet.getCharIdx(ch_val);
    return (chIdx < 0) ? (uint8_t)-1 : _hexdecCvtTable[chIdx];
  }

  //Returns EOS char in case of uknown binary value
  static char uint2char(uint8_t bin_val)
  {
    const uint8_t * pInt = (const uint8_t *)
      memchr(_hexdecCvtTable, bin_val, sizeof(_hexdecCvtTable)/sizeof(uint8_t));
    return pInt ? _hexdecStrAlphaBet._charsSet[pInt - _hexdecCvtTable]
                : _hexdecStrAlphaBet._charEOS;
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

}//util
}//smsc
#endif /* __SMSC_UTIL_HEXDEC_STRING_HPP */

