#ifndef TEST_CONFIG_CONFIG_UTIL
#define TEST_CONFIG_CONFIG_UTIL

#include "test/core/SmeRegistry.hpp"
#include "test/core/AliasRegistry.hpp"
#include "test/core/RouteRegistry.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace config {

using std::pair;
using smsc::sms::Address;
using smsc::smeman::SmeSystemId;
using smsc::test::core::SmeRegistry;
using smsc::test::core::AliasRegistry;
using smsc::test::core::RouteRegistry;
using smsc::test::core::SmeType;

class ConfigUtil
{
	SmeRegistry* smeReg;
	AliasRegistry* aliasReg;
	RouteRegistry* routeReg;
public:
	ConfigUtil(SmeRegistry* _smeReg, AliasRegistry* _aliasReg,
		RouteRegistry* _routeReg)
	: smeReg(_smeReg), aliasReg(_aliasReg), routeReg(_routeReg)
	{
		__require__(smeReg);
		__require__(aliasReg);
		__require__(routeReg);
	}
	void setupSystemSme();
	void setupSystemSmeRoutes();

	void setupRoute(const Address& srcAddr, const Address& destAddr,
		const SmeSystemId destSmeId);
	void setupDuplexRoutes(const Address& addr1, const SmeSystemId smeId1,
		const Address& addr2, const SmeSystemId smeId2);
	
	pair<SmeType, SmeSystemId> checkRoute(const Address& origAddr,
		const SmeSystemId& origSmeId, const Address& destAlias);
	
	void checkRoute2(const Address& origAddr, const SmeSystemId& origSmeId,
		const Address& destAlias);
	
	bool checkAlias(const Address& addr);
};

}
}
}

#endif /* TEST_CONFIG_CONFIG_UTIL */

