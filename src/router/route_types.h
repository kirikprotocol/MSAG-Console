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
static const RoutePriority RoutePriorityMax = 32767;
static const RoutePriority RoutePriorityMin = 1;

typedef std::string RouteId;

struct RouteInfo
{
  RoutePriority priority;
  bool billing;
  bool archived;
  bool enabling;
  int serviceId;
  RouteId routeId;
  smsc::smeman::SmeSystemId smeSystemId;
  smsc::sms::Address source;
  smsc::sms::Address dest;
};

}; // namespace router
}; // namespcase smsc

#endif
