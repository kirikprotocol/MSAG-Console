#ifndef TEST_CORE_SME_UTIL
#define TEST_CORE_SME_UTIL

#include "AliasRegistry.hpp"
#include "RouteRegistry.hpp"
#include "smpp/smpp_structures.h"
#include <string>
#include <ostream>
#include <vector>

namespace smsc {
namespace test {
namespace core {

using std::string;
using std::ostream;
using std::vector;
using smsc::sms::Address;
using smsc::smpp::PduDeliverySm;

class RouteChecker
{
	const AliasRegistry* aliasReg;
	const RouteRegistry* routeReg;
public:
	RouteChecker(const AliasRegistry* aliasReg, const RouteRegistry* routeReg);

	vector<int> checkRouteForNormalSms(PduDeliverySm& pdu1, PduDeliverySm& pdu2,
		const string& systemId, const Address& smeAddr);

	vector<int> checkRouteForNotification(PduDeliverySm& pdu1, PduDeliverySm& pdu2,
		const string& systemId, const Address& smeAddr, const Address& smscAddr);
};

}
}
}

#endif /* TEST_CORE_SME_UTIL */

