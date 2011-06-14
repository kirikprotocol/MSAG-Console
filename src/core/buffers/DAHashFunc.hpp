/* ************************************************************************** *
 * Direct access hash table: hash function definition.  
 * Note: assumed that hash table size type is uint32_t.
 * ************************************************************************** */
#ifndef __CORE_BUFFERS_DIRECT_ACCESS_HASH_FUNC_DEF
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __CORE_BUFFERS_DIRECT_ACCESS_HASH_FUNC_DEF

#include <inttypes.h>

namespace smsc {
namespace core {
namespace buffers {

// -----------------------------------------------------------------
// Default direct access hash function:
// -----------------------------------------------------------------
struct DAHashFunc_T {
  template < class _KeyArg > //Method is to implement!
  static inline uint32_t hashKey(const _KeyArg & use_key, uint16_t num_attempt) /*throw(std::exception)*/;
};

}//buffers
}//core
}//smsc

#endif /* __CORE_BUFFERS_DIRECT_ACCESS_HASH_FUNC_DEF */

