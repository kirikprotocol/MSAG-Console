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

bool RouteConfigGen::checkSubject(const Address& addr)
{
	return (addr.getLenght() % 2);
}

const string RouteConfigGen::genFakeAddress()
{
	Address addr;
	SmsUtil::setupRandomCorrectAddress(&addr, MAX_ADDRESS_LENGTH, MAX_ADDRESS_LENGTH);
	return SmsUtil::configString(addr);
}

bool RouteConfigGen::printFakeMask(ostream& os)
{
	if (!rand0(3))
	{
		os << "\t<mask value=\"" << genFakeAddress() << "\"/>" << endl;
		return true;
	}
	return false;
}

void RouteConfigGen::printSubject(ostream& os, const RouteInfo& route, char type)
{
	__decl_tc__;
	switch (type)
	{
		case 's':
			{
				const string tmp = SmsUtil::configString(route.source);
				os << "<subject_def id=\"" << "_src_" << route.routeId <<
					"\" defSme=\"" << route.smeSystemId << "\">" << endl;
				if (printFakeMask(os))
				{
					__tc__("routeConfig.routeSource.subject.fakeMaskBefore");
					__tc_ok__;
				}
				//__tc__("routeConfig.routeSource.subject");
				os << "\t<mask value=\"" << tmp.c_str() << "\"/>" << endl;
				//__tc_ok__;
				if (printFakeMask(os))
				{
					__tc__("routeConfig.routeSource.subject.fakeMaskAfter");
					__tc_ok__;
				}
				os << "</subject_def>" << endl;
			}
			break;
		case 'd':
			{
				const string tmp = SmsUtil::configString(route.dest);
				os << "<subject_def id=\"" << "_dst_" << route.routeId <<
					"\" defSme=\"" << route.smeSystemId << "\">" << endl;
				if (printFakeMask(os))
				{
					__tc__("routeConfig.routeDest.subject.fakeMaskBefore");
					__tc_ok__;
				}
				//__tc__("routeConfig.routeDest.subject");
				os << "\t<mask value=\"" << tmp.c_str() << "\"/>" << endl;
				//__tc_ok__;
				if (printFakeMask(os))
				{
					__tc__("routeConfig.routeDest.subject.fakeMaskAfter");
					__tc_ok__;
				}
				os << "</subject_def>" << endl;
			}
			break;
		default:
			__unreachable__("Invalid address type");
	}
}

void RouteConfigGen::printRouteStart(ostream& os, const RouteInfo& route)
{
	__decl_tc__;
	os << "<route id=\"" << route.routeId <<
		"\" billing=\"" << (route.billing ? "true" : "false") <<
		"\" archiving=\"" << (route.archived ? "true" : "false") <<
		"\" enabling=\"" << (route.enabling ? "true" : "false") <<
		"\" priority=\"" << route.priority <<
		"\" serviceId=\"" << route.serviceId <<
		"\" suppressDeliveryReports=\"" <<
		(route.suppressDeliveryReports ? "true" : "false") << "\">" << endl;
	if (route.billing)
	{
		__tc__("routeConfig.billing"); __tc_ok__;
	}
	if (route.archived)
	{
		__tc__("routeConfig.archivate"); __tc_ok__;
	}
	if (!route.enabling)
	{
		__tc__("routeConfig.notEnabling"); __tc_ok__;
	}
}

void RouteConfigGen::printRouteEnd(ostream& os)
{
	os << "</route>" << endl;
}

void RouteConfigGen::printSource(ostream& os, const RouteInfo& route)
{
	__decl_tc__;
	if (printFakeSource(os, route))
	{
		__tc__("routeConfig.routeSource.fakeSourceBefore"); __tc_ok__;
	}
	os << "\t<source>" << endl;
	if (checkSubject(route.source))
	{
		__tc__("routeConfig.routeSource.subject"); __tc_ok__;
		os << "\t\t<subject id=\"" << "_src_" << route.routeId << "\"/>" << endl;
	}
	else
	{
		__tc__("routeConfig.routeSource.mask"); __tc_ok__;
		const string tmp = SmsUtil::configString(route.source);
		os << "\t\t<mask value=\"" << tmp.c_str() << "\"/>" << endl;
	}
	os << "\t</source>" << endl;
	if (printFakeSource(os, route))
	{
		__tc__("routeConfig.routeSource.fakeSourceBefore"); __tc_ok__;
	}
}

bool RouteConfigGen::printFakeSource(ostream& os, const RouteInfo& route)
{
	if (rand0(1))
	{
		os << "\t<source>" << endl;
		os << "\t\t<mask value=\"" << genFakeAddress() << "\"/>" << endl;
		os << "\t</source>" << endl;
		return true;
	}
	return false;
}

void RouteConfigGen::printDest(ostream& os, const RouteInfo& route)
{
	__decl_tc__;
	if (printFakeDest(os, route))
	{
		__tc__("routeConfig.routeDest.fakeDestBefore"); __tc_ok__;
	}
	os << "\t<destination sme=\"" << route.smeSystemId << "\">" << endl;
	if (checkSubject(route.dest))
	{
		__tc__("routeConfig.routeDest.subject"); __tc_ok__;
		os << "\t\t<subject id=\"" << "_dst_" << route.routeId << "\"/>" << endl;
	}
	else
	{
		__tc__("routeConfig.routeDest.mask"); __tc_ok__;
		const string tmp = SmsUtil::configString(route.dest);
		os << "\t\t<mask value=\"" << tmp.c_str() << "\"/>" << endl;
	}
	os << "\t</destination>" << endl;
	if (printFakeDest(os, route))
	{
		__tc__("routeConfig.routeDest.fakeDestAfter"); __tc_ok__;
	}
}

bool RouteConfigGen::printFakeDest(ostream& os, const RouteInfo& route)
{
	if (rand0(1))
	{
		os << "\t<destination sme=\"" << route.smeSystemId << "\">" << endl;
		os << "\t\t<mask value=\"" << genFakeAddress() << "\"/>" << endl;
		os << "\t</destination>" << endl;
		return true;
	}
	return false;
}

void RouteConfigGen::printSubjects(ostream& os)
{
	os << "<!-- subjects -->" << endl;
	RouteRegistry::RouteIterator* it = routeReg->iterator();
	__require__(it);
	while (const RouteHolder* routeHolder = it->next())
	{
		if (checkSubject(routeHolder->route.source))
		{
			printSubject(os, routeHolder->route, 's');
		}
		if (checkSubject(routeHolder->route.dest))
		{
			printSubject(os, routeHolder->route, 'd');
		}
	}
	delete it;
}

void RouteConfigGen::printRoutes(ostream& os)
{
	os << "<!-- routes -->" << endl;
	RouteRegistry::RouteIterator* it = routeReg->iterator();
	__require__(it);
	while (const RouteHolder* routeHolder = it->next())
	{
		printRouteStart(os, routeHolder->route);
		printSource(os, routeHolder->route);
		printDest(os, routeHolder->route);
		printRouteEnd(os);
	}
	delete it;
}

void RouteConfigGen::saveConfig(const char* configFileName)
{
	__require__(configFileName);
	ofstream os(configFileName);
	os << "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>" << endl;
	os << "<!DOCTYPE routes SYSTEM \"routes.dtd\">" << endl;
	os << "<routes>" << endl;
	printSubjects(os);
	printRoutes(os);
	os << "</routes>" << endl;
}

}
}
}

