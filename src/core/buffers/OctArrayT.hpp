/* ************************************************************************** *
 * Lightweight Array of octets with length up to 64k.
 * ************************************************************************** */
#ifndef __CORE_BUFFERS_OCT_ARRAY16_DEFS
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __CORE_BUFFERS_OCT_ARRAY16_DEFS

#include "core/buffers/LWArrayTraitsInt.hpp"
#include "core/buffers/LWArrayT.hpp"

namespace smsc  {
namespace core {
namespace buffers {

typedef LWArrayExtension_T<uint8_t, uint16_t, LWArrayTraitsPOD_T>
  OctArray16Iface;

template < uint16_t _StackSzArg >
class OctArray16_T : public 
  LWArray_T<uint8_t, uint16_t, _StackSzArg, LWArrayTraitsPOD_T> {
public:
  explicit OctArray16_T(uint16_t num_to_reserve = 0) 
    : LWArray_T<uint8_t, uint16_t, _StackSzArg, LWArrayTraitsPOD_T>(num_to_reserve)
  { }
  ~OctArray16_T()
  { }
};


} //buffers
} //core
} //smsc

#endif /* __CORE_BUFFERS_OCT_ARRAY16_DEFS */

