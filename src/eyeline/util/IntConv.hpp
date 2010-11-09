/* ************************************************************************* *
 * Various integer types conversion helpers.
 * ************************************************************************* */
#ifndef __INTEGER_CONVERSIONS_DEFS
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INTEGER_CONVERSIONS_DEFS

#include <inttypes.h>

namespace eyeline {
namespace util {

//Macro that perfoms adaptive downcast of unsigned integer value.
#define DOWNCAST_UNSIGNED(val, tgt_type)  (((val) > (tgt_type)(-1)) ? (tgt_type)(-1) : (tgt_type)(val))

//Template, that converts integer from binary to decimal form
//avoiding stdlib usage
template <
  typename _SizeTypeArg /* must be an unsigned integer type! */ 
> 
class UIntToA_T { //converts unsigned integer type to its decimal string representation
protected:
  uint8_t _idx;
  char _buf[sizeof(_SizeTypeArg)*3+1];

public:
  UIntToA_T(_SizeTypeArg use_val)
    : _idx(sizeof(_SizeTypeArg)*3)
  {
    _buf[_idx] = 0;
    do {
      _buf[--_idx] = '0' + (char)(use_val % 10);
    } while ((use_val /= 10) != 0);
  }
  ~UIntToA_T()
  { }

  const char * get(void) const { return _buf + _idx; }
};


} //util
} //eyeline

#endif /* __INTEGER_CONVERSIONS_DEFS */

