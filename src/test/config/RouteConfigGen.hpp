#ifndef TEST_CONFIG_ROUTE_CONFIG_GEN
#define TEST_CONFIG_ROUTE_CONFIG_GEN

#include "test/core/RouteRegistry.hpp"
#include "test/sms/SmsUtil.hpp"
#include "ConfigGen.hpp"
#include "test/util/CheckList.hpp"
#include "util/debug.h"
#include <fstream>
#include <string>
#include <vector>

namespace smsc {
namespace test {
namespace config {

using std::ostream;
using std::string;
using std::vector;
using smsc::router::RouteInfo;
using smsc::test::core::RouteRegistry;
using smsc::test::util::CheckList;
using smsc::sms::Address;

//компараторы

class RouteConfigGen : public ConfigGen
{
	const RouteRegistry* routeReg;
	CheckList* chkList;

public:
	RouteConfigGen(const RouteRegistry* _routeReg, CheckList* _chkList)
		: routeReg(_routeReg), chkList(_chkList)
	{
		__require__(routeReg);
		//__require__(chkList);
	}
	virtual ~RouteConfigGen() {}
	virtual void saveConfig(const char* configFileName);
	
private:
	bool checkSubject(const Address& addr);
	const string genFakeAddress();
	bool printFakeMask(ostream& os);
	void printSubject(ostream& os, const RouteInfo& route, char type);
	void printRouteStart(ostream& os, const RouteInfo& route);
	void printRouteEnd(ostream& os);
	void printSource(ostream& os, const RouteInfo& route);
	bool printFakeSource(ostream& os, const RouteInfo& route);
	void printDest(ostream& os, const RouteInfo& route);
	bool printFakeDest(ostream& os, const RouteInfo& route);
	void printSubjects(ostream& os);
	void printRoutes(ostream& os);
};

}
}
}

#endif /* TEST_CONFIG_ROUTE_CONFIG_GEN */

