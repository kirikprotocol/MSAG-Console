#ifndef TEST_CORE_SME_UTIL
#define TEST_CORE_SME_UTIL

#include "SmeRegistry.hpp"
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
using smsc::smpp::PduAddress;
using smsc::smpp::PduDeliverySm;

class RouteChecker
{
	const string systemId;
	const Address smeAddr;
	const SmeRegistry* smeReg;
	const AliasRegistry* aliasReg;
	const RouteRegistry* routeReg;
public:
	RouteChecker(const string& systemId, const Address& smeAddr,
		const SmeRegistry* smeReg, const AliasRegistry* aliasReg,
		const RouteRegistry* routeReg);

	vector<int> checkRouteForNormalSms(PduDeliverySm& pdu1, PduDeliverySm& pdu2);

	vector<int> checkRouteForNotification(PduDeliverySm& pdu1, PduDeliverySm& pdu2);

	bool isDestUnreachable(PduAddress& destAddr);
};

}
}
}

#endif /* TEST_CORE_SME_UTIL */

