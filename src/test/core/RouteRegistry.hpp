#ifndef TEST_CORE_ROUTE_REGISTRY
#define TEST_CORE_ROUTE_REGISTRY

#include "sms/sms.h"
#include "smeman/smetypes.h"
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
using smsc::smeman::SmeSystemId;
using smsc::test::sms::ltAddress;

class RouteRegistry
{
public:
	RouteRegistry();
	virtual ~RouteRegistry();

	void putRoute(const TestRouteData& data);
	
	/**
	 * @return адрес должен удал€тьс€ вызывающей стороной.
	 */
	const Address* getRandomReachableDestAddress(const Address& origAddr);

	/**
	 * @return адрес должен удал€тьс€ вызывающей стороной.
	 */
	const Address* getRandomNonReachableDestAddress(const Address& origAddr);

	const SmeSystemId* lookup(const Address& origAddr, const Address& destAddr);

private:
	typedef vector<const TestRouteData*> RouteList;
	typedef map<const Address, RouteList, ltAddress> AddressMap;
	AddressMap addrMap;
};

}
}
}

#endif /* TEST_CORE_ROUTE_REGISTRY */

