/*
        $Id$
*/

#if !defined __Cpp_Header__route_manager_h__
#define __Cpp_Header__route_manager_h__

#include "route_types.h"
#include "smeman/smeman.h"
#include "route_iter.h"
#include "route_admin.h"
#include "route_table.h"
#include "sms/sms.h"

namespace smsc {
namespace router {

using smsc::smeman::SmeProxy;
using smsc::smeman::SmeManager;

struct RouteRecord 
{
	RouteUnfo info;
	SmeProxy* proxy;
	int idx;
};

class RouteManager : public RouteAdmin, public RouteManager
{
        //
public :
  void assignWithSmeManager(SmeManager* smeman); // for detach call with NULL;
  RouteInfoIterator* iterator();
  // RouteAdministrator implementaion 
  virtual void addRoute(const RouteInfo& routeInfo);
  virtual void removeRoute(RouteId id);
  virtual void modifyRoute(RouteId id,const RouteInfo& routeInfo);
  // RoutingTable implementation
  virtual int lookup( const smsc::sms::Address& source, const smsc::sms::Address& dest);
  virtual RouteInfo getRouteInfo(int idx);
  virtual SmeProxy* getSmeProxy(int idx);
};

}; // namespace route
}; // namespace smsc

#endif
