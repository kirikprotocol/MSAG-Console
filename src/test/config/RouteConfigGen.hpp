#ifndef TEST_CONFIG_ROUTE_CONFIG_GEN
#define TEST_CONFIG_ROUTE_CONFIG_GEN

#include "test/core/RouteRegistry.hpp"
#include "test/sms/SmsUtil.hpp"
#include "ConfigGen.hpp"
#include "util/debug.h"
#include <fstream>

namespace smsc {
namespace test {
namespace config {

using std::ofstream;
using std::vector;
using smsc::router::RouteInfo;
using smsc::test::core::RouteRegistry;
using smsc::test::sms::ltAddress;

//компараторы

class RouteConfigGen : public ConfigGen
{
	const RouteRegistry* routeReg;
	static ltAddress ltAddr;

public:
	RouteConfigGen(const RouteRegistry* _routeReg)
		: routeReg(_routeReg)
	{
		__require__(routeReg);
	}

	virtual ~RouteConfigGen() {}

	static bool ltSource(const RouteInfo* r1, const RouteInfo* r2);
	static bool ltDest(const RouteInfo* r1, const RouteInfo* r2);
	
	virtual void saveConfig(const char* configFileName);
	
private:
	void printSubject(ofstream& os, vector<const RouteInfo*>& routes,
		int idx, char type, const char* prefix);
	void printRouteStart(ofstream& os, vector<const RouteInfo*>& routes,
		int idx, const char* prefix);
	void printRouteEnd(ofstream& os);
	void printSource(ofstream& os, vector<const RouteInfo*>& routes,
		int idx, const char* prefix);
	void printDest(ofstream& os, vector<const RouteInfo*>& routes,
		int idx, const char* prefix);
	void printSingleSourceMultipleDests(ofstream& os,
		vector<const RouteInfo*>& routes);
	void printMultipleSourcesSingleDest(ofstream& os,
		vector<const RouteInfo*>& routes);
	void printSingleSourceSingleDest(ofstream& os,
		 vector<const RouteInfo*>& routes);
};

}
}
}

#endif /* TEST_CONFIG_ROUTE_CONFIG_GEN */

