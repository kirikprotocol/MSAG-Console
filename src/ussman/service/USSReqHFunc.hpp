/* ************************************************************************** *
 * 
 * ************************************************************************** */
#ifndef __SMSC_USSMAN_USSOP_HASHFUNC
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_USSMAN_USSOP_HASHFUNC

#include "core/buffers/DAHashFuncCStr.hpp"
#include "ussman/comp/USSOperationData.hpp"

#include <string>
#include <stdexcept>

namespace smsc {
namespace core {
namespace buffers {

// -----------------------------------------------------------------
// Hash key: USSOperationData
// -----------------------------------------------------------------
template <> inline uint32_t
  DAHashFunc_T::hashKey(const smsc::ussman::comp::USSOperationData & use_key, uint16_t num_attempt)
    /*throw(std::exception)*/
{
  char tBuf[smsc::ussman::comp::USSOperationData::_max_strSZ];
  if (use_key.serialize(tBuf, (uint16_t)sizeof(tBuf)) >= (uint16_t)sizeof(tBuf))
    throw std::runtime_error("DAHashFunc_T<USSOperationData>::hashKey() - uss data is too long");

  return DAHashFunc_T::hashKey((const char *)&tBuf[0], num_attempt);
}

}//buffers
}//core
}//smsc


#endif /* __SMSC_USSMAN_USSOP_HASHFUNC */

