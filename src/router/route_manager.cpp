/*
	$Id$
*/

#include "route_manager.h"

namespace smsc {
namespace router{


#define check_src(n) \
	((pattern.src_addressMask[n] & addr.src_address[n]) \
		^ pattern.src_addressPattern[n])

#define check_dest(n) \
	((pattern.dest_addressMask[n] & addr.dest_address[n]) \
		^ pattern.dest_addressPattern[n])


static inline bool is_a ( const RouteIdentifier& pattern, const Route& addr )
{
	if ( pattern.num_n_plan != addr.num_n_plan ) return false;
	if ( check_src(0) |
				check_src(1) |
				check_src(2) |
				check_src(3) |
				check_src(4) |
				check_src(5) |
				check_src(6) |
				check_src(7) |
				check_src(8) |
				check_src(9) |
				check_src(10) |
				check_src(11) |
				check_src(12) |
				check_src(13) |
				check_src(14) |
				check_src(15) |
				check_src(16) |
				check_src(17) |
				check_src(18) |
				check_src(19) |
				check_src(20) ) return false
	if ( check_dest(0) |
				check_dest(1) |
				check_dest(2) |
				check_dest(3) |
				check_dest(4) |
				check_dest(5) |
				check_dest(6) |
				check_dest(7) |
				check_dest(8) |
				check_dest(9) |
				check_dest(10) |
				check_dest(11) |
				check_dest(12) |
				check_dest(13) |
				check_dest(14) |
				check_dest(15) |
				check_dest(16) |
				check_dest(17) |
				check_dest(18) |
				check_dest(19) |
				check_dest(20) ) return false;
	
	return true;
}


void RouteManager::assignWithSmeManager(SmeManager* smeman) // for detach call with NULL
{
}

RouteIterator* RouteManager::iterator()
{
}

void insert_into_map(int idx, RouteRecord* record)
{
	__require__ ( idx >= 0 );
	__require__ ( route_map_size >= idx);
	__require__ ( route_length >= idx);
	__require__ ( record != NULL );
	
	if ( route_map_size == idx )
	{
		RouteRecord* map = new RouteRecord[route_map_size+256];
		if ( route_map )
		{
			memcpy(map,route_map,route_map_size*sizeof(RouteRecord*));
			delete route_map;
		}
		route_map = map;
		route_map_size = route_map_size+256; 
	}
	if ( route_map_length > idx )
	{
		memmove(route_map+idx+1,route_map+idx,sizeof(RouteRecord*)*(route_map_length-idx))
	}
	route_map[idx] = record;
}

// RouteAdministrator implementaion 
void RouteManager::addRoute(const RouteInfo& routeInfo)
{
	auto_ptr<RouteRecord> record(new RouteRecord(routeInfo));
	int idx;
	if ( findEqualOrLess(record.get(),&idx) )
	{
		throw RouteAlreadyExists();
	}
	inset_into_map(idx,record.get());
	record->idx = routes.size();
	routes.push_back(record);
}

/*void RouteManager::removeRoute(RouteId id)
{
}*/

/*void RouteManager::modifyRoute(RouteId id,const RouteInfo& routeInfo)
{
}*/
  
// RoutingTable implementation
int RouteManager::lookup( const smsc::sms::Address& source, const smsc::sms::Address& dest)
{
	RouteAddress address;
	make_address(&address,&source,&dest);
	std::vector<__RouteRecord>::iterator it = b_search(records.begin(),records.end(),__RouteRecord(address));
	if ( it == records.end() ) return ROUTE_RECORD_NOT_FOUND;
	return ((*it)-records.begin());
}

RouteInfo RouteManager::getRouteInfo(int idx)
{
	return records.at(idx);
}

SmeProxy* RouteManager::getSmeProxy(int idx)
{
}

}; // namespace router
}; // namespace smsc

