#ifndef routemap_dot_h
#define routemap_dot_h

#include <string>
#include <core/buffers/IntHash.hpp>
#include <core/buffers/Hash.hpp>
#include <core/synchronization/Mutex.hpp>

namespace scag {
namespace stat {

using smsc::core::buffers::Hash;
using smsc::core::buffers::IntHash;
using smsc::core::synchronization::Mutex;

class RouteMap {
public:
	RouteMap();
	virtual ~RouteMap();
	int regRoute(const char *routeId, int &newRouteId);
	const char    *getStrRouteId(int routeId);
    int     getIntRouteId(const char * routeId);
protected:
	Mutex mutex;
	IntHash<std::string> strByInt;
	Hash<int> intByStr;
	int routeId_;
};

}
}

#endif

