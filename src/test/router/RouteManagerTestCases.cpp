#include "RouteManagerTestCases.hpp"
#include "test/core/CoreTestManager.hpp"
#include "test/smeman/SmeManagerUtil.hpp"
#include "test/sms/SmsUtil.hpp"
#include "test/util/Util.hpp"
#include <sys/types.h>

namespace smsc {
namespace test {
namespace router {

using smsc::test::core::CoreTestManager;
using smsc::test::sms::SmsUtil;
using namespace smsc::sms; //constants, AddressValue
using namespace smsc::router; //constants
using namespace smsc::test::util;
using namespace smsc::test::smeman; //constants, TestSmeProxy

RouteManagerTestCases::RouteManagerTestCases()
	: routeMan(CoreTestManager::getRouteManager()) {}

void RouteManagerTestCases::setupRandomCorrectRouteInfo(
	const SmeSystemId& smeSystemId, RouteInfo* route)
{
	route->priority = rand2(RoutePriorityMin, RoutePriorityMax);
	route->billing = rand0(1);
	route->paid = rand0(1);
	route->archived = rand0(1);
	route->routeId = rand0(INT_MAX);
	route->smeSystemId = smeSystemId;
	//route->source = ...
	//route->dest = ...
}

vector<int> RouteManagerTestCases::compareRoutes(const RouteInfo& route1,
	const RouteInfo& route2)
{
	vector<int> res;
	if (route1.priority != route2.priority)
	{
		res.push_back(1);
	}
	if (route1.billing != route2.billing)
	{
		res.push_back(2);
	}
	if (route1.paid != route2.paid)
	{
		res.push_back(3);
	}
	if (route1.archived != route2.archived)
	{
		res.push_back(4);
	}
	if (route1.routeId != route2.routeId)
	{
		res.push_back(5);
	}
	if (route1.smeSystemId != route2.smeSystemId)
	{
		res.push_back(6);
	}
	if (!SmsUtil::compareAddresses(route1.source, route2.source))
	{
		res.push_back(7);
	}
	if (!SmsUtil::compareAddresses(route1.dest, route2.dest))
	{
		res.push_back(8);
	}
	return res;
}

TCResult* RouteManagerTestCases::addCorrectRoute(const SmeSystemId& smeSystemId,
	TestRouteData* data, int num)
{
	TCSelector s(num, 14);
	TCResult* res = new TCResult(TC_ADD_CORRECT_ROUTE, s.getChoice());
	for (; s.check(); s++)
	{
		RouteInfo* route = new RouteInfo();
		data->match = true;
		data->route = route;
		try
		{
			setupRandomCorrectRouteInfo(smeSystemId, route);
			//origAddr
			AddressValue origAddrVal;
			uint8_t origAddrLen = data->origAddr.getValue(origAddrVal);
			int oaLen = rand1(origAddrLen);
			int oaLen2 = rand1(oaLen);
			switch(rand1(2))
			{
				case 1: //source ����� � ����� ��� ����� '?' � �����
					memset(origAddrVal + origAddrLen - oaLen, '?', oaLen);
					route->source.setValue(origAddrLen, origAddrVal);
					data->origAddrMatch = 100 - oaLen;
					break;
				case 2: //source ����� ������� ��� �������� �� '*' � �����
					memset(origAddrVal + origAddrLen - oaLen, '*', oaLen2);
					route->source.setValue(origAddrLen - oaLen + oaLen2, origAddrVal);
					data->origAddrMatch = 100 - oaLen - 0.5; //'*' ����� ����������� '?'
					break;
			}
			//destAddr
			AddressValue destAddrVal;
			uint8_t destAddrLen = data->destAddr.getValue(destAddrVal);
			int daLen = rand1(destAddrLen);
			int daLen2 = rand1(daLen);
			switch(rand1(2))
			{
				case 1: //dest ����� � ����� ��� ����� '?' � �����
					memset(destAddrVal + destAddrLen - daLen, '?', daLen);
					route->dest.setValue(destAddrLen, destAddrVal);
					data->destAddrMatch = 100 - daLen;
					break;
				case 2: //dest ����� ������� ��� �������� �� '*' � �����
					memset(destAddrVal + destAddrLen - daLen, '*', daLen2);
					route->dest.setValue(destAddrLen - daLen + daLen2, destAddrVal);
					data->destAddrMatch = 100 - daLen - 0.5; //'*' ����� ����������� '?'
					break;
			}
			switch(s.value())
			{
				case 1: //������������ ���������
					route->priority = RoutePriorityMax;
					break;
				case 2: //����������� ���������
					route->priority = RoutePriorityMin;
					break;
				case 3: //source ����� ������� �� '?'
					memset(origAddrVal, '?', origAddrLen);
					route->source.setValue(origAddrLen, origAddrVal);
					data->origAddrMatch = 100 - origAddrLen;
					break;
				case 4: //source ����� �� '*'
					memset(origAddrVal, '*', oaLen);
					route->source.setValue(oaLen, origAddrVal);
					data->origAddrMatch = 100 - origAddrLen - 0.5;
					break;
				case 5: //source ����� � ������ '?'
					if (origAddrLen < MAX_ADDRESS_VALUE_LENGTH)
					{
						data->origAddr.getValue(origAddrVal);
						origAddrVal[origAddrLen] = '?';
						route->source.setValue(origAddrLen + 1, origAddrVal);
						data->match = false;
					}
					break;
				case 6: //source ����� � ������ '*'
					if (origAddrLen < MAX_ADDRESS_VALUE_LENGTH)
					{
						data->origAddr.getValue(origAddrVal);
						origAddrVal[origAddrLen] = '*';
						route->source.setValue(origAddrLen + 1, origAddrVal);
						data->origAddrMatch = 100;
					}
					break;
				case 7: //source ����� � ������������� typeOfNumber
					route->source.setTypeOfNumber(
						data->origAddr.getTypeOfNumber() + 1);
					data->match = false;
					break;
				case 8: //source ����� � ������������� numberingPlan
					route->source.setNumberingPlan(
						data->origAddr.getNumberingPlan() + 1);
					data->match = false;
					break;
				case 9: //dest ����� ������� �� '?'
					memset(destAddrVal, '?', destAddrLen);
					route->dest.setValue(destAddrLen, destAddrVal);
					data->destAddrMatch = 100 - destAddrLen;
					break;
				case 10: //dest ����� �� '*'
					memset(destAddrVal, '*', daLen);
					route->dest.setValue(daLen, destAddrVal);
					data->destAddrMatch = 100 - destAddrLen - 0.5;
					break;
				case 11: //dest ����� � ������ '?'
					if (destAddrLen < MAX_ADDRESS_VALUE_LENGTH)
					{
						data->destAddr.getValue(destAddrVal);
						destAddrVal[destAddrLen] = '?';
						route->dest.setValue(destAddrLen + 1, destAddrVal);
						data->match = false;
					}
					break;
				case 12: //dest ����� � ������ '*'
					if (destAddrLen < MAX_ADDRESS_VALUE_LENGTH)
					{
						data->destAddr.getValue(destAddrVal);
						destAddrVal[destAddrLen] = '*';
						route->dest.setValue(destAddrLen + 1, destAddrVal);
						data->destAddrMatch = 100;
					}
					break;
				case 13: //dest ����� � ������������� typeOfNumber
					route->dest.setTypeOfNumber(
						data->destAddr.getTypeOfNumber() + 1);
					data->match = false;
					break;
				case 14: //dest ����� � ������������� numberingPlan
					route->dest.setNumberingPlan(
						data->destAddr.getNumberingPlan() + 1);
					data->match = false;
					break;
				default:
					throw s;
			}
			routeMan->addRoute(*route);
		}
		catch(...)
		{
			error();
			delete route;
			data->route = NULL;
			res->addFailure(100);
		}
	}
	debug(res);
	return res;
}

TCResult* RouteManagerTestCases::addCorrectRoute2(const SmeSystemId& smeSystemId,
	TestRouteData* data, int num)
{
	TCSelector s(num, 1000, 8);
	TCResult* res = new TCResult(TC_ADD_CORRECT_ROUTE, s.getChoice());
	for (; s.check(); s++)
	{
		RouteInfo* route = new RouteInfo();
		data->match = false;
		data->route = route;
		try
		{
			setupRandomCorrectRouteInfo(smeSystemId, route);
			SmsUtil::setupRandomCorrectAddress(&route->source);
			SmsUtil::setupRandomCorrectAddress(&route->dest);
			switch(s.value())
			{
				case 1: //origAddr.typeOfNumber ��� ���������
					route->source.setTypeOfNumber(rand2(0x7, 0xff));
					break;
				case 2: //origAddr.numberingPlan ��� ���������
					route->source.setNumberingPlan(rand2(0x13, 0xff));
					break;
				case 3: //������ '?' � �������� origAddr
					{
						AddressValue addrVal;
						auto_ptr<char> tmp = rand_char(MAX_ADDRESS_VALUE_LENGTH);
						memcpy(addrVal, tmp.get(), MAX_ADDRESS_VALUE_LENGTH);
						addrVal[MAX_ADDRESS_VALUE_LENGTH / 2] = '?';
						route->source.setValue(MAX_ADDRESS_VALUE_LENGTH, addrVal);
					}
					break;
				case 4: //������ '*' � �������� origAddr
					{
						AddressValue addrVal;
						auto_ptr<char> tmp = rand_char(MAX_ADDRESS_VALUE_LENGTH);
						memcpy(addrVal, tmp.get(), MAX_ADDRESS_VALUE_LENGTH);
						addrVal[MAX_ADDRESS_VALUE_LENGTH / 2] = '*';
						route->source.setValue(MAX_ADDRESS_VALUE_LENGTH, addrVal);
					}
					break;
				case 5: //destAddr.typeOfNumber ��� ���������
					route->dest.setTypeOfNumber(rand2(0x7, 0xff));
					break;
				case 6: //destAddr.numberingPlan ��� ���������
					route->dest.setNumberingPlan(rand2(0x13, 0xff));
					break;
				case 7: //������ '?' � �������� destAddr
					{
						AddressValue addrVal;
						auto_ptr<char> tmp = rand_char(MAX_ADDRESS_VALUE_LENGTH);
						memcpy(addrVal, tmp.get(), MAX_ADDRESS_VALUE_LENGTH);
						addrVal[MAX_ADDRESS_VALUE_LENGTH / 2] = '?';
						route->dest.setValue(MAX_ADDRESS_VALUE_LENGTH, addrVal);
					}
					break;
				case 8: //������ '*' � �������� destAddr
					{
						AddressValue addrVal;
						auto_ptr<char> tmp = rand_char(MAX_ADDRESS_VALUE_LENGTH);
						memcpy(addrVal, tmp.get(), MAX_ADDRESS_VALUE_LENGTH);
						addrVal[MAX_ADDRESS_VALUE_LENGTH / 2] = '*';
						route->dest.setValue(MAX_ADDRESS_VALUE_LENGTH, addrVal);
					}
					break;
				default:
					throw s;
			}
			routeMan->addRoute(*route);
		}
		catch(...)
		{
			error();
			delete route;
			data->route = NULL;
			res->addFailure(s.value());
		}
	}
	debug(res);
	return res;
}

TCResult* RouteManagerTestCases::addIncorrectRoute(const SmeSystemId& smeSystemId,
	const RouteInfo& existingRoute, int num)
{
	TCSelector s(num, 2);
	TCResult* res = new TCResult(TC_ADD_INCORRECT_ROUTE, s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			RouteInfo route;
			route.priority = rand2(RoutePriorityMin, RoutePriorityMax);
			route.billing = rand0(1);
			route.paid = rand0(1);
			route.archived = rand0(1);
			route.routeId = rand0(INT_MAX);
			route.smeSystemId = smeSystemId;
			SmsUtil::setupRandomCorrectAddress(&route.source);
			SmsUtil::setupRandomCorrectAddress(&route.dest);
			switch(s.value())
			{
				case 1: //����������� routeId
					route.routeId = existingRoute.routeId;
					break;
				case 2: //�������������� smeSystemId
					{
						auto_ptr<char> tmp = rand_char(rand1(MAX_SYSTEM_ID_LENGTH));
						route.smeSystemId = tmp.get();
					}
					break;
				default:
					throw s;
			}
			routeMan->addRoute(route);
			res->addFailure(101);
		}
		catch(...)
		{
			error();
			res->addFailure(100);
		}
	}
	debug(res);
	return res;
}

TCResult* RouteManagerTestCases::lookupRoute(const Address& origAddr,
	const Address& destAddr, const vector<const TestRouteData*>& routes)
{
	TCResult* res = new TCResult(TC_LOOKUP_ROUTE);
	try
	{
		SmeProxy* proxy;
		bool found = routeMan->lookup(origAddr, destAddr, proxy);
		if (!found)
		{
			res->addFailure(101);
		}
		else if (!proxy)
		{
			res->addFailure(102);
		}
		else
		{
			//���������� ���������: destAddr, origAddr, priority
			int idx = 0;
			for (int i = 0; i < routes.size(); i++)
			{
				if (!SmsUtil::compareAddresses(origAddr, routes[i]->origAddr) ||
					!SmsUtil::compareAddresses(destAddr, routes[i]->destAddr))
				{
					continue;
				}
				if (routes[idx]->destAddrMatch < routes[i]->destAddrMatch)
				{
					idx = i;
					continue;
				}
				if (routes[idx]->destAddrMatch == routes[i]->destAddrMatch)
				{
					if (routes[idx]->origAddrMatch < routes[i]->origAddrMatch)
					{
						idx = i;
						continue;
					}
					if (routes[idx]->origAddrMatch == routes[i]->origAddrMatch &&
						routes[idx]->route->priority < routes[i]->route->priority)
					{
						idx = i;
						continue;
					}
				}
			}
			TestSmeProxy* _proxy = (TestSmeProxy*) proxy;
			if (routes[idx]->route->smeSystemId != _proxy->getSystemId())
			{
				res->addFailure(103);
			}
		}
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	debug(res);
	return res;
}

TCResult* RouteManagerTestCases::iterateRoutes(
	const vector<const TestRouteData*>& routes)
{
	TCResult* res = new TCResult(TC_ITERATE_ROUTES);
	try
	{
		RouteIterator* iter = routeMan->iterator();
		int foundRoutes = 0;
		int extraRoutes = 0;
		typedef map<int, int> MismatchMap;
		MismatchMap mismatch;
		while(iter->next())
		{
			//SmeProxy* proxy = iter->getSmeProxy();
			//int index = iter->getIndex();
			RouteInfo route = iter->getRouteInfo();
			bool found = false;
			for (int i = 0; i < routes.size(); i++)
			{
				if (routes[i]->route->routeId == route.routeId)
				{
					found = true; foundRoutes++;
					vector<int> tmp = compareRoutes(*routes[i]->route, route);
					for (int j = 0; j < tmp.size(); j++)
					{
						mismatch[tmp[j]]++;
					}
					break;
				}
			}
			if (!found)
			{
				extraRoutes++;
			}
		}
		delete iter;
		//�������� ������ ������ ��������
		if (extraRoutes)
		{
			res->addFailure(101);
		}
		//�������� ��������� ��������� ��������
		if (foundRoutes != routes.size())
		{
			res->addFailure(102);
		}
		//����������� ������� � ���������
		for (MismatchMap::iterator it = mismatch.begin(); it != mismatch.end(); it++)
		{
			if (it->second)
			{
				res->addFailure(it->first);
			}
		}
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	debug(res);
	return res;
}

}
}
}

