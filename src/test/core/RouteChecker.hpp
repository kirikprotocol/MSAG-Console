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
using smsc::smeman::SmeInfo;
using namespace smsc::smpp; //PduAddress, PduSubmitSm, PduDeliverySm

class RouteChecker
{
	const SmeRegistry* smeReg;
	const AliasRegistry* aliasReg;
	const RouteRegistry* routeReg;

public:
	RouteChecker(const SmeRegistry* smeReg, const AliasRegistry* aliasReg,
		const RouteRegistry* routeReg);

	SmeType isDestReachable(const Address& srcAddr,
		const Address& destAlias) const;
	SmeType isDestReachable(PduAddress& srcAddr, PduAddress& destAlias) const;

	const RouteInfo* getRouteInfoForNormalSms(const Address& srcAddr,
		const Address& destAlias) const;
	const RouteInfo* getRouteInfoForNormalSms(PduAddress& srcAddr,
		PduAddress& destAlias) const;

	const RouteInfo* getRouteInfoForNotification(const Address& destAddr) const;
	const RouteInfo* getRouteInfoForNotification(PduAddress& destAddr) const;
};

}
}
}

#endif /* TEST_CORE_SME_UTIL */

