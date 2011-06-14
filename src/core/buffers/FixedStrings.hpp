/* ************************************************************************** *
 * Template of char string with fixed length not grater than 64K.
 * String is represented as capacity prefixed array of chars.
 * Note: the main specific of template is singled out interface.
 * ************************************************************************** */
#ifndef __SMSC_CORE_BUFFERS_FIXED_STRING_64K_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_CORE_BUFFERS_FIXED_STRING_64K_HPP

#include <inttypes.h>
#include <string.h>

namespace smsc {
namespace core {
namespace buffers {

template < uint16_t MAX_SZ_Arg > 
struct PrefixedBuffer_T {
  uint16_t  _capSz;
  char      _buf[MAX_SZ_Arg];
};


class FixedStringIface {
private:
  PrefixedBuffer_T<1> & _rStore;

protected:
  explicit FixedStringIface(PrefixedBuffer_T<1> & use_store)
    : _rStore(use_store)
  {
    _rStore._buf[0] = 0;
  }
  //
  ~FixedStringIface()
  { }

public:
  typedef uint16_t  size_type;


  size_type capacity(void) const { return _rStore._capSz; }

  const char * c_str(void) const {  return _rStore._buf; }

  bool empty(void) const { return (_rStore._buf[0] == 0); }

  size_type length(void) const { return (size_type)strlen(c_str()); }

  size_type size(void) const { return length(); }

  void clear(void) { _rStore._buf[0] = 0; }

  void assign(const char * use_str, size_type max_len)
  {
    if (max_len >= capacity())
      max_len = capacity() - 1;
    memcpy(v_str(), use_str, max_len);
    v_str()[max_len] = 0;
  }

  //Exposes internal buffer.
  //NOTE: use this with extreme caution, be sure to not overflow buffer!!!
  char * v_str(void) { return _rStore._buf; }

  FixedStringIface & operator+=(const char use_char)
  {
    size_type len = (size_type)strlen(v_str());
    if ((len + 1) < capacity()) {
      v_str()[len] = use_char;
      v_str()[len + 1] = 0;
    }
    return *this;
  }

  FixedStringIface & operator+=(const char * use_str)
  {
    strncat(v_str(), use_str, capacity()-1);
    v_str()[capacity() - 1] = 0;
    return *this;
  }

  FixedStringIface & operator+=(const FixedStringIface & use_str)
  {
    strncat(v_str(), use_str.c_str(), capacity()-1);
    v_str()[capacity() - 1] = 0;
    return *this;
  }

  FixedStringIface & operator=(const char * use_str)
  {
    strncpy(v_str(), use_str, capacity()-1);
    v_str()[capacity() - 1] = 0;
    return *this;
  }

  FixedStringIface & operator=(const FixedStringIface & use_str)
  {
    strncpy(v_str(), use_str.c_str(), capacity()-1);
    v_str()[capacity() - 1] = 0;
    return *this;
  }

  char operator[](size_type at_pos) const { return c_str()[at_pos]; }

  char & operator[](size_type at_pos) { return v_str()[at_pos]; }

  bool operator==(const char * use_str) const { return !strcmp(c_str(), use_str); }

  bool operator==(const FixedStringIface & use_str) const
  {
    return (*this == use_str.c_str());
  }

  bool operator!=(const char * use_str) const { return !(*this == use_str); }

  bool operator!=(const FixedStringIface & use_str) const
  {
    return !(*this == use_str.c_str());
  }

  bool operator<(const char * use_str)const { return strcmp(c_str(), use_str) < 0; }

  bool operator<(const FixedStringIface & use_str) const
  {
    return (*this < use_str.c_str());
  }
};

template < 
  uint16_t MAX_SZ_Arg //max size of buffer, take in account terminating zero!
> 
class FixedString_T : public FixedStringIface {
protected:
  union {
    PrefixedBuffer_T<1>           _base;
    PrefixedBuffer_T<MAX_SZ_Arg>  _full;
  } _store;

public:
  static const size_type npos = MAX_SZ_Arg;
  static const size_type MAX_SZ = MAX_SZ_Arg;

  FixedString_T() : FixedStringIface(_store._base)
  {
    _store._base._capSz = MAX_SZ_Arg;
  }
  explicit FixedString_T(const char * use_str)
    : FixedStringIface(_store._base)
  {
    _store._base._capSz = MAX_SZ_Arg;
    *(FixedStringIface*)this = use_str;
  }
  explicit FixedString_T(const FixedString_T & use_str)
    : FixedStringIface(_store._base)
  {
    _store._base._capSz = MAX_SZ_Arg;
    *(FixedStringIface*)this = use_str.c_str();
  }
  //
  ~FixedString_T()
  { }
};


}//namespace buffers
}//namespace core
}//namespace smsc


#endif /* __SMSC_CORE_BUFFERS_FIXED_STRING_64K_HPP */

