#ifndef TEST_CORE_ROUTE_REGISTRY
#define TEST_CORE_ROUTE_REGISTRY

#include "sms/sms.h"
#include "smeman/smetypes.h"
#include "router/route_types.h"
#include "RouteUtil.hpp"
#include "test/sms/SmsUtil.hpp"
#include <map>
#include <vector>

namespace smsc {
namespace test {
namespace core {

using std::map;
using std::vector;
using smsc::sms::Address;
using smsc::router::RouteId;
using smsc::test::sms::ltAddress;

class RouteRegistry
{
public:
	typedef vector<const TestRouteData*> RouteList;
	typedef map<const RouteId, const TestRouteData*> RouteMap;
	class RouteIterator
	{
		RouteMap::iterator it, end;
	public:
		RouteIterator(RouteMap::iterator b, RouteMap::iterator e);
		bool hasNext() const;
		const TestRouteData* operator*() const;
		const TestRouteData* operator->() const;
		RouteIterator& operator++();
		RouteIterator operator++(int);
	};

	RouteRegistry(){}

	virtual ~RouteRegistry();

	void putRoute(const TestRouteData& data);
	
	const RouteInfo* getRoute(RouteId routeId) const;

	const RouteList lookup(const Address& origAddr,
		const Address& destAddr) const;

	RouteIterator* iterator();

	int size() const;

private:
	typedef map<const Address, RouteList, ltAddress> AddressMap;
	AddressMap addrMap;
	RouteMap routeMap;
};

}
}
}

#endif /* TEST_CORE_ROUTE_REGISTRY */

