/*
        $Id$
*/

#if !defined __Cpp_Header__router_types_h__
#define __Cpp_Header__router_types_h__

#include "smeman/smetypes.h"
#include <string>
#include "sms/sms.h"

namespace smsc {
namespace router {

typedef int RoutePriority;
static const RoutePriority RoutePriorityMax = 32000;
static const RoutePriority RoutePriorityMin = 0;

typedef std::string RouteId;

struct RouteInfo
{
  RoutePriority priority;
  bool billing;
  bool archived;
  bool enabling;
  bool suppressDeliveryReports;
  bool hide;
  bool forceReplyPath;
  int serviceId;
  RouteId routeId;
  smsc::smeman::SmeSystemId smeSystemId;
  smsc::smeman::SmeSystemId srcSmeSystemId;
  smsc::sms::Address source;
  smsc::sms::Address dest;
  uint8_t deliveryMode;
  std::string forwardTo;
  std::string srcSubj; // for duplucate route debugging purposes only
  std::string dstSubj;
};

} // namespace router
} // namespcase smsc

#endif
