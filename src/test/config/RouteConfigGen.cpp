#include "RouteConfigGen.hpp"
#include "test/sms/SmsUtil.hpp"
#include "test/core/RouteUtil.hpp"
#include "test/util/Util.hpp"
#include <algorithm>

namespace smsc {
namespace test {
namespace config {

using namespace std;
using namespace smsc::test::sms;
using namespace smsc::test::util;
using smsc::test::core::RouteHolder;
using smsc::test::sms::operator==;
using smsc::test::sms::operator!=;
using smsc::test::sms::operator<;

bool RouteConfigGen::ltSource(const RouteInfo* r1, const RouteInfo* r2)
{
	__require__(r1 && r2);
	if (r1->source != r2->source)
	{
		return (r1->source < r2->source);
	}
	if (r1->billing != r2->billing)
	{
		return r1->billing;
	}
	if (r1->archived != r2->archived)
	{
		return r1->archived;
	}
	if (r1->enabling != r2->enabling)
	{
		return r1->enabling;
	}
	//bool paid;
	if (r1->dest != r2->dest)
	{
		return (r1->dest < r2->dest);
	}
	return (r1->smeSystemId < r2->smeSystemId);
}

bool RouteConfigGen::ltDest(const RouteInfo* r1, const RouteInfo* r2)
{
	__require__(r1 && r2);
	if (r1->dest != r2->dest)
	{
		return (r1->dest < r2->dest);
	}
	if (r1->smeSystemId != r2->smeSystemId)
	{
		return (r1->smeSystemId < r2->smeSystemId);
	}
	if (r1->billing != r2->billing)
	{
		return r1->billing;
	}
	if (r1->archived != r2->archived)
	{
		return r1->archived;
	}
	if (r1->enabling != r2->enabling)
	{
		return r1->enabling;
	}
	//bool paid;
	return (r1->source < r2->source);
}

bool RouteConfigGen::eqSource(const RouteInfo* r1, const RouteInfo* r2)
{
	__require__(r1 && r2);
	return (r1->source == r2->source && r1->billing == r2->billing &&
		r1->archived == r2->archived && r1->enabling == r2->enabling);
}

bool RouteConfigGen::eqDest(const RouteInfo* r1, const RouteInfo* r2)
{
	__require__(r1 && r2);
	return (r1->dest == r2->dest && r1->smeSystemId == r2->smeSystemId &&
		r1->billing == r2->billing && r1->archived == r2->archived &&
		r1->enabling == r2->enabling);
}

bool RouteConfigGen::checkSubject(const Address& addr)
{
	return (addr.getLenght() % 2);
}

auto_ptr<char> RouteConfigGen::genFakeAddress()
{
	auto_ptr<char> tmp = rand_char(MAX_ADDRESS_LENGTH);
	char* addr = new char[30];
	sprintf(addr, ".%d.%d.%s", rand0(255), rand0(255), tmp.get());
	return auto_ptr<char>(addr);
}

bool RouteConfigGen::printFakeMask(ostream& os)
{
	if (!rand0(3))
	{
		auto_ptr<char> tmp = genFakeAddress();
		os << "\t<mask value=\"" << tmp.get() << "\"/>" << endl;
		return true;
	}
	return false;
}

void RouteConfigGen::printSubject(ostream& os, const RouteInfo* route,
	char type, const char* prefix)
{
	__require__(route);
	switch (type)
	{
		case 's':
			{
				auto_ptr<char> tmp = SmsUtil::configString(route->source);
				os << "<subject_def id=\"" << prefix << "_src_" << route->routeId <<
					"\" defSme=\"" << route->smeSystemId << "\">" << endl;
				printFakeMask(os);
				os << "\t<mask value=\"" << tmp.get() << "\"/>" << endl;
				printFakeMask(os);
				os << "</subject_def>" << endl;
			}
			break;
		case 'd':
			{
				auto_ptr<char> tmp = SmsUtil::configString(route->dest);
				os << "<subject_def id=\"" << prefix << "_dst_" << route->routeId <<
					"\" defSme=\"" << route->smeSystemId << "\">" << endl;
				printFakeMask(os);
				os << "\t<mask value=\"" << tmp.get() << "\"/>" << endl;
				printFakeMask(os);
				os << "</subject_def>" << endl;
			}
			break;
		default:
			__unreachable__("Invalid address type");
	}
}

void RouteConfigGen::printRouteStart(ostream& os, const RouteInfo* route,
	const char* prefix)
{
	__require__(route);
	os << "<route id=\"" << prefix << "_route_" << route->routeId <<
		"\" billing=\"" << (route->billing ? "true" : "false") <<
		"\" archiving=\"" << (route->archived ? "true" : "false") <<
		"\" enabling=\"" << (route->enabling ? "true" : "false") << "\">" << endl;
	/*
		RoutePriority priority - пока не используетс€
		bool paid - вообще не используетс€, синоним billing
	*/
}

void RouteConfigGen::printRouteEnd(ostream& os)
{
	os << "</route>" << endl;
}

void RouteConfigGen::printSource(ostream& os, const RouteInfo* route,
	const char* prefix, bool printFake)
{
	__require__(route);
	if (printFake)
	{
		printFakeSource(os, route);
	}
	os << "\t<source>" << endl;
	if (checkSubject(route->source))
	{
		os << "\t\t<subject id=\"" << prefix << "_src_" << route->routeId << "\"/>" << endl;
	}
	else
	{
		auto_ptr<char> tmp = SmsUtil::configString(route->source);
		os << "\t\t<mask value=\"" << tmp.get() << "\"/>" << endl;
	}
	os << "\t</source>" << endl;
	if (printFake)
	{
		printFakeSource(os, route);
	}
}

bool RouteConfigGen::printFakeSource(ostream& os, const RouteInfo* route)
{
	if (rand0(3))
	{
		__require__(route);
		os << "\t<source>" << endl;
		auto_ptr<char> tmp = genFakeAddress();
		os << "\t\t<mask value=\"" << tmp.get() << "\"/>" << endl;
		os << "\t</source>" << endl;
		return true;
	}
	return false;
}

void RouteConfigGen::printDest(ostream& os, const RouteInfo* route,
	const char* prefix, bool printFake)
{
	__require__(route);
	if (printFake)
	{
		printFakeDest(os, route);
	}
	os << "\t<destination sme=\"" << route->smeSystemId << "\">" << endl;
	if (checkSubject(route->dest))
	{
		os << "\t\t<subject id=\"" << prefix << "_dst_" << route->routeId << "\"/>" << endl;
	}
	else
	{
		auto_ptr<char> tmp = SmsUtil::configString(route->dest);
		os << "\t\t<mask value=\"" << tmp.get() << "\"/>" << endl;
	}
	os << "\t</destination>" << endl;
	if (printFake)
	{
		printFakeDest(os, route);
	}
}

bool RouteConfigGen::printFakeDest(ostream& os, const RouteInfo* route)
{
	if (rand0(3))
	{
		__require__(route);
		os << "\t<destination sme=\"" << route->smeSystemId << "\">" << endl;
		auto_ptr<char> tmp = genFakeAddress();
		os << "\t\t<mask value=\"" << tmp.get() << "\"/>" << endl;
		os << "\t</destination>" << endl;
		return true;
	}
	return false;
}

void RouteConfigGen::printSubjects(ostream& os, const Routes& routes,
	const char* prefix)
{
	os << "<!-- " << prefix << " subjects -->" << endl;
	for (Routes::const_iterator it = routes.begin(); it != routes.end(); it++)
	{
		if (checkSubject((*it)->source))
		{
			printSubject(os, *it, 's', prefix);
		}
		if (checkSubject((*it)->dest))
		{
			printSubject(os, *it, 'd', prefix);
		}
	}
}
void RouteConfigGen::printRoutes(ostream& os, const Routes& routes,
	const char* prefix)
{
	if (!routes.size())
	{
		return;
	}
	Routes::const_iterator it = routes.begin();
	const RouteInfo* prevRoute = *it++;
	os << "<!-- " << prefix << " routes -->" << endl;
	bool fakeDest = rand0(1);
	printRouteStart(os, prevRoute, prefix);
	printSource(os, prevRoute, prefix, !fakeDest);
	for (; it != routes.end(); it++)
	{
		if (eqSource(*it, prevRoute))
		{
			os << "<!-- eqSource -->" << endl;
			printDest(os, prevRoute, prefix, fakeDest);
		}
		else if (eqDest(*it, prevRoute))
		{
			os << "<!-- eqDest -->" << endl;
			printSource(os, *it, prefix, !fakeDest);
		}
		else
		{
			printDest(os, prevRoute, prefix, fakeDest);
			printRouteEnd(os);
			fakeDest = rand0(1);
			printRouteStart(os, *it, prefix);
			printSource(os, *it, prefix, !fakeDest);
		}
		prevRoute = *it;
	}
	printDest(os, prevRoute, prefix, fakeDest);
	printRouteEnd(os);
}

/*
template <class Pred>
const Routes RouteConfigGen::selectRoutes(Routes& routes, Pred ltPred, Pred eqPred)
{
	Routes res;
	sort(routes.begin(), routes.end(), ltPred);
    Routes::iterator it = adjacent_find(routes.begin(), routes.end(), eqPred);
	while (it != routes.end())
	{
		res.push_back(*it);
		it = routes.erase(it);
		while (it != routes.end() || !eqPred(*it, src.back()))
		{
			res.push_back(*it);
			it = routes.erase(it);
		}
		it = adjacent_find(it, routes.end(), eqPred);
	}
	return res;
}
*/

void RouteConfigGen::saveConfig(const char* configFileName)
{
	__require__(configFileName);
	//список маршрутов
	Routes routes;
	RouteRegistry::RouteIterator* it = routeReg->iterator();
	__require__(it);
	while (const RouteHolder* routeHolder = it->next())
	{
		/*
		RouteInfo* routeInfo = new RouteInfo(routeHolder->route);
		routeInfo.routeId = i++;
		routes.push_back(routeInfo);
		*/
		routes.push_back(&routeHolder->route);
	}
	delete it;
	/*
	const Routes routes_ssmd =
		selectRoutes(routes, RouteConfigGen::ltSource, RouteConfigGen::eqSource);
	const Routes routes_mssd =
		selectRoutes(routes, RouteConfigGen::ltDest, RouteConfigGen::eqDest);
	*/
	//генераци€ конфига
	ofstream os(configFileName);
	os << "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>" << endl;
	os << "<!DOCTYPE routes SYSTEM \"routes.dtd\">" << endl;
	os << "<routes>" << endl;
	//subjects
	/*
	printSubjects(os, routes_ssmd, "ssmd");
	printSubjects(os, routes_mssd, "mssd");
	*/
	printSubjects(os, routes, "sssd");
	//routes
	/*
	os << "<!-- single source & multiple dests -->" << endl;
	printRoutes(os, routes_ssmd, "ssmd");
	os << "<!-- multiple sources & single dest -->" << endl;
	printRoutes(os, routes_mssd, "mssd");
	*/
	os << "<!-- single source & single dest -->" << endl;
	printRoutes(os, routes, "sssd");
	os << "</routes>" << endl;
}

}
}
}

