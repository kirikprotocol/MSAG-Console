#include "RouteConfigGen.hpp"
#include "test/sms/SmsUtil.hpp"
#include "test/core/RouteUtil.hpp"
#include <algorithm>

namespace smsc {
namespace test {
namespace config {

using namespace std;
using smsc::test::core::RouteHolder;
using smsc::test::sms::SmsUtil;

ltAddress RouteConfigGen::ltAddr = ltAddress();

bool RouteConfigGen::ltSource(const RouteInfo* r1, const RouteInfo* r2)
{
	if (!r1)
	{
		return false;
	}
	if (!r2)
	{
		return true;
	}
	if (ltAddr.operator()(r1->source, r2->source))
	{
		return true;
	}
	if (!SmsUtil::compareAddresses(r1->source, r2->source))
	{
		return false;
	}
	if (r1->billing != r2->billing)
	{
		return r1->billing;
	}
	if (r1->archived != r2->archived)
	{
		return r1->archived;
	}
	//bool paid;
	if (ltAddr.operator()(r1->dest, r2->dest))
	{
		return true;
	}
	return false;
}

bool RouteConfigGen::ltDest(const RouteInfo* r1, const RouteInfo* r2)
{
	if (!r1)
	{
		return false;
	}
	if (!r2)
	{
		return true;
	}
	if (ltAddr.operator()(r1->dest, r2->dest))
	{
		return true;
	}
	if (!SmsUtil::compareAddresses(r1->dest, r2->dest))
	{
		return false;
	}
	if (r1->billing != r2->billing)
	{
		return r1->billing;
	}
	if (r1->archived != r2->archived)
	{
		return r1->archived;
	}
	//bool paid;
	if (ltAddr.operator()(r1->source, r2->source))
	{
		return true;
	}
	return false;
}

void RouteConfigGen::printSubject(ofstream& os, vector<const RouteInfo*>& routes,
	int idx, char type, const char* prefix)
{
	__require__(routes[idx]);
	if (idx % 2)
	{
		switch (type)
		{
			case 's':
				{
					auto_ptr<char> tmp = SmsUtil::configString(routes[idx]->source);
					os << "<subject_def id=\"" << prefix << "_" << idx << "\">" << endl;
					os << "\t<mask value=\"" << tmp.get() << "\"/>" << endl;
					os << "</subject_def>" << endl;
				}
				break;
			case 'd':
				{
					auto_ptr<char> tmp = SmsUtil::configString(routes[idx]->dest);
					os << "<subject_def id=\"" << prefix << "_" << idx << "\">" << endl;
					os << "\t<mask value=\"" << tmp.get() << "\"/>" << endl;
					os << "</subject_def>" << endl;
				}
				break;
			default:
				__unreachable__("Invalid address type");
		}
	}
}

void RouteConfigGen::printRouteStart(ofstream& os, vector<const RouteInfo*>& routes,
	int idx, const char* prefix)
{
	__require__(routes[idx]);
	os << "<route id=\"" << prefix << "_" << idx <<
		"\" billing=\"" << (routes[idx]->billing ? "true" : "false") <<
		"\" archiving=\"" << (routes[idx]->archived ? "true" : "false") <<
		"\" enabling=\"" << (routes[idx]->enabling ? "true" : "false") << "\">" << endl;
	/*
		RoutePriority priority - пока не используется
		bool paid - вообще не используется, синоним billing
	*/
}

void RouteConfigGen::printRouteEnd(ofstream& os)
{
	os << "</route>" << endl;
}

void RouteConfigGen::printSource(ofstream& os, vector<const RouteInfo*>& routes,
	int idx, const char* prefix)
{
	__require__(routes[idx]);
	os << "\t<source>" << endl;
	if (idx % 2)
	{
		os << "\t\t<subject id=\"" << prefix << "_" << idx << "\"/>" << endl;
	}
	else
	{
		auto_ptr<char> tmp = SmsUtil::configString(routes[idx]->source);
		os << "\t\t<mask value=\"" << tmp.get() << "\"/>" << endl;
	}
	os << "\t</source>" << endl;
}

void RouteConfigGen::printDest(ofstream& os, vector<const RouteInfo*>& routes,
	int idx, const char* prefix)
{
	__require__(routes[idx]);
	os << "\t<destination sme=\"" << routes[idx]->smeSystemId << "\">" << endl;
	if (idx % 2)
	{
		os << "\t\t<subject id=\"" << prefix << "_" << idx << "\"/>" << endl;
	}
	else
	{
		auto_ptr<char> tmp = SmsUtil::configString(routes[idx]->dest);
		os << "\t\t<mask value=\"" << tmp.get() << "\"/>" << endl;
	}
	os << "\t</destination>" << endl;
}

void RouteConfigGen::printSingleSourceMultipleDests(ofstream& os,
	vector<const RouteInfo*>& routes)
{
	sort(routes.begin(), routes.end(), RouteConfigGen::ltSource);
	os << "<!-- single source & multiple dests -->" << endl;
	int idx = 0;
	int count = 0;
	for (int i = 1; i < routes.size(); i++)
	{
		if (!routes[i])
		{
			continue;
		}
		if (SmsUtil::compareAddresses(routes[idx]->source, routes[i]->source) &&
			routes[idx]->billing == routes[i]->billing &&
			routes[idx]->archived == routes[i]->archived /* paid */)
		{
			count++;
		}
		else if (count)
		{
			printSubject(os, routes, idx, 's', "ssmd_s");
			for (int j = idx; j < i; j++)
			{
				if (routes[j])
				{
					printSubject(os, routes, j, 'd', "ssmd_d");
				}
			}
			printRouteStart(os, routes, idx, "ssmd_r");
			printSource(os, routes, idx, "ssmd_s");
			for (int j = idx; j < i; j++)
			{
				if (routes[j])
				{
					printDest(os, routes, j, "ssmd_d");
					routes[j] = NULL;
				}
			}
			printRouteEnd(os);
			idx = i;
			count = 0;
		}
	}
}

void RouteConfigGen::printMultipleSourcesSingleDest(ofstream& os,
	vector<const RouteInfo*>& routes)
{
	sort(routes.begin(), routes.end(), RouteConfigGen::ltDest);
	os << "<!-- multiple sources & single dest -->" << endl;
	int idx = 0;
	int count = 0;
	for (int i = 1; i < routes.size(); i++)
	{
		if (!routes[i])
		{
			continue;
		}
		if (SmsUtil::compareAddresses(routes[idx]->dest, routes[i]->dest) &&
			routes[idx]->billing == routes[i]->billing &&
			routes[idx]->archived == routes[i]->archived /* paid */)
		{
			count++;
		}
		else if (count)
		{
			for (int j = idx; j < i; j++)
			{
				if (routes[j])
				{
					printSubject(os, routes, j, 's', "mssd_s");
				}
			}
			printSubject(os, routes, idx, 'd', "mssd_d");
			printRouteStart(os, routes, idx, "mssd_r");
			for (int j = idx; j < i; j++)
			{
				if (routes[j])
				{
					printSource(os, routes, j, "mssd_s");
					routes[j] = NULL;
				}
			}
			printDest(os, routes, idx, "mssd_d");
			printRouteEnd(os);
			idx = i;
			count = 0;
		}
	}
}

void RouteConfigGen::printSingleSourceSingleDest(ofstream& os,
	vector<const RouteInfo*>& routes)
{
	//генерация конфига
	os << "<!-- single source & single dest -->" << endl;
	for (int i = 0; i < routes.size(); i++)
	{
		if (!routes[i])
		{
			continue;
		}
		printSubject(os, routes, i, 's', "sssd_s");
		printSubject(os, routes, i, 'd', "sssd_d");
		printRouteStart(os, routes, i, "sssd_r");
		printSource(os, routes, i, "sssd_s");
		printDest(os, routes, i, "sssd_d");
		printRouteEnd(os);
	}
}

void RouteConfigGen::saveConfig(const char* configFileName)
{
	__require__(configFileName);
	//список маршрутов
	vector<const RouteInfo*> routes;
	RouteRegistry::RouteIterator* it = routeReg->iterator();
	__require__(it);
	while (const RouteHolder* routeHolder = it->next())
	{
		routes.push_back(&routeHolder->route);
	}
	delete it;
	//генерация конфига
	ofstream os(configFileName);
	os << "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>" << endl;
	os << "<!DOCTYPE routes SYSTEM \"routes.dtd\">" << endl;
	os << "<routes>" << endl;
	printSingleSourceMultipleDests(os, routes);
	printMultipleSourcesSingleDest(os, routes);
	printSingleSourceSingleDest(os, routes);
	os << "</routes>" << endl;
}

}
}
}

