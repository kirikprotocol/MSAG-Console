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

#include <vector>

namespace smsc {
namespace router {

using smsc::smeman::SmeProxy;
using smsc::smeman::SmeManager;

struct RoutePattern
{
	union
	{
		struct
		{
			uint8_t dest_typeOfNumber;
			uint8_t src_typeOfNumber;
			uint8_t dest_numberingPlan;
			uint8_t src_numberingPlan;
		};
		int32_t num_n_plan;
	};
	uint8_t[21] src_addressMask;
	uint8_t[21] dest_addressMask;
	uint8_t[21] src_addressPattern;
	uint8_t[21] dest_addressPattern;
};

struct RouteAddress
{
	union
	{
		struct
		{
			uint8_t dest_typeOfNumber;
			uint8_t src_typeOfNumber;
			uint8_t dest_numberingPlan;
			uint8_t src_numberingPlan;
		};
		int32_t num_n_plan;
	};
	uint8_t[21] src_address;
	uint8_t[21] dest_address;
};

struct RouteRecord 
{
	RouteInfo info;
  SmeProxy* proxy;
	RouteIdentifier rid;
  int idx;
};

class RouteManager : public RouteAdmin, public RouteTable
{
  std::vector< auto_ptr<RouteRecord> > routes;
	RouteRecord* map;
	int map_size;
	int map_ptr;
	//
public :
	RouteManager();
	~RouteManager();
  void assignWithSmeManager(SmeManager* smeman); // for detach call with NULL;
  RouteIterator* iterator();
  // RouteAdministrator implementaion 
  virtual void addRoute(const RouteInfo& routeInfo);
  /*virtual void removeRoute(RouteId id);
  virtual void modifyRoute(RouteId id,const RouteInfo& routeInfo);*/
  // RoutingTable implementation
  virtual int lookup( const smsc::sms::Address& source, const smsc::sms::Address& dest);
  virtual RouteInfo getRouteInfo(int idx);
  virtual SmeProxy* getSmeProxy(int idx);
};

}; // namespace route
}; // namespace smsc

#endif
