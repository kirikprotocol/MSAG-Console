/* ************************************************************************** *
 * 
 * ************************************************************************** */
#ifndef __SMSC_INMAN_IAPQUERY_HASHFUNC
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_IAPQUERY_HASHFUNC

#include "util/TonNpiAddress.hpp"
#include "core/buffers/DAHashFuncCStr.hpp"

namespace smsc {
namespace core {
namespace buffers {

// -----------------------------------------------------------------
// Hash key: TonNpiAddress
// -----------------------------------------------------------------
template <> inline uint32_t
  DAHashFunc_T::hashKey(const smsc::util::TonNpiAddress & use_key, uint16_t num_attempt)
    /*throw(std::exception)*/
{
  // NOTE: IAPqueries operates only with ISDN International addresses, so only
  // signals portion is used for hashCode calculation.
  return DAHashFunc_T::hashKey(use_key.getSignals(), num_attempt);
}

}//buffers
}//core
}//smsc


#endif /* __SMSC_INMAN_IAPQUERY_HASHFUNC */

