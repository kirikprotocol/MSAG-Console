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

	vector<int> checkRouteForNormalSms(PduDeliverySm& pdu1, PduDeliverySm& pdu2) const;

	vector<int> checkRouteForNotification(PduDeliverySm& pdu1, PduDeliverySm& pdu2) const;

	/**
	 * ѕровер€ет среди всех доступных маршрутов наличие хот€ бы одного маршрута,
	 * по которому доставка возможна.
	 * @param checkSme дополнительно провер€ет на bound sme.
	 */
	bool checkExistsReachableRoute(PduAddress& destAddr, bool checkSme) const;

	/**
	 * ѕровер€ет среди всех доступных маршрутов наличие хот€ бы одного маршрута,
	 * по которому доставка не возможна.
	 * @param checkSme дополнительно провер€ет на bound sme.
	 */
	bool checkExistsUnreachableRoute(PduAddress& destAddr, bool checkSme) const;
};

}
}
}

#endif /* TEST_CORE_SME_UTIL */

