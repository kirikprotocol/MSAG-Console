/* ************************************************************************** *
 * Direct access hash table: hash function specialization for char* key type.
 * Note: assumed that hash table size type is uint32_t.
 * ************************************************************************** */
#ifndef __CORE_BUFFERS_DIRECT_ACCESS_HASH_FUNC_CSTR
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __CORE_BUFFERS_DIRECT_ACCESS_HASH_FUNC_CSTR

#include "core/buffers/DAHashFunc.hpp"

namespace smsc {
namespace core {
namespace buffers {

// -----------------------------------------------------------------
// Hash key: const char*
// -----------------------------------------------------------------
template <> inline uint32_t
  DAHashFunc_T::hashKey(const char * const & use_key, uint16_t num_attempt)
    /*throw(std::exception)*/
{
  const char * pCurr = use_key;
  uint32_t hSumm = *pCurr;

  while (*pCurr) {
    hSumm += (37 * hSumm) + *pCurr;
    pCurr++;
  }
  hSumm = (uint32_t)((( hSumm * 19U) + 12451U) % 8882693UL);
  return hSumm + num_attempt;
}

}//buffers
}//core
}//smsc

#endif /* __CORE_BUFFERS_DIRECT_ACCESS_HASH_FUNC_CSTR */

