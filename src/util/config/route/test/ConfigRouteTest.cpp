#include <util/config/route/RouteConfig.h>
#include <iostream>

using smsc::util::config::route;

int main(int argc, const char * const * const argv)
{
	route::RouteConfig rc;
	rc.load("./routes.xml");
	route::Route *r;
	for (route::RouteConfig::RouteIterator ri = rc.getRouteIterator(); ri.fetchNext(r) == route::RouteConfig::success;)
	{
		std::cout << "==========================" << std::endl << r->getId()
		          << std::endl << "Sources:" << std::endl;
		char * key;
		route::Source s;
		for (route::SourceHash::Iterator i = r->getSources().getIterator(); i.Next(key, s);)
		{
			std::cout << s.getId() << std::endl;
		}
		std::cout << "Destinations:" << std::endl;
		route::Destination d;
		for (route::DestinationHash::Iterator i = r->getDestinations().getIterator(); i.Next(key, d);)
		{
			std::cout << d.getId() << "(" << d.getSmeId() << ")" << std::endl;
		}
	}

	rc.store("route.copy.xml");
}
