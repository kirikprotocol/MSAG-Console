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
	RouteRegistry(){}

	virtual ~RouteRegistry();

	bool putRoute(const RouteInfo& route, SmeProxy* proxy);
	
	void clear();
	
	//RouteIterator* iterator();
	
	const RouteHolder* getRoute(RouteId routeId) const;

	const RouteHolder* lookup(const Address& origAddr,
		const Address& destAddr) const;

	int size() const;

	void saveConfig(const char* configFileName);
	
private:
	typedef map<const RouteId, const RouteHolder*> RouteMap;
	typedef map<const Address, const RouteHolder*, ltAddress> AddressMap2;
	typedef map<const Address, AddressMap2, ltAddress> AddressMap;
	RouteMap routeMap;
	AddressMap addrMap; //поиск маршрута сначала по destAddr, затем по origAddr

	const RouteHolder* RouteRegistry::lookup2(const AddressMap2& addrMap2,
		const Address& origAddr) const;
};

}
}
}

#endif /* TEST_CORE_ROUTE_REGISTRY */

