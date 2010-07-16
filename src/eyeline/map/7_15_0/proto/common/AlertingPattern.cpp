#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/map/7_15_0/proto/common/AlertingPattern.hpp"

namespace eyeline {
namespace map {
namespace common {

AlertingPattern::Id_e AlertingPattern::value2Id(value_type use_val)
{
  return (use_val > AlertingPattern::alertingCategory_5) ? 
      alertingCategory_Rsrv : static_cast<AlertingPattern::Id_e>(use_val);
}

}}}

