/* ************************************************************************** *
 * Direct access hash table: hash function specializations for integer key types.  
 * Note: assumed that hash table size type is uint32_t.
 * ************************************************************************** */
#ifndef __CORE_BUFFERS_DIRECT_ACCESS_HASH_FUNC_INTS
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __CORE_BUFFERS_DIRECT_ACCESS_HASH_FUNC_INTS

#include "core/buffers/DAHashFunc.hpp"

namespace smsc {
namespace core {
namespace buffers {

// -----------------------------------------------------------------
// Hash key: int16_t
// -----------------------------------------------------------------
template <> inline uint32_t
  DAHashFunc_T::hashKey(const int16_t & use_key, uint16_t num_attempt)
    /*throw(std::exception)*/
{
  return (uint32_t)use_key + num_attempt;
}

template <> inline uint32_t
  DAHashFunc_T::hashKey(const uint16_t & use_key, uint16_t num_attempt)
    /*throw(std::exception)*/
{
  return (uint32_t)use_key + num_attempt;
}

// -----------------------------------------------------------------
// Hash key: int32_t
// -----------------------------------------------------------------
template <> inline uint32_t
  DAHashFunc_T::hashKey(const int32_t & use_key, uint16_t num_attempt)
    /*throw(std::exception)*/
{
  return (uint32_t)(use_key + num_attempt);
}

template <> inline uint32_t
  DAHashFunc_T::hashKey(const uint32_t & use_key, uint16_t num_attempt)
    /*throw(std::exception)*/
{
  return (uint32_t)(use_key + num_attempt);
}

// -----------------------------------------------------------------
// Hash key: int64_t
// -----------------------------------------------------------------
template <> inline uint32_t
  DAHashFunc_T::hashKey(const int64_t & use_key, uint16_t num_attempt)
    /*throw(std::exception)*/
{
  return (uint32_t)(use_key + num_attempt);
}

template <> inline uint32_t
  DAHashFunc_T::hashKey(const uint64_t & use_key, uint16_t num_attempt)
    /*throw(std::exception)*/
{
  return (uint32_t)(use_key + num_attempt);
}

}//buffers
}//core
}//smsc

#endif /* __CORE_BUFFERS_DIRECT_ACCESS_HASH_FUNC_INTS */

