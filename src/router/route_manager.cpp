/*
*/

#include "route_manager.h"

namespace smsc {
namespace router{

void RouteManager::assignWithSmeManager(SmeManager* smeman) // for detach call with NULL
{
}

RouteInfoIterator* RouteManager::iterator()
{
}

// RouteAdministrator implementaion 
void RouteManager::addRoute(const RouteInfo& routeInfo)
{
}

void RouteManager::removeRoute(RouteId id)
{
}

void RouteManager::modifyRoute(RouteId id,const RouteInfo& routeInfo)
{
}
  
// RoutingTable implementation
int RouteManager::lookup( const smsc::sms::Address& source, const smsc::sms::Address& dest)
{
}

RouteInfo RouteManager::getRouteInfo(int idx)
{
}

SmeProxy* RouteManager::getSmeProxy(int idx)
{
}

}; // namespace router
}; // namespace smsc

