/* ************************************************************************** *
 * Universal Mobile Telecommunications System: location identification types.
 * ************************************************************************** */
#ifndef __INMAN_UMTS_TYPES_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_UMTS_TYPES_HPP

#include "inman/common/NumericStringT.hpp"
#include "inman/common/HexStringT.hpp"

namespace smsc {
namespace inman {
  
typedef smsc::util::NumericString_T<3,3> MobileCountryCode;
typedef smsc::util::NumericString_T<3,2> MobileNetworkCode;

typedef smsc::util::HexString_T<4, 4> LocationAreaCode;
typedef smsc::util::HexString_T<4, 4> CellIdentity;

}//inman
}//smsc
#endif /* __INMAN_UMTS_TYPES_HPP */

