#ifndef TEST_CONFIG_ROUTE_CONFIG_GEN
#define TEST_CONFIG_ROUTE_CONFIG_GEN

#include "test/core/RouteRegistry.hpp"
#include "test/sms/SmsUtil.hpp"
#include "ConfigGen.hpp"
#include "test/util/CheckList.hpp"
#include "util/debug.h"
#include <fstream>
#include <memory>

namespace smsc {
namespace test {
namespace config {

using std::auto_ptr;
using std::ostream;
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

	static bool ltSource(const RouteInfo* r1, const RouteInfo* r2);
	static bool ltDest(const RouteInfo* r1, const RouteInfo* r2);
	static bool eqSource(const RouteInfo* r1, const RouteInfo* r2);
	static bool eqDest(const RouteInfo* r1, const RouteInfo* r2);

	virtual void saveConfig(const char* configFileName);
	
private:
	typedef vector<const RouteInfo*> Routes;

	bool checkSubject(const Address& addr);
	auto_ptr<char> genFakeAddress();
	bool printFakeMask(ostream& os);
	void printSubject(ostream& os, const RouteInfo* route, char type,
		const char* prefix);
	void printRouteStart(ostream& os, const RouteInfo* route, const char* prefix);
	void printRouteEnd(ostream& os);
	void printSource(ostream& os, const RouteInfo* route,
		const char* prefix, bool printFake);
	bool printFakeSource(ostream& os, const RouteInfo* route);
	void printDest(ostream& os, const RouteInfo* route,
		const char* prefix, bool printFake);
	bool printFakeDest(ostream& os, const RouteInfo* route);
	void printSubjects(ostream& os, const Routes& routes, const char* prefix);
	void printRoutes(ostream& os, const Routes& routes, const char* prefix);
	/*
	template <class Pred>
	const Routes selectRoutes(Routes& routes, Pred ltPred, Pred eqPred);
	*/
};

}
}
}

#endif /* TEST_CONFIG_ROUTE_CONFIG_GEN */

