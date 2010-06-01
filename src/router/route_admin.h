
/*
        $Id$
*/

#if !defined __Cpp_Header__route_admin_h__
#define __Cpp_Header__route_admin_h__

#include "route_types.h"

namespace smsc {
namespace router {

// adbstract
class RouteAdmin
{
public :
  // RouteAdministrator implementaion
  virtual void addRoute(RouteInfo* routeInfo,const RoutePoint& rp) = 0;
  /*virtual void removeRoute(RouteId id) = 0;
  virtual void modifyRoute(RouteId id,const RouteInfo& routeInfo) = 0;*/
};

} // namespace route
} // namespace smsc

#endif
