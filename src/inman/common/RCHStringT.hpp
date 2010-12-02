/* ************************************************************************** *
 * Restricted Characters Set String.
 * ************************************************************************** */
#ifndef __SMSC_UTIL_RCHSTRING_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_UTIL_RCHSTRING_HPP

#include <string.h>

namespace smsc {
namespace util {

struct RCHAlphaBet {
  const char *  _charsSet;
  char          _charEOS;   //End-Of-String char

  explicit RCHAlphaBet(const char * chars_set, char not_a_char = 0)
    : _charsSet(chars_set), _charEOS(not_a_char)
  { }

  //Returns index of char in alphaBet or negative value in case of illegal char.
  int getCharIdx(char ch_val) const
  {
    const char * pChar = strchr(_charsSet, ch_val);
    return pChar ? (int)(pChar - _charsSet) : -1;
  }
};

/* -------------------------------------------------------------------------- *
 * Restricted Characters Set String of fixed length.
 * -------------------------------------------------------------------------- */
template <
    unsigned _MAX_SZ_Arg
  , unsigned _MIN_SZ_Arg = 0
>
class RCHFixedString_T {
protected:
  const RCHAlphaBet * _alphaBet;

  static unsigned length(const char char_EOS, const char * str_val)
  {
    unsigned i = 0;
    while ((i < _MAX_SZ_Arg) && (str_val[i] != char_EOS))
       ++i;
    return i;
  }

  static bool validateChars(const RCHAlphaBet & use_alhabet, const char * str_val)
  {
    for (unsigned i = 0; (i < _MAX_SZ_Arg) && (str_val[i] != use_alhabet._charEOS); ++i) {
      if (use_alhabet.getCharIdx(str_val[i]) < 0)
        return false;
    }
    return true;
  }

public:
  static const unsigned _MAX_SZ = _MAX_SZ_Arg;
  static const unsigned _MIN_SZ = _MIN_SZ_Arg;

  char _value[_MAX_SZ_Arg];

  explicit RCHFixedString_T(const RCHAlphaBet & use_alhabet)
    : _alphaBet(&use_alhabet)
  {
    _value[0] = _alphaBet->_charEOS;
  }
  ~RCHFixedString_T()
  { }

  bool empty(void) const
  {
    return (_value[0] == _alphaBet->_charEOS);
  }

  unsigned length(void) const
  {
    return length(_alphaBet->_charEOS, _value);
  }

  bool validate(void) const
  {
    return length() && validateChars(*_alphaBet, _value);
  }

  void clear(void)
  {
    _value[0] = _alphaBet->_charEOS;
  }
};

}//util
}//smsc
#endif /* __SMSC_UTIL_RCHSTRING_HPP */

