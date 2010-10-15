#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "eyeline/ros/proto/enc/RERResultPdu.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace enc {

void REReturnResult::setValue(const ros::ROSPduWithArgument & use_val) /*throw(std::exception)*/
{
  _invId.setValue(use_val.getHeader()._invId);

  if (use_val.hasArgument()) {
    _result.init(getTSRule()).setValue(use_val.getHeader()._opCode, use_val.getArg());
    setField(1, *_result.get());
  } else
    clearField(1);
}


}}}}

